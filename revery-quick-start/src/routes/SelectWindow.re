open Revery;
open Revery.UI;
open Revery.UI.Components;

module WindowRow = {
  let style = Styles.[`BackgroundColor(Color.hex("#17212b"))];

  let make = (~name: string, ~id: int, ()) => {
    let text = Printf.sprintf("%s (%d)", name, id);
    let inactiveStyle=Styles.withTextDefault(Styles.[`PaddingRight(10)]);
    /*
      TODO:
      * Make text clickable
      * Set windowID to global state
      * Navigate to the selection window
    */
    <Row>
      <Components.ClickableText
          inactiveStyle
          activeStyle=Style.merge(
            ~source=inactiveStyle,
            ~target=Styles.[Style.color(Colors.blue)]
          )
          fontSize=Styles.fontSmall
          text=text
          onClick={(_) => {
            GlobalState.update((state) => {
              { ...state, selectedWindowId: Some(id) }
            });
            Router.push(AppMenu);
          }}
      />
      <Router.Link to_=WindowPreview(id)>
        <Text
          text="Preview"
          fontSize=Styles.fontSmall
        />
      </Router.Link>
    </Row>
  };
}

let%component make = () => {
  let%hook (windows, setWindows) = Hooks.state(Stubs.getWindows());
  let%hook ({ selectedWindowId }: GlobalState.t, setState) = GlobalState.useState();

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
              onClick={() => setWindows(_ => Stubs.getWindows())}
              fontSize=Styles.fontSmall
              width=100
              height=40
            />
          </Row>
        </Column>
  | xs => {
      let rows = React.listToElement(
        List.map(({ name, id}: Stubs.window) => <WindowRow name=name id={id} />, windows)
      );
      <Column>
        rows
      </Column>
    }
  }}
  </View>
};
