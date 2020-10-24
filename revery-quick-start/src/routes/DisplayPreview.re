open Revery;
open Revery.UI;
open Revery.UI.Components;

type state =
| Loading
| Loaded(string)
| Failed(string)

let fileName = "/home/augustinas/projects/github/alt1-linux/revery-quick-start/test.jpg"

let%component make = (~id: int, ()) => {
  let%hook (state, setState) = Hooks.state(Loading);
  let%hook ({ size }: GlobalState.t, update) = GlobalState.useState();

  let barSize = 20;
  let windowPadding = 20;
  let headerPadding = 10;
  let height = (size.height - barSize - (windowPadding * 2) - headerPadding);

  let%hook _ = Hooks.effect(
    OnMount,
    () => {
      let result = Stubs.screenshotWindow(~width=size.width, ~height=height, id, fileName);
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
    <View style=Style.[`Padding(windowPadding)]>
      <Column>
        <Row>
          <Text text="Loading" fontSize=Styles.fontSmall /> 
        </Row>
      </Column>
    </View>
  | Failed(msg) => <Text text=Printf.sprintf("Failed to screenshot: \n %s", msg) />
  | Loaded(file) =>
    <View style=Style.[`Padding(windowPadding)]>
      <Column>
          <View
            style=Style.[
              `PaddingBottom(headerPadding),
              flexDirection(`Row),
              alignItems(`Stretch),
              justifyContent(`FlexStart)
            ]
          >
            <Router.Back render={(_) => <Text text="Back" fontSize=Styles.fontSmall /> } />   
          </View>
        <Row>
          <Image height=height width=size.width src={`File(fileName)} quality=`medium />
        </Row>
      </Column>
    </View>
      
      /* <View
        style=Style.[
          flexDirection(`Column),
          flexGrow(1),
          justifyContent(`Center),
          alignItems(`Center),
        ]>
        <View style=Style.[padding(8)]>
          <Image height=height width=size.width src={`File(fileName)} quality=`medium />
        </View>
      </View> */
  }
};