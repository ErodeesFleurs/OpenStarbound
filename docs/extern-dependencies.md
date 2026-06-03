# Vendored Dependency Notes

The remaining files under `source/extern` are still tied to project code or a
build option:

- `fast_float.h` is used by `StarLexicalCast.hpp` for floating point and integer
  parsing. Keep it vendored until standard-library parsing is benchmarked across
  the supported GCC and Clang versions.
- `rpmalloc.c`, `rpmalloc.h`, and `rpnew.h` are only compiled when
  `STAR_USE_RPMALLOC` is enabled. Windows presets still use that allocator path.
- `imgui_lua_bindings.*` is included by `StarLua.cpp`, so it is still part of the
  core Lua integration rather than a removable GUI-only dependency.
- Lua, xxHash, and curve25519 sources are part of the core runtime surface.

Dependencies that no longer had project references, such as `fmt` and
`tinyformat`, have been removed from the vendored tree.
