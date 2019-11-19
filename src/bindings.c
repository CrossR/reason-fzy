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

typedef struct _choice {
  choices_t *choice;
} choice_W;

void finalize_choice(value v) {
  choice_W *p;
  p = (choice_W *)Data_custom_val(v);
  choices_destroy(p->choice);
}

static struct custom_operations choices_custom_ops = {
  .identifier = "choices handling",
  .finalize = custom_finalize_default,
  .compare = custom_compare_default,
  .hash = custom_hash_default,
  .serialize = custom_serialize_default,
  .deserialize = custom_deserialize_default
};

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
        const double score = choices_getscore(&choices, i);
        const char *term = choices_get(&choices, i);
        
        // This isn't actually computed at the moment.
        // Its only done in the tty bits. I can either call
        // match_positions directly here. Or... I can edit
        // fzy to just always call that. That way I don't need
        // to recalc the expensive score again.
        //
        // Probably something I should benchmark, but seems
        // silly to me to redo a full match alg, just for the
        // positions.
        //
        // Finally, could just do the calculation here, since
        // it is only a basic thing. That way, I don't need to
        // edit the fzy source code, aiding bringing in upstream
        // stuff.

        // const size_t* positions = []; 

        // We want to store the score and the search term.
        // Well and the positions.
        Store_double_field(scoreList, i, score);

        if (score > 0) {
          printf("%s scored %f\n.", term, score);
        }
    }

    choices_destroy(&choices);

    // We want to return score, terms and positions.
    // I think just returning only the few that have stuff is the best.
    // We can then just populate the rest in Reason-land. I.e. if its missing,
    // give it 0 and [].
    //
    // I should check that though...maybe its better/faster to populate
    // that here, I'm not really sure.
    CAMLreturn(scoreList);
}
