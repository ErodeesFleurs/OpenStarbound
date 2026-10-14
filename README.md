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

## Building

Requires **C++23**, **CMake 3.28+**, **Ninja 1.11+**, and pinned [xclang](https://github.com/clice-io/xclang) **23.1.2.9** with bundled **libc++/libc++abi/libunwind** and its `std` module sources. Nix, presets and CI use this toolchain; Windows uses the MinGW ABI, not MSVC STL.

C++ dependencies and Qt plugins use the same compiler/runtime and static linkage. Do not mix GNU/MSVC C++ objects or pass C++ objects, exceptions or RTTI across independently linked shared libraries. Opaque C-ABI system libraries and vendor SDKs are supported.

### Nix on Linux

Supported systems: x86_64 and ARM64. From the repository root:

```sh
nix build --cores 16 --max-jobs 1
OPENSTARBOUND_ASSETS=/path/to/Starbound/assets ./result/bin/starbound
```

Other packages: `.#server` (headless), `.#full` (Qt tools and Steam/Discord), `.#asserts` and `.#sanitized`. Linux ARM64 supports SDK-free variants only. For the full x86_64 package:

```sh
nix build .#full --cores 16 --max-jobs 1 -o result-full
```

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
OPENSTARBOUND_ASSETS=/path/to/Starbound/assets nix run .#sanitized-tests
```

Reports default to `./game-tests-report`; override with `OPENSTARBOUND_TEST_OUT`. For direct binaries, `-bootconfig` selects a writable runtime configuration: `assetSources` names individual pak/mod paths, while `assetDirectories` scans directories containing those sources. Keep commercial assets out of commits and distribution archives.

### Development notes

- C++23 module interfaces use `.cppm`; implementations use matching `.cpp` companions where needed. Import converted families, including template-only math, containers and algorithms, instead of restoring retired headers; register them in the owning CMake file set and link the complete module provider.
- Import APIs used by declarations and template bodies directly, at file scope before namespaces; private imports are not re-exports. Keep caller-owned textual codecs, such as `StarDataStreamExtra.hpp` and `StarLuaConverters.hpp`, before imports that instantiate their templates. Public project-template specializations need explicit exported redeclarations alongside their primary templates.
- Use `import std;` for standard C++ APIs. `star_std` builds the pinned libc++ `std.cppm` from `libc++.modules.json` once per build tree and propagates through `star_modules`; no PCH or prebuilt BMI is used. Keep C macro/global-name headers and third-party declarations textual. The Linux `sys/cdefs.h` overlay preserves glibc's checked fortify bodies with hidden inline C++ linkage so they can be exported by `std`; include C headers before imports in files that call fortified global functions.
- Formatting uses `std::format` and `std::vformat`, without bundled fmt. `strf`, logging and exception helpers check literals through `std::format_string`; runtime text preserves byte lengths and wraps formatting errors in `FormatException`. Project `std::formatter` adapters accept generic formatting contexts, retain ostream representations with the classic locale and a stack buffer, and use `std::range_formatter` for concrete lists and sets. Export module formatter specializations explicitly; a GMF specialization alone is not reachable by importing consumers. When an inline GMF function needs the specialization first, define it there and export a specialization redeclaration after the module declaration.
- The formatting chain is provided by `star.format`, `star.stream_formatter`, `star.string`, `star.string_view`, `star.json`, `star.lua` and `star.logging`, with their container/hash, Unicode, thread and stream/file dependencies also imported as modules. `StarConfig.hpp` retains platform and assertion macros; retired formatter-chain headers are not compatibility interfaces.
- `FlatHashTable` retains its one-value union, layout, sentinels and explicit placement construction/destruction. The union is named because pinned xclang crashes generating imported anonymous-union member accesses; its empty constructor/destructor must never activate or destroy the stored value.
- For C-ABI strings in trailing storage, pass `std::string_view` with the API-provided byte length, such as `SYMBOL_INFO::Name`/`NameLen`. The bundled libc++ bounds a `char[N]` argument by its declared extent; it is not an unbounded C-string pointer.
- Lua userdata must retain `max_align_t` alignment for stored C++ objects. Liquid network updates saturate their byte representation without clamping physical levels or pressure; hue conversion truncates while signed before byte wrapping. Check a positive remaining tick budget before converting it to unsigned sleep milliseconds.
- Build configurations are `Debug`, `RelWithAsserts`, `RelWithDebInfo` and `Release`; `STAR_DEBUG_SYMBOL_LEVEL` accepts `default`, `minimal` or `full`. Caller compiler flags are preserved.
- Retain compatible objects/BMIs for incremental builds. Use fresh build directories after language-standard, compiler, ABI or sanitizer changes, and separate checkouts when retaining native/cross outputs: in-tree builds share `dist/`. Adjust parallelism to available memory.

## Community

[Issues and contributions](https://github.com/OpenStarbound/OpenStarbound/issues) are welcome.

<details>
<summary>Discord invite</summary>

Reverse the text in this image to obtain the invite code (spam prevention):

<img width="380" height="70" alt="Reversed Discord invite code" src="https://github.com/user-attachments/assets/9db11a89-0c32-4034-8555-68b54e5918a8" />

</details>
