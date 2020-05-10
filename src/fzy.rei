module Result: {
  type t = {
    term: string,
    score: float,
    original_index: int,
    positions: array(int),
  };
};

let fzySearchArray:
  (array(string), string, ~sorted: bool=?, unit) => array(Result.t);
let fzySearchList:
  (list(string), string, ~sorted: bool=?, unit) => list(Result.t);
