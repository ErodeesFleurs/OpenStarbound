# AGENTS.md - OpenStarbound

## Build system

- **C++20**, CMake >= 3.23, **vcpkg** (required, must set `VCPKG_ROOT` env var).
- All CMake commands run from `source/` or with `-S source` from repo root.
- Dependencies declared in `source/vcpkg.json`, custom triplets in `triplets/`.
- Ninja generator only. No Makefile or Visual Studio generator for real builds (a `windows-release-VS2022` preset exists but CI uses Ninja).
- Build output goes to `dist/` in repo root.

### Presets (configure from `source/`)

```sh
# Linux clang (default CI)
cmake --preset=linux-release-clang
cmake --build --preset=linux-release-clang

# Linux GCC
cmake --preset=linux-release-gcc
cmake --build --preset=linux-release-gcc

# Linux Debug (GCC)
cmake --preset=linux-debug-gcc
cmake --build --preset=linux-debug-gcc

# Windows (MSVC)
cmake --preset=windows-release
cmake --build --preset=windows-release
```

Presets set `BUILD_TESTING=ON`, `STAR_ENABLE_STEAM_INTEGRATION=ON`, `STAR_ENABLE_DISCORD_INTEGRATION=ON`. ARM presets disable Steam/Discord.

### Nix

Nix flake provides `devShells.default` (clang), `.#build-clang`, `.#build-gcc`, `.#test-clang`, `.#test-gcc`, and profiling variants.

## Tests

Two test executables:

| Executable | Label | Requirements |
|---|---|---|
| `core_tests` | `NoAssets` | None |
| `game_tests` | `Assets` | `assets/packed.pak` must exist |

- Test presets (e.g. `ctest --preset=linux-release-clang`) filter to `NoAssets` label only. `game_tests` is never run in CI.
- `game_tests` is registered only if `assets/packed.pak` exists at configure time. CMake auto-generates an `sbinit.config` for it.
- Run all tests manually: `ctest --test-dir build/<preset-dir> --output-on-failure`
- `core_tests` timeout: 120s. `game_tests` timeout: 300s, uses `RESOURCE_LOCK game_assets`.

## Formatting

- `.clang-format` at repo root: BasedOnStyle LLVM, 2-space indent, ColumnLimit 0 (no line length cap).
- Run: `scripts/format-source.sh` (uses `clang-format`, respects `CLANG_FORMAT` env var). Skips `source/extern/`.
- Do not reformat `source/extern/` (vendored external code, tracked in `.gitattributes`).

## Source layout

```
source/
  CMakeLists.txt    # Top-level build definition (RUN FROM HERE)
  CMakePresets.json
  vcpkg.json
  extern/           # Vendored external code (linguist-vendored, no reformatting)
  core/             # Foundation: containers, serialization, threading, scripting
  base/             # Game-adjacent: assets, Lua bindings, protocol
  game/             # Core game logic (shared by client + server)
  platform/         # Platform abstraction interfaces
  application/      # Window/input/rendering layer (GUI only)
  rendering/        # Renderer code
  windowing/        # Panes, widgets, UI system
  frontend/         # Client interface
  client/           # Starbound client
  server/           # Starbound dedicated server
  utility/          # CLI tools (btree_repacker, asset_packer/unpacker, etc.)
  test/             # GoogleTest tests (bundled gtest, not fetched)
  json_tool/        # Qt JSON tool (disabled by default)
  mod_uploader/     # Qt mod uploader (disabled by default)
```

Top-level directories:
- `assets/` - OpenStarbound mod assets (patches, Lua scripts). Packed into `opensb.pak` by build scripts.
- `lib/` - Pre-built library binaries organized by platform (Steam/OSX API, Discord SDK, Windows DLLs).
- `cmake/` - Custom CMake find modules.
- `scripts/` - CI assembly, formatting, distribution scripts.
- `triplets/` - Custom vcpkg triplets.
- `attic/` - Legacy files, ignore.
- `doc/` / `docs/` - Documentation (Lua stubs, JSON format docs, OPENSOURCE.md).

## Code conventions

- 2-space indent, no tabs, LLVM-style braces (no line break before brace).
- Column limit is intentionally off (0).
- Pointers/references left-aligned (`int* p`, `int& r`).
- Compiler-specific preprocessor guards: `STAR_SYSTEM_WINDOWS`, `STAR_SYSTEM_LINUX`, `STAR_SYSTEM_MACOS`, `STAR_COMPILER_GNU`, `STAR_COMPILER_CLANG`, `STAR_COMPILER_MSVC`.
- No precompiled headers when using sccache/ccache (auto-detected).
- Unity builds off by default (`STAR_ENABLE_UNITY_BUILD=OFF`).

## Key gotchas

- **vcpkg is mandatory.** There is no fallback. `VCPKG_ROOT` must be set before running CMake.
- **CMake runs from `source/`, not repo root.** All `--preset` paths are relative to `source/`.
- **No submodules.** External deps are vendored in `source/extern/` or fetched by vcpkg.
- **`dist/` is the output directory** but it's gitignored. Binaries are placed there directly.
- Test presets filter out `game_tests` — if you need those, run `ctest` directly without a preset.
- Build type names are custom: `Debug`, `RelWithAsserts`, `RelWithDebInfo`, `Release`. `RelWithDebInfo` maps to `-O3 -ffast-math -DNDEBUG` on Release configs.
- Linux default linking uses `lld` when available (`STAR_USE_LLD=ON` for clang/gnu).
- Jemalloc is default on Linux; rpmalloc on Windows; neither on macOS.
