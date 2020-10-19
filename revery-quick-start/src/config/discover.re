open Configurator.V1;

let default_config : Pkg_config.package_conf = {
    libs: [],
    cflags: [],
};

let combine_configs = (pc, items) => List.fold_left((cfg: Pkg_config.package_conf, package) => {
  switch (Pkg_config.query(pc, ~package=package)) {
  | None => cfg
  | Some(conf) => {
      libs: cfg.libs @ conf.libs,
      cflags: cfg.cflags @ conf.cflags,
    }
  }
}, default_config, items);

let get_linux_config = (t) => {
  switch (Pkg_config.get(t)) {
  | None => default_config
  | Some(pc) => combine_configs(pc, ["opencv4", "tesseract", "x11"])
  };
}

main(~name="discover", t => {
  /* Only supports Linux at the moment*/
  let conf = get_linux_config(t);
  /* write_sexp("flags.sexp", conf.flags); */
  Flags.write_sexp("c_flags.sexp", conf.cflags);
  Flags.write_sexp("c_library_flags.sexp", conf.libs);
});
