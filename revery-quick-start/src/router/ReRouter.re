open Revery;
open UI;

module Log = (val Log.withNamespace("Router"))

module type RouterConfig = {
  type route;
  let defaultRoute: route;
  let toString: route => string;
  let clone: route => route;
};

exception NoHistory(string);

let apply1 = (arg) => f => f(arg);

module Make = (RouterConfig: RouterConfig) => {
  include RouterConfig;

  type meta = { route: route, push: route => unit, pop: unit => unit };

  module Store = {
    let subscriptions = ref([]);
    let history = ref([defaultRoute]);
    let currentRoute = ref(defaultRoute);

    let length = () => List.length(history^);

    let push = (route) => {
      history := [route, ...history^];
      currentRoute := route;
      Log.infof(m => m("Pushed new route: [%s]", toString(route)));
      List.iter(f => f(route), subscriptions^);
    };

    let pop = () => {
      /* Return previous route, so we can notify subscribers*/
      let (cr, ncr, xs) = switch (history^) {
      | [cr, pr, ...xs] => (cr, pr, xs)
      | _ => raise(NoHistory("Can't pop with no previous history"))
      };
      
      Log.infof(m => m("Returning from [%s] to [%s]", toString(cr), toString(ncr)));
      history := [ncr, ...xs];
      currentRoute := ncr;
      List.iter(f => f(ncr), subscriptions^);
    }


    let subscribe = cb => {
      subscriptions := [cb, ...subscriptions^];
      () => {
        subscriptions := List.filter(c => c !== cb, subscriptions^);
      };
    };
  };


  type action =
  | SetRoute(route);

  type state = { route: route };

  let reducer = (action, _state) => {
    switch (action) {
    | SetRoute(route) => { route: route }
    };
  };

  let useRoute = (~name="Anonymous", ()) => {
    let%hook (state, dispatch) =
      Hooks.reducer(
        ~initialState={ route: Store.currentRoute^ },
        reducer,
      );
    /*
      There seems to be an issue with the scope of state. If we try to access it directly
      it will shows the state value to be same as the new route.
      Relevant link:
      https://github.com/revery-ui/revery/blob/master/src/UI_Hooks/Tick.re

     */
    let%hook stateVal = Hooks.ref(state);
    stateVal := state;

    let%hook _ =
      Hooks.effect(
        OnMount,
        () => {
          let unsubscribe =
            Store.subscribe((newRoute) => {
              let pRoute = (stateVal^).route;
              if (newRoute != pRoute) {
                Log.infof(m => m("(%s) Route change [%s] -> [%s]", name, toString(pRoute), toString(newRoute)));
                dispatch(SetRoute(newRoute))
              }
            });
          Some(unsubscribe);
        },
      );
    let result: meta = { route: state.route, push: Store.push , pop: Store.pop };
    result;
  };

  module Link = {
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

  module Back = {
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
};
