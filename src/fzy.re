type t;

external _searchForItem: (array(string), string) => array(float) = "fzy_search_for_item";
let searchForItem = (haystack, needle) => _searchForItem(haystack, needle);

