# OpenStarbound

OpenStarbound is a fork of the [Starbound 1.4.4 source code](https://archive.org/details/starbound_source_code), with bug fixes, new features and performance improvements. Contributions are welcome.

**You must own Starbound.** Its commercial game assets are not included.

## Installation

1. Download the [latest stable release](https://github.com/OpenStarbound/OpenStarbound/releases/latest). Use the Windows installer or extract the client/server archive for your platform.
2. Copy `assets/packed.pak` from your Starbound installation into OpenStarbound's `assets/` directory.
3. Optionally copy `storage/` for saves/settings, `mods/` for local mods and `assets/user/` for instrument songs. Launch with Steam open to load subscribed Workshop mods.

[Nightly builds](https://github.com/OpenStarbound/OpenStarbound/actions?query=branch%3Amain) may be less stable than releases. OpenStarbound is a separate installation from vanilla Starbound.

## Features and documentation

- Multithreaded lighting with higher color range and additive point lights.
- Asset-load Lua scripts and `.patch.lua` patches for JSON and images.
- Positional Opus voice chat, including on vanilla servers.
- Custom keybindings, character swapping, multiple fonts and an instrument volume slider.
- Improved mod compatibility, inventory recovery and world-file storage.
- Client-side tile placement prediction; custom tile collision requires an OpenStarbound server.

Use `/help` for commands. See the [command reference](assets/opensb/help.config.patch), [Lua API](doc/lua/openstarbound) and [JSON extensions](doc/json/openstarbound). Many mods targeting StarExtensions work with OpenStarbound; StarExtensions is deprecated.

## Lua runtime

**Lua 5.5.1** retains compound assignment, safe navigation, `math.round` and `math.copysign`. Migration notes:

- `global` is reserved; rename identifiers or use `_ENV["global"]`.
- Numeric loop variables and the first variable of generic loops are read-only. Use a body-local copy when needed. OSB repairs vanilla `/scripts/actions/flying.lua` in memory; mod scripts still need migration.
- Rebuild Lua 5.3 bytecode. Finalizer errors become warnings, and `math.random` sequences may change.
- Text templates use `<name>` tags and `<<` for a literal `<`, not brace-format placeholders.

## Building

Requires **C++23**, **CMake 3.28+**, **Ninja 1.11+**, and [xclang](https://github.com/clice-io/xclang) **23.1.2.9** with bundled libc++ and `std` module sources. Windows, Linux, macOS and CI use this toolchain; GCC, Apple Clang and MSVC builds are unsupported. C++ dependencies must use the same compiler/runtime; Windows uses the MinGW ABI.

Lua is fetched using a [version/SHA-256 lock](cmake/dependencies/lua.json) and [project patch](cmake/patches/lua-openstarbound.patch), not vendored. Git is required for this path; Nix supplies prepatched sources. For offline builds, pass `-DFETCHCONTENT_SOURCE_DIR_STAR_LUA=/path/to/prepatched-lua` and `-DFETCHCONTENT_FULLY_DISCONNECTED=ON`; the source root must contain `src/`.

### Nix on Linux

Supported systems: x86_64 and ARM64. From the repository root:

```sh
nix build --cores 16 --max-jobs 1
OPENSTARBOUND_ASSETS=/path/to/Starbound/assets ./result/bin/starbound
```

Packages: `.#server` (headless), `.#full` (Qt tools and Steam/Discord), `.#asserts` and `.#sanitized`. Linux ARM64 supports SDK-free variants only.

Use `nix develop` or `nix develop .#full` for the matching development environment. Launchers keep writable data under `$XDG_DATA_HOME/openstarbound` (default `~/.local/share/openstarbound`); override paths with `OPENSTARBOUND_RUNTIME_DIR`, `OPENSTARBOUND_MODS`, `OPENSTARBOUND_STORAGE` or `OPENSTARBOUND_LOGS`.

### Native CMake presets

Bootstrap [vcpkg](https://github.com/microsoft/vcpkg#quick-start) **outside this repository** and set `VCPKG_ROOT` to its absolute path. Keep the project's overlay ports and triplets.

| Platform | Preset | Additional requirements |
|---|---|---|
| Linux x86_64 | `linux-release` | C-ABI/window-system development packages listed in [CI](.github/workflows/build.yml) |
| Linux ARM64 | `linux-arm-release` | Same prerequisites; Steam/Discord SDKs disabled |
| Windows x64 | `windows-release` | MSYS2 UCRT64 `make`; set `MSYS2_ROOT` and add `ucrt64/bin` to `PATH` |
| macOS Intel | `macos-release` | Xcode Command Line Tools and a macOS 14-capable SDK |
| macOS Apple Silicon | `macos-arm-release` | Same SDK requirements |

Linux/macOS, from the repository root; choose your preset from the table:

```sh
export VCPKG_ROOT=/absolute/path/to/vcpkg
export VCPKG_MAX_CONCURRENCY=16
cmake -DXCLANG_INSTALL_DIR="$PWD/build/compiler-sdk" -P scripts/ci/install-xclang.cmake
export XCLANG_ROOT="$PWD/build/compiler-sdk/xclang"
# macOS also requires: export SDKROOT="$(xcrun --sdk macosx --show-sdk-path)"
cd source
cmake --preset linux-release
cmake --build --preset linux-release --parallel 16
ctest --preset linux-release
```

Windows PowerShell, after setting `VCPKG_ROOT` and `MSYS2_ROOT`:

```powershell
$env:VCPKG_MAX_CONCURRENCY = "16"
cmake "-DXCLANG_INSTALL_DIR=$PWD/build/compiler-sdk" -P scripts/ci/install-xclang.cmake
$env:XCLANG_ROOT = "$PWD/build/compiler-sdk/xclang"
cd source
cmake --preset windows-release
cmake --build --preset windows-release --parallel 16
ctest --preset windows-release
```

Presets write binaries to `dist/`. Supply your owned assets, the platform's `sbinit.config` and vendor runtime libraries before launching; see [Linux](scripts/ci/linux), [Windows](scripts/ci/windows) and [macOS](scripts/ci/macos) assembly scripts. Generated distributions never include commercial `packed.pak`.

### Cross-compiling Windows from Linux

Requires Nix, bootstrapped vcpkg, CMake/Ninja/GNU make and the Linux host prerequisites above. From the repository root:

```sh
nix develop .#full --cores 16 --max-jobs 1
export OSB_NATIVE_COMPILER_ROOT="$(dirname "$(dirname "$(command -v clang)")")"
cmake -DXCLANG_INSTALL_DIR="$PWD/build/compiler-sdk" -P scripts/ci/install-xclang.cmake
export XCLANG_ROOT="$PWD/build/compiler-sdk/xclang"
export VCPKG_ROOT=/absolute/path/to/vcpkg
export VCPKG_MAX_CONCURRENCY=16
export CMAKE_BUILD_PARALLEL_LEVEL=16
cd source
cmake --preset windows-cross
cmake --build --preset windows-cross --parallel 16
```

Host Qt generators and Windows libraries are built separately with xclang/libc++. To run CTest, configure with `-DCMAKE_CROSSCOMPILING_EMULATOR=wine`; use Wine 11+ or a native Windows runner. Preserve executable/PDB pairs for debug diagnostics. macOS cross-compilation remains unverified and requires a separately obtained, authorized Apple SDK; xclang does not provide it.

### Tests

CTest presets run asset-free core tests. Real-asset game tests require your Starbound assets:

```sh
OPENSTARBOUND_ASSETS=/path/to/Starbound/assets nix run .#game-tests
# Optional assertion/sanitizer gates:
OPENSTARBOUND_ASSETS=/path/to/Starbound/assets nix run .#asserts-tests
OPENSTARBOUND_ASSETS=/path/to/Starbound/assets OPENSTARBOUND_TEST_JOBS=4 nix run .#sanitized-tests
```

Set `OPENSTARBOUND_TEST_JOBS` to bound parallel cases (default `1`); each discovered case gets a separate process, runtime and log/JSON report. Reports default to `./game-tests-report`; override with `OPENSTARBOUND_TEST_OUT`. Keep commercial assets out of commits and distributions.

### Development notes

Project C++ uses named modules, `import std;` and `std::expected`, with RTTI and C++ exceptions disabled. Preserve the same static C++ runtime across dependencies.

Containers own their storage; shared algorithms and codecs use `SequenceContainer`, `MapContainer` and `SetContainer` constraints. Use `keyView()`/`valueView()` for nonmutating map traversal and `std::span`/`std::mdspan` for contiguous borrowed data. Views must not outlive owners or invalidation; retain snapshots across mutation or callbacks, and materialize Lua arguments before invoking scripts.

`BTree<Storage>` owns a `BTreeStorage` implementation. Lua scripts use fixed-address, nonmovable hosts with value-owned features; preserve each host's initialization, storage-snapshot and shutdown ordering.

Synchronization uses standard mutexes, guards and condition variables; clock acquisition uses `std::chrono`. Checked native thread lifecycle, writer-preferring read/write locking and durable file I/O remain explicit. Convert raw clock ticks using their reported frequencies. TCP close must wake readiness waits promptly while allowing already-buffered complete packets to drain.

Numeric lexical conversion uses `std::from_chars` and requires full tokens. Integer overflow leaves existing destinations unchanged; value-initialized conversions yield `0`, including oversized `uint64_t` inputs that previously wrapped. JSON numbers use bounded UTF-32 stack buffers with `std::format_to_n`, preserving default formatting.

Fixed input/log-level name tables are `constexpr` arrays. Use `enumName<Names>`, `enumValue<Names>` or `maybeEnumValue<Names>` for checked lookup; returned names borrow static storage. Runtime text still undergoes UTF-8 validation and case-insensitive comparison.

For clangd 23, run `nix develop -c bash scripts/ide/configure-clangd.sh`. Use `--experimental-modules-support`, `--compile-commands-dir=/absolute/path/to/OpenStarbound/build/clangd` and `--query-driver=/nix/store/*-xclang-wrapper-*/bin/clang++,/nix/store/*-xclang-wrapper-*/bin/clang`; set `LC_ALL=C` and `LANGUAGE=C`.

Reuse compatible build trees for incremental work. Use separate native/cross checkouts and fresh trees after compiler, ABI or sanitizer changes; adjust parallelism to available memory.

## Community

[Issues and contributions](https://github.com/OpenStarbound/OpenStarbound/issues) are welcome.

<details>
<summary>Discord invite</summary>

Reverse the text in this image to obtain the invite code (spam prevention):

<img width="380" height="70" alt="Reversed Discord invite code" src="https://github.com/user-attachments/assets/9db11a89-0c32-4034-8555-68b54e5918a8" />

</details>
