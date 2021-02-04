#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <caml/alloc.h>
#include <caml/bigarray.h>
#include <caml/callback.h>
#include <caml/custom.h>
#include <caml/memory.h>
#include <caml/mlvalues.h>
#include <caml/threads.h>

#ifdef __APPLE__
#include "sys/sysctl.h"
#endif

#ifdef _WIN32
#include <windows.h>

// strndup is not available on windows
char *strndup( const char *sz, size_t len)
{
    char *ret= (char*)malloc(len+1);
    memcpy(ret, sz, len);
    ret[len] = 0;
    return ret;
};

#endif

#include <choices.h>
#include <match.h>

int get_core_count() {
#if defined(__APPLE__)
    int core_count = 1;
    size_t size = sizeof(core_count);
    sysctlbyname("hw.logicalcpu", &core_count, &size, NULL, 0);
    return core_count;
#elif defined(_WIN32)
    SYSTEM_INFO siSysInfo;
    GetSystemInfo(&siSysInfo);
    return (int)siSysInfo.dwNumberOfProcessors;
#elif defined(__unix__) && defined(_SC_NPROCESSORS_ONLN)
    return (int)sysconf(_SC_NPROCESSORS_ONLN);
#else
    return 1; // Sane default for now.
#endif
}

choices_t fzy_init(int sorted) {

    options_t options;
    choices_t choices;
    options_init(&options);
    options.workers = get_core_count();
    options.sort_choices = sorted;
    choices_init(&choices, &options);

    return choices;
}

CAMLprim value format_return_item(choices_t choices, const int itemNum, const char *query) {
    CAMLparam0();
    CAMLlocal2(match_item, matched_chars);

    const int queryLen = strlen(query);
    const char *itemString = choices_get(&choices, itemNum);
    const int index = choices_getindex(&choices, itemNum);

    size_t positions[queryLen];
    for (int i = 0; i < queryLen; i++)
        positions[i] = -1;

    const double score = match_positions(query, itemString, &positions[0]);

    match_item = caml_alloc(4, 0);
    matched_chars = caml_alloc(queryLen, 0);

    for (int i = 0; i < queryLen; ++i)
        Store_field(matched_chars, i, Val_int(positions[i]));

    Store_field(match_item, 0, caml_copy_string(itemString));
    Store_field(match_item, 1, caml_copy_double(score));
    Store_field(match_item, 2, Val_int(index));
    Store_field(match_item, 3, matched_chars);

    CAMLreturn(match_item);
}

CAMLprim value fzy_search_for_item_in_list(value vItems, value vQuery, value vSort) {
    CAMLparam3(vItems, vQuery, vSort);
    CAMLlocal4(head, itemsCopy, cons, return_list);

    return_list = Val_emptylist;

    if (vItems == Val_emptylist) {
        CAMLreturn(return_list);
    }

    int sorted = Bool_val(vSort) ? 1 : 0;
    choices_t choices = fzy_init(sorted);

    // Store a copy of the head of the list, so we can reset back.
    itemsCopy = vItems;
    int numItems = 0;

    // Since we can't get the length of the list directly, run through it once
    // to get the size, so we can do a single allocation and store the results
    // in a subsequent pass.
    //
    // Benchmarking put this at the same speed as doing dynamic reallocs whilst
    // running through the list...but the code is cleaner so.
    while (vItems != Val_emptylist) {
        head = Field(vItems, 0);
        vItems = Field(vItems, 1);
        ++numItems;
    }

    int currentItem = 0;
    char *items[numItems];

    // Return vItems back to the start.
    vItems = itemsCopy;

    // Lists here are represented as [0, [1, [2, []]]]
    while (vItems != Val_emptylist) {
        head = Field(vItems, 0);
        const char* str = String_val(head);
        int len = strlen(str);
        if (len > 1024 /* MATCH_MAX_LEN */) {
            len = 1024 /* MATCH_MAX_LEN */;
        }
        items[currentItem] = strndup(str, len);
        choices_add(&choices, items[currentItem]);
        vItems = Field(vItems, 1);
        ++currentItem;
    }

    char *query = strdup(String_val(vQuery));
    choices_search(&choices, query);

    const int numChoices = choices_available(&choices);

    // Fzy only stores scores for those with an actual match.
    for (int i = numChoices - 1; i >= 0; --i) {
        cons = caml_alloc(2, 0);
        Store_field(cons, 0, format_return_item(choices, i, query));
        Store_field(cons, 1, return_list);
        return_list = cons;
    }

    free(query);
    choices_destroy(&choices);

    for (int i = 0; i < currentItem; ++i) {
        free(items[i]);
    }

    CAMLreturn(return_list);
}

CAMLprim value fzy_search_for_item_in_array(value vItems, value vQuery, value vSort) {
    CAMLparam3(vItems, vQuery, vSort);
    CAMLlocal1(return_array);

    return_array = Val_emptylist;

    if (vItems == Val_emptylist) {
        CAMLreturn(return_array);
    }

    int sorted = Bool_val(vSort) ? 1 : 0;
    choices_t choices = fzy_init(sorted);
    const int nItems = Wosize_val(vItems);
    char *items[nItems];

    for (int i = 0; i < nItems; ++i) {
        const char* str = String_val(Field(vItems, i));
        int len = strlen(str);
        if (len > 1024 /* MATCH_MAX_LEN */) {
            len = 1024 /* MATCH_MAX_LEN */;
        }
        items[i] = strndup(str, len);
        choices_add(&choices, items[i]);
    }

    char *query = strdup(String_val(vQuery));
    choices_search(&choices, query);

    int numChoices = choices_available(&choices);
    return_array = caml_alloc(numChoices, 0);

    // Fzy only stores scores for those with an actual match.
    for (int i = 0; i < numChoices; ++i) {
        Store_field(return_array, i, format_return_item(choices, i, query));
    }

    free(query);
    choices_destroy(&choices);

    for (int i = 0; i < nItems; ++i) {
        free(items[i]);
    }

    CAMLreturn(return_array);
}
