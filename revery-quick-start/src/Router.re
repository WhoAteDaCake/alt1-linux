open  LightRouter;

module  RouterConfig  = {
	/* Define your routes */
	type route =
  | SelectWindow
  | SelectBoundingBox

	/* Set de the default one */
	let  defaultRoute = SelectWindow;
};

include Make(RouterConfig);
