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
  .finalize = finalize_choice,
  .compare = custom_compare_default,
  .hash = custom_hash_default,
  .serialize = custom_serialize_default,
  .deserialize = custom_deserialize_default
};

void test_match() {
    score_t score = match("main", "packages/core/src/main.tex");
    printf("Score was %f\n", score);
};

CAMLprim value fzy_choices_init(value unit) {
    CAMLparam0();
    CAMLlocal1(ret);

    choices_t choices;
    options_t options;

    options_init(&options);
    choices_init(&choices, &options);

    ret = caml_alloc_custom(&choices_custom_ops, sizeof(choice_W), 0, 1);
    memcpy(Data_custom_val(ret), &choices, sizeof(choice_W));

    CAMLreturn(ret);
}

CAMLprim value fzy_choices_add(value vChoices, value vToAdd) {
    CAMLparam2(vChoices, vToAdd);
    CAMLlocal1(ret);

    choice_W *p = Data_custom_val(vChoices);
    choices_t *choices = p->choice;

    const char *to_add = String_val(vToAdd);

    choices_add(choices, to_add);

    ret = caml_alloc_custom(&choices_custom_ops, sizeof(choice_W), 0, 1);
    memcpy(Data_custom_val(ret), &choices, sizeof(choice_W));

    CAMLreturn(ret);
}

CAMLprim value fzy_choices_search(value vChoices, value vSearch) {
    CAMLparam2(vChoices, vSearch);
    CAMLlocal1(ret);

    choice_W *p = Data_custom_val(vChoices);
    choices_t *choices = p->choice;

    char *search = String_val(vSearch);

    choices_search(choices, search);

    ret = caml_alloc_custom(&choices_custom_ops, sizeof(choice_W), 0, 1);
    memcpy(Data_custom_val(ret), &choices, sizeof(choice_W));

    CAMLreturn(ret);
}

CAMLprim value fzy_choices_get(value vChoices, value vN) {
    CAMLparam2(vChoices, vN);
    CAMLlocal1(ret);

    choice_W *p = Data_custom_val(vChoices);
    choices_t *choices = p->choice;

    size_t n = (size_t) Int_val(vN);

    const char *result = choices_get(choices, n);

    Store_field(ret, 0, caml_copy_string(result));
    CAMLreturn(ret);
}

CAMLprim value fzy_choices_getscore(value vChoices, value vN) {
    CAMLparam2(vChoices, vN);

    choice_W *p = Data_custom_val(vChoices);
    choices_t *choices = p->choice;

    size_t n = (size_t) Int_val(vN);

    score_t score = choices_getscore(choices, n);

    CAMLreturn(Val_long(score));
}
