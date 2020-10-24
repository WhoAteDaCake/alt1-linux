open ReRouter;

module RouterConfig  = {
	/* Define your routes */
	type route =
  | SelectWindow
	| WindowPreview(int)

	/* Set de the default one */
	let defaultRoute = SelectWindow;

	let toString = route => switch (route) {
	| SelectWindow => "SelectWindow"
	| WindowPreview(id) => Printf.sprintf("WindowPreview(%d)", id)
	};

	let clone = route => switch (route) {
  | SelectWindow => SelectWindow
	| WindowPreview(id) => WindowPreview(id)
	};
};

include Make(RouterConfig);
