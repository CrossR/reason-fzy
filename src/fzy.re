module Result = {
  type t = {
    term: string,
    score: float,
    positions: array(int),
  };
};

external _searchInArray: (array(string), string, bool) => array(Result.t) = "fzy_search_for_item_in_array";
external _searchInList: (list(string), string, bool) => list(Result.t) = "fzy_search_for_item_in_list";
let fzySearchArray = (haystack, needle, sorted) => _searchInArray(haystack, needle, sorted);
let fzySearchList = (haystack, needle, sorted) => _searchInList(haystack, needle, sorted);
