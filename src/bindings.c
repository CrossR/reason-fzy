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
    sysctlbyname("hw.logicalcpu", &core_count, &size, NULL, 0 );
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

CAMLprim value format_return_item(choices_t choices, const int i, const char *needle) {
    CAMLparam0();
    CAMLlocal2(match_item, matched_chars);

    const int needleSize = strlen(needle);
    const char *matchTerm = choices_get(&choices, i);
    const int index = choices_getindex(&choices, i);

    size_t positions[needleSize];
    for (int i = 0; i < needleSize; i++)
        positions[i] = -1;

    const double score = match_positions(needle, matchTerm, &positions[0]);

    match_item = caml_alloc(4, 0);
    matched_chars = caml_alloc(needleSize, 0);

    for (int i = 0; i < needleSize; ++i)
        Store_field(matched_chars, i, Val_int(positions[i]));

    Store_field(match_item, 0, caml_copy_string(matchTerm));
    Store_field(match_item, 1, caml_copy_double(score));
    Store_field(match_item, 2, Val_int(index));
    Store_field(match_item, 3, matched_chars);

    CAMLreturn(match_item);
}

CAMLprim value fzy_search_for_item_in_list(value vHaystack, value vNeedle, value vSort) {
    CAMLparam3(vHaystack, vNeedle, vSort);
    CAMLlocal3(head, cons, return_list);

    return_list = Val_emptylist;

    if (vHaystack == Val_emptylist) {
        CAMLreturn(return_list);
    }

    int sorted = Bool_val(vSort) ? 1 : 0;
    choices_t choices = fzy_init(sorted);

    // Lists here are represented as [0, [1, [2, []]]]
    while(vHaystack != Val_emptylist) {
        head = Field(vHaystack, 0);
        choices_add(&choices, String_val(head));
        vHaystack = Field(vHaystack, 1);
    }

    const char *needle = String_val(vNeedle);
    choices_search(&choices, needle);

    const int numChoices = choices_available(&choices);

    // Fzy only stores scores for those with an actual match.
    for (int i = numChoices - 1; i >= 0; --i) {
        cons = caml_alloc(2, 0);
        Store_field(cons, 0, format_return_item(choices, i, needle));
        Store_field(cons, 1, return_list);
        return_list = cons;
    }

    choices_destroy(&choices);

    CAMLreturn(return_list);
}

CAMLprim value fzy_search_for_item_in_array(value vHaystack, value vNeedle, value vSort) {
    CAMLparam3(vHaystack, vNeedle, vSort);
    CAMLlocal1(return_array);

    return_array = Val_emptylist;

    if (vHaystack == Val_emptylist) {
        CAMLreturn(return_array);
    }

    int sorted = Bool_val(vSort) ? 1 : 0;
    choices_t choices = fzy_init(sorted);
    const int nItems = Wosize_val(vHaystack);
    char *items[nItems];

    for (int i = 0; i < nItems; ++i) {
        items[i] = strdup(String_val(Field(vHaystack, i)));
        choices_add(&choices, items[i]);
    }

    char *needle = strdup(String_val(vNeedle));
    choices_search(&choices, needle);

    int numChoices = choices_available(&choices);
    return_array = caml_alloc(numChoices, 0);

    // Fzy only stores scores for those with an actual match.
    for (int i = 0; i < numChoices; ++i) {
        Store_field(return_array, i, format_return_item(choices, i, needle));
    }

    free(needle);
    choices_destroy(&choices);

    for (int i = 0; i < nItems; ++i) {
        free(items[i]);
    }

    CAMLreturn(return_array);
}

