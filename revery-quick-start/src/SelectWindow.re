open Revery;
open Revery.UI;
open Revery.UI.Components;

let%component make = () => {
  let%hook ({ selectedWindowId, windowIds }: GlobalState.t, setState) = GlobalState.useState();

  let windows = List.map(id => <Text text={id}/>, windowIds);
  
  let content = switch(windowIds) {
    | [] => <Loading />
    /* | [] => <Text text="Hello"/> */
    | _ => React.listToElement(windows)
  };

  <View>
    <Center>
      {content}
    </Center>
  </View>;
};
