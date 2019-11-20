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
    CAMLlocal1(scoreList);

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

    scoreList = caml_alloc(nItems * Double_wosize, Double_array_tag);

    // Fzy only stores scores for those with an actual match.
    for (int i = 0; i < choices_available(&choices); ++i) {

        // Get the search terms that actually matched.
        const char *term = choices_get(&choices, i);
        
        // Now go back and get the score again, and populate the
        // match locations.
        int n = strlen(term);
        size_t positions[n + 1];
        for (int i = 0; i < n + 1; i++)
          positions[i] = -1;

        const double score = match_positions(needle, term, &positions[0]);

        // At this point we have everything:
        //    - term
        //    - score
        //    - positions[:strlen(needle)]

        // TODO: Is it always strlen(needle)?
        // If there is a needle of len 10 and we match 8, what then?

        Store_double_field(scoreList, i, score);
    }

    choices_destroy(&choices);

    CAMLreturn(scoreList);
}
