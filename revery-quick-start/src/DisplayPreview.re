open Revery;
open Revery.UI;
open Revery.UI.Components;

type state =
| Loading
| Loaded(string)
| Failed(string)

let%component make = (~id: int, ()) => {
  let%hook (state, setState) = Hooks.state(Loading)
  let%hook _ = Hooks.effect(
    OnMount,
    () => {
      let result = NativeHelpers.screenshotWindow(id);
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
    <Text text="Loaded" />
  }
};