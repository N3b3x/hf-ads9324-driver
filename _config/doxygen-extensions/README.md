# Doxygen Awesome CSS Theme

Vendored [doxygen-awesome-css](https://github.com/jothepro/doxygen-awesome-css) **v2.3.4**
(`doxygen-awesome.css`) so HTML generation does not require a git submodule.

Referenced from `_config/Doxyfile` as `HTML_EXTRA_STYLESHEET`.

To refresh:

```bash
curl -fsSL -o _config/doxygen-extensions/doxygen-awesome-css/doxygen-awesome.css \
  https://raw.githubusercontent.com/jothepro/doxygen-awesome-css/v2.3.4/doxygen-awesome.css
```

Graphviz (`dot`) is optional. Set `HAVE_DOT = YES` in the Doxyfile when `dot` is on PATH
for class and include graphs.
