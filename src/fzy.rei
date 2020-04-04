module Result: {
  type t = {
    term: string,
    score: float,
    positions: array(int),
  };
};

let fzySearchArray: (array(string), string, bool) => array(Result.t);
let fzySearchList: (list(string), string, bool) => list(Result.t);
