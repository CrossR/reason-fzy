#include <string.h>
#include <stdio.h>

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

CAMLprim value fzy_search_for_item(choices_t choices, value vNeedle) {
    CAMLparam1(vNeedle);
    CAMLlocal3(match_item, matched_chars, return_array);

    const char *needle = String_val(vNeedle);
    const int needleSize = strlen(needle);
    choices_search(&choices, needle);

    return_array = caml_alloc(choices_available(&choices), 0);

    // Fzy only stores scores for those with an actual match.
    for (int i = 0; i < choices_available(&choices); ++i) {

        // Get the search terms that actually matched.
        const char *matchTerm = choices_get(&choices, i);

        // Now go back and get the score again, and populate the
        // match locations.
        const int n = strlen(matchTerm) + 1;
        size_t positions[n];
        for (int i = 0; i < n; i++)
          positions[i] = -1;

        const double score = match_positions(needle, matchTerm, &positions[0]);

        match_item = caml_alloc(3, 0);

        matched_chars = caml_alloc(needleSize, 0);

        for (int i = 0; i < needleSize; ++i)
            Store_field(matched_chars, i, Val_int(positions[i]));

        Store_field(match_item, 0, caml_copy_string(matchTerm));
        Store_field(match_item, 1, caml_copy_double(score));
        Store_field(match_item, 2, matched_chars);

        Store_field(return_array, i, match_item);
    }

    choices_destroy(&choices);

    return return_array;
}

CAMLprim value fzy_search_for_item_in_list(value vHaystack, value vNeedle, value vSort) {
    CAMLparam3(vHaystack, vNeedle, vSort);
    CAMLlocal2(head, return_array);

    if (vHaystack == Val_emptylist) {
        return return_array;
    }

    int sorted = Bool_val(vSort) ? 1 : 0;
    choices_t choices = fzy_init(sorted);

    // Lists here are represented as [0, [1, [2, []]]]
    while(vHaystack != Val_emptylist) {
        head = Field(vHaystack, 0);
        choices_add(&choices, String_val(head));
        vHaystack = Field(vHaystack, 1);
    }

    CAMLreturn(fzy_search_for_item(choices, vNeedle));
}

CAMLprim value fzy_search_for_item_in_array(value vHaystack, value vNeedle, value vSort) {
    CAMLparam3(vHaystack, vNeedle, vSort);
    CAMLlocal1(return_array);

    if (vHaystack == Val_emptylist) {
        return return_array;
    }

    int sorted = Bool_val(vSort) ? 1 : 0;
    choices_t choices = fzy_init(sorted);
    const int nItems = Wosize_val(vHaystack);

    for (int i = 0; i < nItems; ++i) {
        choices_add(&choices, String_val(Field(vHaystack, i)));
    }

    CAMLreturn(fzy_search_for_item(choices, vNeedle));
}

