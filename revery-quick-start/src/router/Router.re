open ReRouter;

module RouterConfig  = {
	/* Define your routes */
  type route =
  | SelectWindow
  | WindowPreview(int)
  | AppMenu
	| HallTracker;


  /* Set de the default one */
  let defaultRoute = SelectWindow;

  let toString = route => switch (route) {
  | SelectWindow => "SelectWindow"
  | WindowPreview(id) => Printf.sprintf("WindowPreview(%d)", id)
  | AppMenu => "AppMenu"
	| HallTracker => "HallTracker"
  };
};

include Make(RouterConfig);
