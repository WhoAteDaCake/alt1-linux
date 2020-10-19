module Style = Revery.UI.Style;
module Color = Revery.Color;

let textDefault =
  Style.[color(Color.rgb(0.0, 0.0, 0.0))];

let fontSmall = 20.0;

let withTextDefault = style => Style.merge(~source=textDefault, ~target=style)