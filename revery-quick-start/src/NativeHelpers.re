type window = {
  name: string,
  id: int
};

external get_windows: unit => list(int) = "get_windows"

let getWindows = (): list(window) => {
  /* In case we ever want renaming, or something like that ?*/
  let list = get_windows();
  List.fold_left((ls, id) => [{ name: "RuneScape", id: id }, ...ls], [], list);
};