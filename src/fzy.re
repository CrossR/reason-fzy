module Result = {
  type t = {
    term: string,
    score: float,
    original_index: int,
    positions: array(int),
  };
};

external _searchInArray: (array(string), string, bool) => array(Result.t) =
  "fzy_search_for_item_in_array";
external _searchInList: (list(string), string, bool) => list(Result.t) =
  "fzy_search_for_item_in_list";
let fzySearchArray = (haystack, needle, ~sorted=true, ()) =>
  _searchInArray(haystack, needle, sorted);
let fzySearchList = (haystack, needle, ~sorted=true, ()) =>
  _searchInList(haystack, needle, sorted);

let%test_module "fzySearchList " =
  (module
   {
     type expectedResult = {
       term: string,
       positions: list(int),
     };
     let item = (~term, ~positions) => {term, positions};

     let validate =
         (actualItems: list(Result.t), expectedItems: list(expectedResult)) => {
       let compare = (idx, actual: Result.t, expected: expectedResult) =>
         if (String.equal(actual.term, expected.term)) {
           let actualPositionList = actual.positions |> Array.to_list;
           if (actualPositionList == expected.positions) {
             true;
           } else {
             prerr_endline(
               Printf.sprintf(
                 "Expected positions and actual positions differ at index %d for term %s",
                 idx,
                 actual.term,
               ),
             );
             false;
           };
         } else {
           prerr_endline(
             Printf.sprintf(
               "Actual term %s and expected term %s don't match at index %d",
               actual.term,
               expected.term,
               idx,
             ),
           );
           false;
         };

       let rec loop = (idx, actual, expected) => {
         switch (actual, expected) {
         | ([], []) => true
         | ([actualHd, ...actualTail], [expectedHd, ...expectedTail]) =>
           if (compare(idx, actualHd, expectedHd)) {
             loop(idx + 1, actualTail, expectedTail);
           } else {
             false;
           }
         | _ => false
         };
       };

       loop(0, actualItems, expectedItems);
     };

     let fzySearchBoth = (items, needle) => {
       let itemsList = fzySearchList(items, needle, ());
       let itemsArray = fzySearchArray(Array.of_list(items), needle, ());
       (itemsList, itemsArray |> Array.to_list);
     };

     let%test "simple test" = {
       let items = ["a", "b"];
       let needle = "a";
       let expected = [item(~term="a", ~positions=[0])];

       let (itemsList, itemsArray) = fzySearchBoth(items, needle);
       validate(itemsList, expected) && validate(itemsArray, expected);
     };
     let%test "before limit test" = {
       let almostPastLengthStr = String.make(1023, 'a');
       let items = [almostPastLengthStr];
       let needle = "a";
       let expected = [item(~term=almostPastLengthStr, ~positions=[0])];

       let (itemsList, itemsArray) = fzySearchBoth(items, needle);
       validate(itemsList, expected) && validate(itemsArray, expected);
     };
     let%test "at limit test" = {
       let atLimitStr = String.make(1024, 'a');
       let items = [atLimitStr];
       let needle = "a";
       let expected = [item(~term=atLimitStr, ~positions=[0])];

       let (itemsList, itemsArray) = fzySearchBoth(items, needle);
       validate(itemsList, expected) && validate(itemsArray, expected);
     };

     let%test "past limit test" = {
       let pastLimitStr = String.make(1025, 'a');
       let limitStr = String.make(1024, 'a');

       let items = [pastLimitStr];
       let needle = "a";
       let expected = [item(~term=limitStr, ~positions=[0])];

       let (itemsList, itemsArray) = fzySearchBoth(items, needle);
       validate(itemsList, expected) && validate(itemsArray, expected);
     };
   });
