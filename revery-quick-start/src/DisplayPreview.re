open Revery;
open Revery.UI;
open Revery.UI.Components;

type state =
| Loading
| Loaded(string)
| Failed(string)

let fileName = "/home/augustinas/projects/github/alt1-linux/revery-quick-start/test.jpg"

let%component make = (~id: int, ()) => {
  let%hook (state, setState) = Hooks.state(Loading)
  let%hook _ = Hooks.effect(
    OnMount,
    () => {
      let result = Stubs.screenshotWindow(~width=512, ~height=384, id, fileName);
      let nState = switch (result) {
      | Ok(fName) => Loaded(fName)
      | Error(msg) => Failed(msg);
      }
      setState(_ => nState);
      None
    },
  );

  switch (state) {
  | Loading =>
    <View style=Style.[`Padding(20)]>
      <Column>
        <Row>
          <Text text="Loading" fontSize=Styles.fontSmall /> 
        </Row>
      </Column>
    </View>
  | Failed(msg) => <Text text=Printf.sprintf("Failed to screenshot: \n %s", msg) />
  | Loaded(file) =>
      <View
        style=Style.[
          flexDirection(`Column),
          flexGrow(1),
          justifyContent(`Center),
          alignItems(`Center),
        ]>
        <View style=Style.[padding(8)]>
          <Image height=384 width=512 src={`File(fileName)} quality=`medium />
        </View>
      </View>
  }
};