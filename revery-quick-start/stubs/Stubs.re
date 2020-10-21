type window = {
  name: string,
  id: int
};

external get_windows: unit => list(int) = "get_windows";

let getWindows = (): list(window) => {
  /* In case we ever want renaming, or something like that ?*/
  let list = get_windows();
  List.fold_left((ls, id) => [{ name: "RuneScape", id: id }, ...ls], [], list);
};

external screenshot_window: string => int => int => int => int = "screenshot_window";

let screenshotWindow = (windowId) => {
  let fileName = "test.png"
  let retcode = screenshot_window(fileName, windowId, 0, 0);
  switch (retcode) {
  | 0 => Ok(fileName)
  | 1 => Error("No display found")
  | 2 => Error("No window attributes found")
  | _ => Error("Unknown return code")
  }
};