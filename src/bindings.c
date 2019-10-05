#include <caml/alloc.h>
#include <caml/bigarray.h>
#include <caml/callback.h>
#include <caml/custom.h>
#include <caml/memory.h>
#include <caml/mlvalues.h>
#include <caml/threads.h>

#include <match.h>
#include <stdio.h>

void test_match() {
    score_t score = match("main", "packages/core/src/main.tex");
    printf("Score was %f", score);
};

