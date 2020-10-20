open Revery;
open Revery.UI;
open Revery.UI.Components;

module WindowRow = {
  let style = Styles.[`BackgroundColor(Color.hex("#17212b"))];

  let make = (~name: string, ~id: int, ()) => {
    let text = Printf.sprintf("%s (%d)", name, id);
    <Row>
      <Text
        style=Styles.withTextDefault(Styles.[`PaddingRight(10)])
        fontSize=Styles.fontSmall
        text=text
      />
      <Button
        title="Preview"
        onClick={() => print_endline("Preview")}
        fontSize=Styles.fontSmall
        width=80
        height=20
      />
    </Row>
  };
}

let%component make = () => {
  let%hook (windows, setWindows) = Hooks.state(NativeHelpers.getWindows());
  let%hook ({ selectedWindowId, windowIds }: GlobalState.t, setState) = GlobalState.useState();

  <View style=Style.[`Padding(20)]>
    {switch (windows) {
    | [] =>
        <Column>
          <Row>
            <Column>
              <Text style=Styles.textDefault fontSize=Styles.fontSmall text="No windows found" />
            </Column>
          </Row>
          <Row>
            <Button
              title="Refresh"
              onClick={() => setWindows(_ => NativeHelpers.getWindows())}
              fontSize=Styles.fontSmall
              width=100
              height=40
            />
          </Row>
        </Column>
  | xs => {
      let rows = React.listToElement(
        List.map(({ name, id}: NativeHelpers.window) => <WindowRow name=name id={id} />, windows)
      );
      <Column>
        rows
      </Column>
    }
  }}
  </View>
};
