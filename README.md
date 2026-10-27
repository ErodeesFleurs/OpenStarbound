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
- Lua-managed HUD and window instances, including a Canvas-painted chat; shared pane, input, text-editing and message APIs allow new interfaces without per-window native hosts.
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

Lua is fetched using a [version/SHA-256 lock](cmake/dependencies/lua.json) and [project patch](cmake/patches/lua-openstarbound.patch), not vendored. Git is required for this path; Nix supplies prepatched sources. Patch files are checked out with LF endings, including on Windows with `core.autocrlf=true`. For offline builds, pass `-DFETCHCONTENT_SOURCE_DIR_STAR_LUA=/path/to/prepatched-lua` and `-DFETCHCONTENT_FULLY_DISCONNECTED=ON`; the source root must contain `src/`.

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
| Linux x86_64 | `linux-release` | Debian/Ubuntu native C SDK (`libc6-dev`) and C-ABI/window-system packages listed in [CI](.github/workflows/build.yml) |
| Linux ARM64 | `linux-arm-release` | Same prerequisites; Steam/Discord SDKs disabled |
| Windows x64 | `windows-release` | MSYS2 UCRT64 `make`; set `MSYS2_ROOT` and add `ucrt64/bin` to `PATH` |
| macOS Apple Silicon | `macos-arm-release` | Xcode Command Line Tools, a macOS 14-capable SDK, and Homebrew `autoconf autoconf-archive automake libtool tarlz` |

On native Linux, the installer adapts only the host target to the complete system C SDK (headers, glibc and startup files), while retaining xclang's bundled static libc++, libc++abi, libunwind and compiler-rt. CI uses Ubuntu 24.04/glibc 2.39; these native builds do **not** retain the archive's glibc 2.17 compatibility baseline. Other targets keep their bundled SDKs. An explicit `OSB_NATIVE_COMPILER_ROOT` preserves an existing native wrapper's SDK instead, including Nix host tools for Windows cross-builds.

All presets chainload the repository's [toolchain wrapper](triplets/toolchain.cmake). On macOS it explicitly selects bundled `ld64.lld` so CMake and vcpkg's Meson ports use the same linker as the compiler driver. macOS C++ module providers and importers share full complex-arithmetic range semantics; the scalar color-conversion accuracy override remains in place.

The pinned `vcpkg-make` overlay preserves default configure options when CMake leaves an empty `EXCLUDE_FILTER` undefined (CMake before 3.31/CMP0174). This keeps installation prefixes, release/debug directories and shared/static linkage options intact for all autotools ports, without per-package staging relocation.

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

macOS tests also need `libsteam_api.dylib` beside the test executable. CI stages vendor runtime libraries after building and before invoking CTest.

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

### Performance benchmarks

Benchmarks are opt-in and retain correctness tests. They require static [Google Benchmark](https://github.com/google/benchmark) 1.9.5+ built with the same xclang/runtime:

```sh
nix build .#benchmarks
./result/bin/core_benchmarks --benchmark_repetitions=7 --benchmark_out=core.json --benchmark_out_format=json
# Optional GUI cases (also includes the headless benchmarks):
nix build .#benchmarks-gui -o result-benchmarks-gui
```

For native presets, configure from `source/` with `-DSTAR_BUILD_BENCHMARKS=ON -DVCPKG_MANIFEST_FEATURES="qt-tools;benchmarks"`. Targets: `core_benchmarks`, `json_benchmarks`, `worker_benchmarks`, `game_benchmarks`, `ownership_benchmarks`, `performance_benchmarks`, Linux/MinGW-only `allocator_benchmarks`, and GUI-only `frontend_benchmarks` / `tile_performance_benchmarks`. Nix development shells include the dependency.

Asset-backed benchmarks and GUI-only `frontend_tests` need owned assets and a fresh, private bootconfig; never point them at saves. For example:

```json
{
  "assetSources": ["/path/to/Starbound/assets/packed.pak", "/path/to/OpenStarbound/assets/opensb"],
  "storageDirectory": "/tmp/osb-benchmark/storage",
  "logDirectory": "/tmp/osb-benchmark/logs"
}
```

```sh
./result/bin/game_benchmarks -bootconfig /path/to/benchmark.config --benchmark_repetitions=7 --benchmark_out=game.json --benchmark_out_format=json
```

Core cases cover JSON encoding, fixed enum lookups and reserved string-key hash maps. Worker cases submit 256 checked tasks per batch, varying capture size, producer results and queue-only versus one-worker execution. Frontend cases exercise dense mixed-size bubble separation and an explicit representative gameplay-key fixture, not the user's bindings or the complete client frame. Game cases cover object/projectile parsing, generated world serialization and genuine nested Lua profile export. Ownership cases cover small-record serialization/readback, real ItemBag stacking/matching, valid PCM WAV and real Ogg cloning/seeking/reading/reclamation, and real image/bytes cache getters and mixed-resource eviction. Asset caches are warmed; fixture setup and validation are untimed. Core/allocator cases report CPU time; worker/frontend/game/ownership cases report wall time, and profile export includes file writes.

`allocator_benchmarks` compares `BlockAllocator`, default PMR and bounded PMR pools. Allocation counters run outside timed samples and measure outstanding C++ `new` requests, not RSS or fragmentation. Production retains `BlockAllocator`: default PMR retained substantially more memory in the representative statistics-list workload; bounded PMR reduced retention but was much slower.

`json_benchmarks` covers numeric/Unicode/comment parsing, formatted array/object construction and output, and path/patch updates. Its 28 cases use fixed complete batches, independent semantic/format checks and byte fingerprints; named output and destruction remain timed. Use repeated samples and the same per-case warmup for both versions (`--benchmark_min_warmup_time=0.1`); keep initial/no-warmup data separate.

`performance_benchmarks` exercises production collision, liquid scheduling, light gathering/propagation, mixed-version client snapshots with real TCP consumers, and queued Lua asset patches. Collision/contact order, liquid state and lighting output have behavior fingerprints; network consumers validate fields and ordering without assuming unordered objects re-encode byte-for-byte. Asset timings include result readback, full collection and destruction. Automatic GC avoids per-item full collections but allows a larger transient managed heap; compare requested-byte peak and post-collection retention separately, not as allocator fragmentation measurements.

`tile_performance_benchmarks` requires a real SDL/OpenGL display. It validates cached output against fresh framebuffer pixels, including foreground changes underneath partial liquids. Native frame/resource reclamation runs outside setup samples; reported setup/GL-upload costs are not whole-frame or hardware GPU timings. Use fixed complete batches (`--benchmark_min_time=1x`) with repeated samples for expensive world/active-cell resets; do not shrink workloads or relax timeouts.

Compare identical inputs/build flags on the same CPU, preferably A/B/B/A with repeated samples; account for VPS noise. These benchmarks do not replace correctness tests or justify timing thresholds on shared CI.

### Development notes

Project C++ uses named modules, `import std;` and `std::expected`, with RTTI and C++ exceptions disabled. Preserve the same static C++ runtime across dependencies.

Name results by their domain meaning, not migration counters. Return identical result types directly; use short `transform`/`and_then` projections for pure conversions and explicit sequences for side effects. Preserve the first failure, partial mutations and codec field order. Keep module imports direct and minimal rather than adding umbrella exports.

Containers own their storage; shared algorithms and codecs use `SequenceContainer`, `MapContainer` and `SetContainer` constraints. Use `keyView()`/`valueView()` for nonmutating map traversal and `std::span`/`std::mdspan` for contiguous borrowed data. Views must not outlive owners or invalidation; retain snapshots across mutation or callbacks, and materialize Lua arguments before invoking scripts.
Reuse owned sequence buffers for nonfallible path conversion and sorting; keep resource-name ordering identical across network peers. Materialize serialized arrays only once. Lua profile export borrows call-tree entries while its detached engine remains alive.
Resource reset defaults distinguish absolute values from fractions of the current maximum (`1.0` means full). Keep configured defaults unclamped; only actual values and locks enter disk/network state.

`IODevice` byte transfers accept `std::span<char>`/`std::span<char const>`; pointer/count overloads are removed. Short-transfer, EOF, closed-device and absolute-position semantics are unchanged. JSON serialization borrows retained string storage and writes UTF-8 directly, preserving sink/encoding error order.

`DataStreamBuffer` value-owns its `Buffer` and is move-only. `DataStreamIODevice` and `IODeviceCallbacks` borrow an `IODevice&`; retain the device through every operation/callback and keep its address fixed while a decoder uses it. Audio owns its decoder and cursor; clones share immutable audio bytes, not mutable decoder state. Item comparison/stack queries use required or optional references; inventory ownership-transfer APIs still transfer owning handles.

Prefer values or `unique_ptr` for exclusive ownership. Business code uses `T&`/`T const&` for required borrows, `reference_wrapper<T>` for rebindable borrows, `optional<reference_wrapper<T>>` for nullable borrows, and spans/views for ranges. Checked borrowed results use `expected<reference_wrapper<T>, StarException>`; access the referent with `.get()`. `optionalRef` accepts named owners only; neither it nor `asRef` extends lifetime. Container references remain subject to erasure/invalidation. Raw pointers are confined to C/OS APIs, native resource/memory adapters, storage/iterator internals and type-identity machinery.
Client interface/configuration/helper objects and atlas management nodes are exclusive owners; preserve audio/HTTP/Lua shutdown barriers before releasing them. Root reloadable services, asset payloads, asynchronous completion state and escapable WirePane painter handles retain real shared ownership. Root staying alive does not pin a replaced service generation; `use_count()` is not a concurrency/exclusivity guarantee.

Widget parents are borrowed; children and pane handles retain their actual owners. Escaping callbacks use weak owners, and serialized UI/Lua callbacks borrow the client interface behind its standard stop token. Terminal interface detachment requests cancellation and clears service bindings; ordinary pane dismissal must remain reversible.
Client transitions and pane assembly use ordered domain phases; preserve service leases and teardown barriers across phase boundaries. Successful error recovery must stop session startup, not masquerade as a successful connection. Transport and message-response variants use exhaustive dispatch.

Worker submissions retain concrete constrained callables until queue-boundary erasure; move-only and lvalue-callable work is supported. Client gameplay input snapshots held/edge actions only within a stable focus/configuration interval, never across frames. Radio scroll length is cached per message; bubble separation folds candidates without a filtered temporary list.

Collision reuses distances and only skips repeated sorts for strictly unique keys; equal-distance permutations remain significant. Liquid scheduling preserves per-liquid budgets, unlimited regions, activation order and partial-error recovery. Asset workers leave automatic Lua GC enabled during load bursts and collect fully before idle waits, outside the asset mutex. The debug display (`/debug`) reports the previous completed frame's events, catch-up updates, rendering, final submission, ImGui, presentation and throttle wall times; these are CPU durations, not GPU execution measurements.

`BTree<Storage>` owns a `BTreeStorage` implementation. Lua scripts use fixed-address, nonmovable hosts with value-owned features; preserve each host's initialization, storage-snapshot and shutdown ordering.

Synchronization uses standard mutexes, guards and condition variables; clock acquisition uses `std::chrono`. Checked native thread lifecycle, writer-preferring read/write locking and durable file I/O remain explicit. Convert raw clock ticks using their reported frequencies. TCP close must wake readiness waits promptly while allowing already-buffered complete packets to drain.

Numeric lexical conversion uses `std::from_chars` and requires full tokens. Integer overflow leaves existing destinations unchanged; value-initialized conversions yield `0`, including oversized `uint64_t` inputs that previously wrapped. JSON numeric tokens and bounded formatting buffers use ASCII without changing numeric kinds or binary tags.

JSON numeric hashes follow equality across exactly equivalent integer/float values and signed zero; large integers do not pass through `double`. `find` borrows only from lvalue owners and preserves present null; owning iteration supports temporary snapshots. Existing default policies remain distinct: typed key getters default on null, typed index getters report null conversion errors. Ordinary parsing discards trivia and rejects duplicate keys; formatted JSON retains token order, comments and number spelling, with duplicate tokens and last-value lookup. Formatted builders collect mutably and freeze once. Effect sources cache only successful selected particle choices per immutable stage, retaining lazy validation and RNG order.

Fixed input, renderer, biome, log-level and Perlin name tables are `constexpr` arrays. Use `enumName<Names>`, `enumValue<Names>` or `maybeEnumValue<Names>` for checked lookup; returned names borrow static storage. ASCII reverse lookup uses byte views and `std::ranges::lower_bound` over precomputed indices. Non-ASCII input is fully UTF-8-validated before returning a checked miss; case folding remains ASCII-only. Wire-network traversal uses an explicit-object recursive lambda rather than local type erasure.
Hash text with explicit byte views. Literal adapters preserve first-NUL C-text semantics; explicit views retain their full extent. Boolean hash overloads accept only actual `bool`, so text seeds cannot silently select pointer-to-boolean conversion and change generated worlds.

For clangd 23, run `nix develop -c bash scripts/ide/configure-clangd.sh`. Use `--experimental-modules-support`, `--compile-commands-dir=/absolute/path/to/OpenStarbound/build/clangd` and `--query-driver=/nix/store/*-xclang-wrapper-*/bin/clang++,/nix/store/*-xclang-wrapper-*/bin/clang`; set `LC_ALL=C` and `LANGUAGE=C`.

Reuse compatible build trees for incremental work. Use separate native/cross checkouts and fresh trees after compiler, ABI or sanitizer changes; adjust parallelism to available memory.

Changed-code checks use clang-format 21.1.8 with a 120-column limit, include `.cppm`, and reject newly added numbered result/condition temporaries. Only changed tracked C++ lines are formatted; the Linux x86-64 PR gate runs before dependency/compiler setup.

```sh
nix shell --inputs-from . nixpkgs#python3 nixpkgs#clang-tools --command scripts/format-source.sh --check
```

Use `--apply` to format or `--base <commit>` to compare a branch. This shell is for formatting only; project compilation still uses pinned xclang. Reuse already-imported result classification where semantics match; callable ownership/reference-adaptation policies remain independent.

Core standard-library adapters retain engine-specific contracts:

- `File::copy` uses `std::filesystem::copy_file` with overwrite and `error_code` reporting. It copies regular files, follows symlinks, rejects source/target aliases, and uses the standard operation's permission behavior. It is neither an atomic replacement nor a durability barrier.
- Use `List::filter` for nonfallible sequence cleanup when predicates do not depend on intermediate erases or destruction order; its standard `remove_if` path compacts once. Generic `eraseWhere` retains immediate erasure and checked-predicate short-circuiting.
- Stream formatting writes through `std::format_to` without constructing a complete result string on the default-width path. Explicit ostream widths retain buffered field padding; stream errors, tied streams, `unitbuf`, and the explicit `coutf`/`cerrf` flushes remain effective.
- Standard-backed sequence constructors delegate directly to their containers. Lua callable argument packs use `FunctionTraits::ArgTuple` with `std::type_identity`, without constructing argument values.
- Lua string values and JSON keys use explicit byte lengths, preserving leading, embedded and trailing NUL bytes, including entity unique-ID and portrait results. There is no Lua NUL-truncation policy or per-engine override, including after engine restart. Entity callbacks use ordinary `std::optional` results; missing values become Lua `nil`, distinct from empty strings. Network protocols with explicit NUL-terminated fields retain their own encoding.
- JSON-RPC's private handler registry uses standard node-hash storage so that registering more methods during a call does not relocate its active callable. Other engine hash tables retain their existing storage and iteration contracts.
- Entity and universe message responses use `std::expected<Json, String>`, with packet-local codecs preserving error tag `1`, success tag `2`, the default empty-string error, and legacy unknown-tag/partial-read behavior.
- Use `std::clamp` only where ordered bounds are established and no reference escapes. Presentation/audio scalar interpolation uses `std::lerp`; procedural generation and generic/vector/integer interpolation retain the original `Star::lerp` arithmetic. Songbook pitch conversion uses `std::log2`, without a bitwise rounding-equivalence promise.

Check library implementation support, not just the language standard: the pinned libc++ does not implement `std::move_only_function`, so `UniqueFunction` remains in use.

## Community

[Issues and contributions](https://github.com/OpenStarbound/OpenStarbound/issues) are welcome.

<details>
<summary>Discord invite</summary>

Reverse the text in this image to obtain the invite code (spam prevention):

<img width="380" height="70" alt="Reversed Discord invite code" src="https://github.com/user-attachments/assets/9db11a89-0c32-4034-8555-68b54e5918a8" />

</details>
