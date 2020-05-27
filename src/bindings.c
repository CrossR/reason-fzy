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
#endif

#include <choices.h>

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

void *safe_realloc(void *buffer, size_t size) {
    buffer = realloc(buffer, size);
    if (!buffer) {
        fprintf(stderr, "Error: Can't allocate memory (%zu bytes)\n", size);
        abort();
    }

    return buffer;
}

CAMLprim value fzy_search_for_item_in_list(value vItems, value vQuery, value vSort) {
    CAMLparam3(vItems, vQuery, vSort);
    CAMLlocal3(head, cons, return_list);

    return_list = Val_emptylist;

    if (vItems == Val_emptylist) {
        CAMLreturn(return_list);
    }

    int sorted = Bool_val(vSort) ? 1 : 0;
    choices_t choices = fzy_init(sorted);

    int itemCapacity = 100;
    int currentItem = 0;
    char **items;
    items = safe_realloc(items, itemCapacity * sizeof(const char *));

    // Lists here are represented as [0, [1, [2, []]]]
    while (vItems != Val_emptylist) {

        // Since we don't know how many items there are for a list, we may need
        // a resize. currentItem - 1 is the current capacity.
        if (currentItem >= itemCapacity) {
            itemCapacity += 100;
            items = safe_realloc(items, itemCapacity * sizeof(const char *));
        }

        head = Field(vItems, 0);
        items[currentItem] = strdup(String_val(head));
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
        items[i] = strdup(String_val(Field(vItems, i)));
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
