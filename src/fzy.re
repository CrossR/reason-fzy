type t;

module ScoredResult = {
    type t = {
        score: float,
        str: string
    };
};

module Choices = {
    type t = {
        buffer: string,
        buffer_size: float,
        capacity: float,
        size: float,
        strings: array(string),
        results: array(ScoredResult.t),
        available: float,
        selection: float,
        workerCount: int,
    };
};

external _test: unit => unit = "test_match";
let test = () => _test();
test()

external _init: unit => Choices.t = "fzy_choices_init";
external _add: (Choices.t, string) => Choices.t = "fzy_choices_add";
external _search: (Choices.t, string) => Choices.t = "fzy_choices_search";
external _get: (Choices.t, int) => string = "fzy_choices_get";
external _getscore: (Choices.t, int) => float = "fzy_choices_getscore";

let init = () => _init();
let add = (choices, choice) => _add(choices, choice);
let search = (choices, search) => _search(choices, search);
let get = (choices, n) => _get(choices, n);
let getScore = (choices, n) => _getscore(choices, n);
