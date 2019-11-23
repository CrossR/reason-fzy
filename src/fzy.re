module Result = {
  type t = {
    term: string,
    score: float,
    positions: array(int),
  };
};

external _searchInArray: (array(string), string) => array(Result.t) = "fzy_search_for_item";
external _searchInList: (list(string), string) => list(Result.t) = "fzy_search_for_item";
let fzySearchArray = (haystack, needle) => _searchInArray(haystack, needle);
let fzySearchList = (haystack, needle) => _searchInList(haystack, needle);
