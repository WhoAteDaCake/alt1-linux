open Revery.UI;

module type RouterConfig = {
  type route;
  let defaultRoute: route;
};

exception NoHistory(string);

let apply1 = (arg) => f => f(arg);

module Make = (RouterConfig: RouterConfig) => {
  include RouterConfig;

  type meta = { route: route, push: route => unit, pop: unit => unit };

  module Store = {
    let subscriptions = ref([]);
    let history = ref([defaultRoute]);

    let length = () => List.length(history^);

    let getRoute = () => switch(history^) {
    | [cr, ...xs] => cr
    | _ => raise(NoHistory("Should never be in this history state"))
    };

    let push = (route) => {
      history := [route, ...history^];
      subscriptions^ |> List.iter(apply1(route));
    };

    let pop = () => {
      /* Return previous route, so we can notify subscribers*/
      let (ncr, xs) = switch (history^) {
      | [cr, pr, ...xs] => (pr, xs)
      | _ => raise(NoHistory("Can't pop with no previous history"))
      };
      history := [ncr, ...xs];
      subscriptions^ |> List.iter(apply1(ncr));
    }


    let subscribe = cb => {
      subscriptions := List.append(subscriptions^, [cb]);
      () => {
        subscriptions := List.filter(c => c !== cb, subscriptions^);
      };
    };
  };


  let useRoute = () => {
    let%hook (route, setState) = Hooks.state(Store.getRoute());

    let%hook _ =
      Hooks.effect(
        OnMount,
        () => {
          let unsubscribe =
            Store.subscribe((newRoute) => {setState(_ => newRoute)});
          Some(unsubscribe);
        },
      );
    let result: meta = { route, push: Store.push , pop: Store.pop };
    result;
  };

  module RouterLink = {
    let make = (~children, ~to_, ~onClick=?, ~style=[], ()) => {
      <Components.Clickable
        onClick={_ => {
          let _ = switch (onClick) {
          | Some(c) => c()
          | None => ()
          };
          Store.push(to_);
        }}
        style
      >
        children
      </Components.Clickable>;
    };
  };

  module RouterBack = {
    let%component make = (~render, ~onClick=?, ~style=[], ()) => {
      let%hook (disabled, setState) = Hooks.state(Store.length() == 0);
      let%hook _ =
        Hooks.effect(
          OnMount,
          () => {
            let unsubscribe =
              Store.subscribe((_) => {
                setState(_ => Store.length() == 0);
              });
            Some(unsubscribe);
          },
        );
      let content = render(disabled);
      <Components.Clickable
        onClick={_ => {
          let _ = switch (onClick) {
          | Some(c) => c()
          | None => ()
          };
          Store.pop();
        }}
        style
      >
        content
      </Components.Clickable>;
    };
  };

  let subscribe = Store.subscribe;
  let push = Store.push;
  let pop = Store.pop;
  let getRoute = Store.getRoute;
};