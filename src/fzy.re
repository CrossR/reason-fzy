let _ = print_endline("Hello");

external _test: unit => unit = "test_match"

let test = () => _test()

test()
