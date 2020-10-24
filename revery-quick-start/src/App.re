open Revery;
open Revery.UI;
open Revery.UI.Components;

module Styles = {
  open Style;

  let text = [marginTop(24), color(Color.hex(Theme.darkBlue))];
};

Revery.App.initConsole();

Timber.App.enable();
Timber.App.setLevel(Timber.Level.info);

/* Helpers.run(); */

let%component main = () => {
  let%hook { route }: Router.meta = Router.useRoute();
  /* let%hook (count, setCount) = React.Hooks.state(0); */

  /* let increment = () => {
    setCount(count => count + 1);
    Helpers.run();
  }; */
  let style = Style.[
    color(Color.rgb(0.,0.,0.))
  ];

  let content = switch(route) {
  | SelectWindow => <SelectWindow />
  | WindowPreview(id) => <DisplayPreview id=id />
  };

  <View style=style>
    content
  </View>
};

let init = app => {
  /* Revery.App.initConsole();

  Timber.App.enable();
  Timber.App.setLevel(Timber.Level.info); */
  /* Timber.App.setLevel(Timber.Level.perf); */
  /* let style = Styles.[backgroundColor(Color.doubleHex("#17212b"))] */

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

   Window.onSizeChanged(window, ({width, height}) => GlobalState.update(pState => {
     {...pState, size: {width, height }}
   }));

  let _update: Revery.UI.renderFunction = UI.start(window, <main />);
  ();
};

App.start(init);
