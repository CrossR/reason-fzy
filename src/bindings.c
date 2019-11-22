#include <string.h>
#include <stdio.h>

#include <caml/alloc.h>
#include <caml/bigarray.h>
#include <caml/callback.h>
#include <caml/custom.h>
#include <caml/memory.h>
#include <caml/mlvalues.h>
#include <caml/threads.h>

#include <choices.h>

CAMLprim value fzy_search_for_item(value vHaystack, value vNeedle) {
    CAMLparam2(vHaystack, vNeedle);
    CAMLlocal2(v, ret);

    options_t options;
    choices_t choices;
    options_init(&options);
    choices_init(&choices, &options);

    int nItems = Wosize_val(vHaystack);

    for (int i = 0; i < nItems; ++i) {
        choices_add(&choices, String_val(Field(vHaystack, i)));
    }

    const char *needle = String_val(vNeedle);
    choices_search(&choices, needle);

    ret = caml_alloc(choices_available(&choices), 0);

    // Fzy only stores scores for those with an actual match.
    for (int i = 0; i < choices_available(&choices); ++i) {

        // Get the search terms that actually matched.
        const char *matchTerm = choices_get(&choices, i);
        
        // Now go back and get the score again, and populate the
        // match locations.
        int n = strlen(matchTerm) + 1;
        size_t positions[n];
        for (int i = 0; i < n; i++)
          positions[i] = -1;

        const double score = match_positions(needle, matchTerm, &positions[0]);

        printf("Starting alloc....\n");
        v = caml_alloc(2, 0);

        printf("Starting stores....\n");
        Store_field(v, 0, caml_copy_string(matchTerm));
        Store_field(v, 1, caml_copy_double(score));
        // Store_field(v, 2, Double_val(positions));

        printf("Starting store to ret....\n");
        Store_field(ret, i, v);
    }

    choices_destroy(&choices);

    printf("Returning....\n");
    CAMLreturn(ret);
}
