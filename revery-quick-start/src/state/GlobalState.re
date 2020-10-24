type t = {
  selectedWindowId: option(int),
  size: Types.size
};

let default = { selectedWindowId: None, size: Config.defaultSize };

include State.Impl({ type nonrec t = t; let default = default;});