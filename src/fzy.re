let _ = print_endline("Hello");

external _test: unit => float = "test_match"

let test = () => _test()

test()
