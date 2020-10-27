module Style = Revery.UI.Style;
module Color = Revery.Color;
module Colors = Revery.Colors;

let textDefault = Style.[color(Colors.white)];

let fontSmall = 14.0;

let withTextDefault = style =>
  Style.merge(~source=textDefault, ~target=style);
