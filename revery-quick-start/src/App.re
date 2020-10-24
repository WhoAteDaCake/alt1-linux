open Revery;
open Revery.UI;
open Revery.UI.Components;

module Styles = {
  open Style;

  let text = [marginTop(24), color(Color.hex(Theme.darkBlue))];
};

let%component main = () => {
  let%hook { route }: Router.meta = Router.useRoute(~name="App", ());

  let content = switch(route) {
  | SelectWindow => <SelectWindow />
  | WindowPreview(id) => <DisplayPreview id=id />
  };

  <View> content </View>
};

let init = app => {
  Revery.App.initConsole();

  Timber.App.enable();
  Timber.App.setLevel(Timber.Level.info);

  let window =
    App.createWindow(
      app,
      "Hello Revery!",
      ~createOptions=
        WindowCreateOptions.create(
          ~backgroundColor=Color.hex("#17212b"),
          ~width=Config.defaultSize.width,
          ~height=Config.defaultSize.height,
          (),
        )
    );

  let _ = Window.onSizeChanged(window, ({width, height}) => GlobalState.update(pState => {
    {...pState, size: {width, height }}
  }));

  let _update: Revery.UI.renderFunction = UI.start(window, <main />);
  ();
};

App.start(init);
