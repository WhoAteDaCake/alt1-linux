type t = {
  selectedWindowId: option(int),
  windowIds: list(string)
};

let default = { selectedWindowId: None, windowIds: []};

include State.Impl({ type nonrec t = t; let default = default;});