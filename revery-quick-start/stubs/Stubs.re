type window = {
  name: string,
  id: int,
};

external get_windows: unit => list(int) = "get_windows";

let getWindows = (): list(window) => {
  /* In case we ever want renaming, or something like that ?*/
  let list = get_windows();
  List.fold_left((ls, id) => [{name: "RuneScape", id}, ...ls], [], list);
};

external screenshot_window: (string, int, int, int) => int =
  "screenshot_window";

let screenshotWindow = (~width=0, ~height=0, windowId, fileName) =>
  if (!Helpers.contains(fileName, "jpg")) {
    Error("Image bust be a jpg type");
  } else {
    let retcode = screenshot_window(fileName, windowId, width, height);
    switch (retcode) {
    | 0 => Ok(fileName)
    | 1 => Error("No display found")
    | 2 => Error("No window attributes found")
    | n => Error(Printf.sprintf("Unknown return code: [%d]", n))
    };
  };
