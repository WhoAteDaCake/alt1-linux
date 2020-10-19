open Revery;
open Revery.UI;
open Revery.UI.Components;

type test = {
  first: int,
  second: int,
  third: int,
};

external get_windows: unit => list(int) = "get_windows"

module Styles = {
  open Style;

  let text = [marginTop(24), color(Color.hex(Theme.darkBlue))];
};

Revery.App.initConsole();

Timber.App.enable();
Timber.App.setLevel(Timber.Level.info);

/* Helpers.run(); */

let%component main = () => {
  let%hook (route, redirect) = Router.useRoute();
  /* let%hook (count, setCount) = React.Hooks.state(0); */

  /* let increment = () => {
    setCount(count => count + 1);
    Helpers.run();
  }; */
  let style = Style.[
    color(Color.rgb(0.,0.,0.))
  ];

  <View style=style>
      <Router
        render={route => switch(route) {
          | SelectWindow => <SelectWindow />
          | SelectBoundingBox => <Text style=Styles.text text="SelectBoundingBox"/>
        }}
      />
    /* <Row>
      <Clickable onClick={_ => {
        let _ = redirect(SelectBoundingBox);
      }}>
        <Text
          style=Styles.text
          text="Click me"
        />
      </Clickable>
    </Row> */
  </View>
  /* <Center>
    <Padding padding=24>
      <Row>
        <AnimatedText delay={Time.ms(0)} text="Welcome" />
        <AnimatedText delay={Time.ms(500)} text="to" />
        <AnimatedText delay={Time.ms(1000)} text="Revery" />
      </Row>
    </Padding>
    <SimpleButton text="Increment" onClick=increment />
    <Text
      fontSize=16.
      style=Styles.text
      text={"Times clicked: " ++ string_of_int(count)}
    />
  </Center>; */
};

let init = app => {
  /* Revery.App.initConsole();

  Timber.App.enable();
  Timber.App.setLevel(Timber.Level.info); */
  /* Timber.App.setLevel(Timber.Level.perf); */

  let obj = get_windows();
  List.iter(l => print_endline(string_of_int(l)), obj);
  if (List.length(obj) == 0) {
    print_endline("No runescape windows found")
  } else {
    print_endline("Windows found")
  }

  let win =
    App.createWindow(
      app,
      "Hello Revery!",
      ~createOptions=
        WindowCreateOptions.create(
          ~backgroundColor=Color.hex(Theme.lightBlue),
          ~width=512,
          ~height=384,
          (),
        ),
    );

  let _update: Revery.UI.renderFunction = UI.start(win, <main />);
  ();
};

App.start(init);
