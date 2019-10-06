type t;

external _test: unit => unit = "test_match";
let test = () => _test();
test()

external _init: unit => t = "fzy_choices_init";
external _add: (t, string) => t = "fzy_choices_add";
external _search: (t, string) => t = "fzy_choices_search";
external _get: (t, int) => string = "fzy_choices_get";
external _getscore: (t, int) => float = "fzy_choices_getscore";

let init = () => _init();
let add = (choices, choice) => _add(choices, choice);
let search = (choices, search) => _search(choices, search);
let get = (choices, n) => _get(choices, n);
let getScore = (choices, n) => _getscore(choices, n);
