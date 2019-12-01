module Result: {
  type t = {
    term: string,
    score: float,
    positions: array(int),
  };
};

let fzySearchArray: (array(string), string) => array(Result.t);
let fzySearchList: (list(string), string) => list(Result.t);
