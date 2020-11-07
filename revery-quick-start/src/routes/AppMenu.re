open Revery;
open Revery.UI;
open Revery.UI.Components;

type path = {
  route: Router.route,
  name: string
};

let paths: list(path) = [
  {
    name: "Hall tracker",
    route: HallTracker
  }
]

let make = () => {
  <Column>
    <Row></Row>

  </Column>
};
