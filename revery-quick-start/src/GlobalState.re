type t = {
  selectedWindowId: option(string),
  windowIds: list(string)
};

let default = { selectedWindowId: None, windowIds: []};

include State.Impl({ type nonrec t = t; let default = default;});