open ReRouter;

module RouterConfig  = {
	/* Define your routes */
	type route =
  | SelectWindow
	| WindowPreview(int)

	/* Set de the default one */
	let defaultRoute = SelectWindow;
};

include Make(RouterConfig);
