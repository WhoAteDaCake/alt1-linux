open Revery;
open Revery.UI;
open Revery.UI.Components;

module Log = (val Log.withNamespace("Routes/DisplayPreview"))

type state =
| Loading
| Loaded(string)
| Failed(string)

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
      let fileName = Helpers.tmpImage();
      Log.infof(m => m("Generated file for preview [%s]", fileName));
      let result = Stubs.screenshotWindow(~width=size.width, ~height=height, id, fileName);
      let nState = switch (result) {
      | Ok(fName) => Loaded(fName)
      | Error(msg) => {
        Log.errorf(m => m("Failed to get a screenshot: \n %s", msg));
        Failed(msg)
      };
      }
      setState(_ => nState);
      Some(() => {
        Log.infof(m => m("Screenshot preview removed [%s]", fileName));
        Sys.remove(fileName)
      }); 
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
  | Loaded(fileName) =>
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
