module Result = {
  type t = {
    term: string,
    score: float,
    positions: array(int),
  };
};

external _searchInArray: (array(string), string) => array(Result.t) = "fzy_search_for_item_in_array";
external _searchInList: (list(string), string) => list(Result.t) = "fzy_search_for_item_in_list";
let fzySearchArray = (haystack, needle) => _searchInArray(haystack, needle);
let fzySearchList = (haystack, needle) => _searchInList(haystack, needle);
