module Result = {
  type t = {
    term: string,
    score: float,
    positions: array(int),
  };
};

external _searchForItem: (array(string), string) => array(Result.t) =
  "fzy_search_for_item";
let searchForItem = (haystack, needle) => _searchForItem(haystack, needle);
