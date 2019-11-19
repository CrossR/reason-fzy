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

    for (int i = 0; i < nItems; ++i) {
        double score = choices_getscore(&choices, i);
        // printf("Score was %f\n", score);

        // If the score is NaN, just store 0.
        if (score != score) {
          Store_double_field(scoreList, i, 0.0);
        } else {
          Store_double_field(scoreList, i, score);
        }
    }

    choices_destroy(&choices);
    CAMLreturn(scoreList);
}
