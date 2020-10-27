open Revery;
open Revery.UI;
open Revery.UI.Components;


let tmpImage = () => {
	let name = Random.int(10000) |> string_of_int;
	Filename.temp_file(name, ".jpg");
}
