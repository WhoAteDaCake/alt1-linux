/* Check if s1 contains substring s2 */
let contains = (s1, s2) => {
  let re = Str.regexp_string(s2);
  try ({Str.search_forward(re,s1,0) |> ignore; true}) {
  | Not_found => false
  }
};