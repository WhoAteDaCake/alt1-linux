open Revery;
open Revery.UI;
open Revery.UI.Components;

type windowData = {
  name: string,
  id: string
};

module Log = (val Log.withNamespace("Revery.Application"));

let runCmd = (cmd) => {
  let (ic, oc) = Unix.open_process(cmd);
  let all_input = ref([]);

  try (
    while(true) {
      all_input := [input_line(ic), ...all_input^]; 
    }
  ) {
  | End_of_file => {
      close_in(ic);
      close_out(oc);
    }
  }
  all_input^;
}

let screenshot = (windowId) => {
  let fileName = Printf.sprintf("%s/%i.png",Filename.get_temp_dir_name(), Unix.time() |> int_of_float);
  let _ = runCmd(Printf.sprintf("import -window %s %s", windowId, fileName));
  fileName;
};

let run = () => {
  let output = runCmd("wmctrl -l");
  let windows = List.map(row => {
    let items = String.split_on_char(' ', row);
    let id = List.hd(items);
    let name = items |> List.rev |> List.hd;
    { name, id };
  }, output); 
  
  List.iter((row) => {
    Log.infof(m => m("Window: %s (%s)", row.name, row.id));
  }, windows);
  Log.info("Finished");

  let fileName = screenshot("0x0440000b");
  Log.info(fileName);
  /*try
    while true do
      all_input := input_line oc :: !all_input
    done
  with
    End_of_file ->
    (* Just an example, you can do something else here *)
    close_in ic;
    List.iter print_endline !all_input */
}