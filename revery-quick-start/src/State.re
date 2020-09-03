open Revery.UI;

module type StateDef = {
  type t;
  
  let default: t;
};

module Impl = (T: StateDef) => {
  let subscriptions = ref([]);
  let prev = ref(T.default);
  let current = ref(T.default);

  let update = newVal => {
    prev := current^;
    current := newVal;

    subscriptions^ |> List.iter(sub => sub(prev^, current^));
    (prev^, current^);
  };

  let subscribe = cb => {
    subscriptions := List.append(subscriptions^, [cb]);

    () => {
      subscriptions := List.filter(c => c !== cb, subscriptions^);
    };
  };

  let useState = () => {
    let%hook (state, setState) =
      Hooks.state(current^);

    let%hook _ =
      Hooks.effect(
        OnMount,
        () => {
          let unsubscribe =
            subscribe((_, newVal) => setState((_) => newVal));
          Some(unsubscribe);
        },
      );

    (state, update);
  };
};
