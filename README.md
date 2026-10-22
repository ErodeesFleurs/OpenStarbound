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

For clangd, run `nix develop -c bash scripts/ide/configure-clangd.sh` after CMake or toolchain changes. This freshly configures `build/clangd` without building game targets. Start clangd 23 with `--experimental-modules-support`, `--compile-commands-dir=/absolute/path/to/OpenStarbound/build/clangd` and `--query-driver=/nix/store/*-xclang-wrapper-*/bin/clang++,/nix/store/*-xclang-wrapper-*/bin/clang`; set `LC_ALL=C` and `LANGUAGE=C` for the server. The explicit database argument is also required for the toolchain's external `std.cppm`, which cannot inherit the repository's `.clangd`. Keep the database's C++23 flags for `.cppm` files and let clangd build its own module cache, rather than loading compiler BMIs. Restart the editor's language server after changing its arguments.

- Project-owned C++ interfaces use `.cppm`, including codecs, Lua components, application/SDK adapters, tools and test helpers; ordinary `.cpp` companions retain native implementations. Keep declarations in the owning interface, not `.inc` fragments or substitute private headers. Register each interface in its owning CMake file set and link the complete module provider.
- Import APIs used by declarations and template bodies directly, at file scope before namespaces; private imports are not re-exports. Import `star.data_stream_extra`, `star.json_extra`, `star.lua_converters` and the relevant game converters before instantiating their codecs. Preserve caller-owned ADL and explicitly export canonical primary templates and specialization redeclarations. Function-template specialization redeclarations use `extern "C++"` when needed to retain their global-module ownership.
- Use `import std;` and explicitly qualified `std::` names for standard C++ APIs; do not introduce standard-library using declarations or namespace directives. Dependent customization calls retain ADL with an explicitly qualified standard fallback. `star_std` builds the pinned libc++ `std.cppm` from `libc++.modules.json` once per build tree and propagates through `star_modules`; no PCH or prebuilt BMI is used. Keep C macro/global-name headers and third-party declarations textual. The Linux `sys/cdefs.h` overlay preserves glibc's checked fortify bodies with hidden inline C++ linkage so they can be exported by `std`; include C headers before imports in files that call fortified global functions.
- All project-owned C++ targets and their `std` module require `-fno-rtti` and `-fno-exceptions`; do not override either policy on individual sources, modules or adapters. CMake rejects conflicting flags and treats ignored `[[nodiscard]]` results as errors. Imported dependencies retain their own build policy: linked runtimes may still contain RTTI, exception support and unwind tables. RTTI-dependent `vptr` sanitizer checks remain unavailable for project targets.
- Linux ASan builds use `nix/asan-runtime.nix` to reproduce the pinned xclang libc++/libc++abi with `-fno-sanitize-address-globals-dead-stripping`, avoiding strong/weak EH typeinfo COMDAT collisions with no-RTTI consumers. Only fine-grained global dead-stripping is disabled: the external runtime retains RTTI, exceptions, ASan instrumentation, global redzones, private aliases, ODR indicators and container annotations. Its generated `__config_site` must match the bundled ASan configuration byte-for-byte; bundled headers, compiler-rt and unwinder remain unchanged. Separate runtime probes cover exception catches, global-buffer-overflow and cross-DSO ODR violations; this is not an EH-enabled project adapter.
- Import `star.type_identity` for `typeKey<T>()` and diagnostic-only `typeName<T>()`. Keys identify a cv/ref-normalized type within one linked image; never serialize them, use spelling hashes as identity, or exchange them across independent images. Polymorphic project capabilities use explicit `queryType` overrides, retaining all relevant base branches at multiple-inheritance joins. `queryTypeOf` merges shared virtual-base addresses and rejects distinct matching subobjects; it is not a general replacement for source-subobject-sensitive C++ RTTI. Expose a new concrete capability in its owner before querying it. Checked `as` casts preserve constness, adjusted interface pointers and the original shared/intrusive ownership.
- Recoverable failures return `[[nodiscard]] std::expected<T, StarException>` directly; void operations use `std::expected<void, StarException>` and fallible reference access returns a pointer payload. Check success before dereferencing, distinguish outer failure from an empty optional/null payload, and propagate errors through callbacks and virtual interfaces. Fallible acquisition uses checked `create` factories; constructors establish non-failing state. `StarException` is an owning diagnostic value, not a `std::exception`; `error.is<DomainError>()` preserves base-category matching, while `withContext` and `ExceptionDiagnostic` retain causes and stacks. Do not replace ordinary errors with aborts, defaults, ignored results or global error slots. Allocation exhaustion is terminal. Lua/libpng C jumps remain confined to their protected C boundaries, after nontrivial C++ temporaries have been released.
- A successful `std::expected<T*, StarException>` owns only the pointer, not its referent. Retain any by-value container in a named local before taking a checked `first()`, `last()`, `at()` or map lookup result; consume the borrow before destroying or invalidating that owner. Splitting `temporaryContainer().first()` and its dereference across statements leaves a dangling payload.
- Use `std::optional<T>` directly; `Maybe` and `star.maybe` are retired without an alias. Check presence before dereferencing; do not use throwing `.value()` for recoverable input. Use `.value_or(...)` only at an intended fallback boundary. Standard optional moves do not disengage the source. Import `star.optional` for consuming `takeOptional` (reset only after successful transfer), nullable `optionalPtr`, or the explicit `outputOptional` diagnostic view. Use ordinary `if` statements for conditional side effects rather than adding callback wrappers. Do not specialize `std::formatter` for a pure standard-library type. `star.hash` preserves empty/payload hashes; `writeOptional`/`readOptional` and `jsonFromOptional`/`jsonToOptional` preserve the existing presence-bit/JSON encoding. Lua keeps its nil/conversion-failure distinction; `Maybe<T>` in Lua/animation documentation denotes an optional script value, not a C++ wrapper.
- Use `std::variant<T...>` directly; `Variant`, `MVariant` and `star.variant` are retired without compatibility aliases. Nullable unions use a leading `std::monostate`, with explicit presence checks and reset through `emplace<0>()`. Use `std::get_if` for recoverable alternative checks; `std::get` requires an already established alternative. Visitors handle monostate explicitly. Standard moves retain the source alternative, so consuming owners clear it only after successful transfer. Preserve alternative order, duplicate slots and the existing one-byte discriminant/no-payload monostate encoding. Lua conversion probes alternatives in declaration order and distinguishes successful nil from conversion failure. Use checked `printLuaValue` and `printWorldId` before formatting their results; a standard formatter cannot return a recoverable serialization error. Lua documentation's `Variant<...>` notation denotes a script union, not a C++ wrapper.
- Formatting uses the standard library, without bundled fmt. `strf`, logging and diagnostic helpers accept compile-time-checked `std::format_string` literals only; arbitrary runtime standard-format strings are no longer an API. Validate any dynamic width, precision or fallible argument conversion before standard formatting. Only nonfallible ostream representations can use `StreamFormatter`; checked JSON/Lua/domain printers must return and propagate their errors before their text is formatted. Export formatter specializations explicitly; when an inline GMF function needs one first, define it there and export its specialization redeclaration after the module declaration.
- Dynamic text uses `formatTags(text, bindings) -> std::expected<std::string, StarException>` from `star.format`: `<name>` inserts a named value and `<<` emits a literal `<`. Unknown, empty or unclosed tags return `FormatException`; replacement bytes are not rescanned. Byte lengths, embedded NULs and UTF-8 replacements are retained. Chat templates use `<fromNick>`/`<text>`, planet-name templates use `<worldName>`, and command-help templates use `<commands>`. Mods or configuration overrides supplying the former brace-format strings must migrate to this syntax.
- The formatting chain is provided by `star.format`, `star.stream_formatter`, `star.string`, `star.string_view`, `star.json`, `star.lua` and `star.logging`, with their container/hash, Unicode, thread and stream/file dependencies also imported as modules. Retired formatter-chain headers are not compatibility interfaces.
- Import `star.memory` for `NPos`, `star.io_device` for `StreamOffset`, and `star.format` for `DebugEnabled` and `starAssertFailed`. Guard assertion calls with `if constexpr (Star::DebugEnabled) { Star::starAssertFailed((condition)); }`: release assertions must not evaluate their operands, while debug assertions preserve contextual boolean conversion and capture the caller's source location through the default argument. There is no assertion macro or configuration umbrella header. Windows `NOMINMAX` is supplied by `star_build_config` before native SDK headers.
- Spell standard ownership directly with `std::shared_ptr<T>`, `std::weak_ptr<T>` or `std::unique_ptr<T, Deleter>` instead of generated pointer aliases or alias chains. Preserve pointee constness, custom deleters and nested/template scopes; intrusive `RefPtr` ownership remains distinct. Retain native class/struct forward declarations needed by opaque or recursive references before their definitions, and export native type identities rather than pointer aliases.
- Inside `Star`, omit redundant `::Star::` prefixes when ordinary lookup finds the same declaration. Keep qualification for shadowed names, cross-namespace references and native module export declarations such as `using ::Star::Name;`; local helper types can otherwise hide public types, as in `Root::scanForAssetSources`.
- Use scoped functions, constants and explicit native wrapper declarations instead of implementation macros. Strong wrappers retain their original class-template identities, explicit base construction, assignment restrictions and ADL; import `star.strong_typedef` for `StrongTypedefBuiltin`. Preserve SHA-256's 32-bit arithmetic and padding/length accounting, and xxHash's explicit primitive overloads and little-endian representations, including 64-bit serialization of native `long` on Windows. Keep preprocessor switches required by platform headers and vendor SDKs; CMake-only selection flags need not become C/C++ definitions.
- Qt tools generate moc output directly from their `.cppm` interfaces with `moc -i`; the owning scanned `.cpp` includes that generated output. Keep AUTOMOC disabled for these targets and preserve Qt preprocessing and generation dependencies.
- Import `star.native_runtime` and check `NativeRuntime::create()` at the start of every executable, before application objects; retain its fixed-address owner until all borrowers have shut down. Windows owners balance Winsock startup/cleanup and share a reference-counted DbgHelp session under a process-lifetime native lock. Do not restore pre-main initialization objects or hide an owner in a module initializer. Other platforms require no corresponding native facilities.
- `Root` is an ordinary fixed-address composition root, owned by `RootLoader` or an explicit caller. Pass `Root&` or a narrower dependency through factories, entities, worlds, serialization and tools; there is no global accessor or thread-local current Root. Assets/configuration, catalogs, logging/debug maps, RNG, timing, identity and synchronization state belong to that instance. Every borrower must be destroyed before its Root.
- Catalog loaders acquire dependencies only on a cache miss. Item construction passes a lexical ready-catalog capability through recipes, rebuilders, treasure, versioning, species/NPC and collection lookups; never retain that construction-only capability. Collection metadata is computed outside its publication mutex and retains the first published winner, not a last-writer replacement; concurrent misses may still compute twice.
- Lua execution scopes borrow their owning Root explicitly; asset image decoding uses an explicit resolver rather than importing game services into core Lua. Pass `GuiContext&` through readers/widgets and bind Input, Voice and TileDrawer to that context. Clear bindings and disconnect callbacks before tearing down the borrowed services.
- Native SDL/OpenGL/ImGui calls run under `NativeUiContextScope`, including initialization, events/Lua, frames and backend shutdown. Activate a newly created ImGui context before its first IO/style/backend access: `CreateContext` may restore an existing parent context. Restore the previous native context when leaving the scope, and discard saved references before destroying their context. This does not turn the whole application loop into a multi-window event dispatcher.
- The Lua ImGui adapter registers bindings for the supplied Lua state; it has no global Lua pointer or shared Begin/End tracking vector. Scripts must still balance native ImGui calls, including failed calls; deleting the unused `RunString` example does not provide automatic balancing. Flatpak detection is a platform-service instance result exposed through `ApplicationController`, not a global flag.
- HTTP trust, pending requests and handles belong to a `LuaHttpSession`, not a shared global queue. Replies/userdata retain only the originating session's weak capability. Stop settles pending and accepted asynchronous promises, rejects new work and joins the client's dedicated I/O workers before dependencies are destroyed; a stale reply must never authorize a replacement session.
- Generate identities with `Uuid(RandomService&)`; use `Uuid::nil()` only for explicitly unfilled wire staging. Decode a variant's payload before constructing its alternative instead of consuming randomness or requiring a dummy default constructor. Keep deterministic world seeds and existing packet/persistence representations unchanged.
- Native signal/SDK entry points may require process-level hooks, but those bridges must not expose Root or business services. Fatal/interrupt callback state must outlive unregistration; POSIX fatal callbacks use signal-safe output, and the Windows fault snapshot preserves the original minidump/trampoline path without borrowing the handler's destroyed implementation. Ordinary socket/PNG diagnostics use the caller's Logger; stateless emergency output is reserved for bootstrap/fatal paths.
- `FlatHashTable` retains its one-value union, layout, sentinels and explicit placement construction/destruction. The union is named because pinned xclang crashes generating imported anonymous-union member accesses; its empty constructor/destructor must never activate or destroy the stored value.
- Flat hash range insertion may pre-count forward iterators, but must consume input iterators only once. After erasing a back-shifted bucket, continue from the returned iterator. `try_emplace` constructs mapped values only on a miss; duplicate `insert_or_assign` does not move the key. String maps accept borrowed UTF-8 lookup keys without changing byte hashes or case-insensitive codepoint rules. `OrderedHashMap` retains typed equality because its internal keys are `reference_wrapper`s.
- Fixed enum/name/type tables and the server nickname are read-only shared data, not per-Root services. Preserve their contents and lookup semantics; namespace-scope exported `extern const` data keeps an explicit `extern` definition in its companion so the importing ABI retains external linkage.
- LRU hits only promote the existing entry; evict only on misses, before invoking the producer, and keep capacity at least one. LRU/TTL callbacks are borrowed synchronously and invoked as lvalues, including mutable move-only callables. TTL refresh retains empty `std::function`/null-pointer handling; a callable's boolean conversion is not an emptiness test.
- Borrow read-only JSON arrays through `arrayPtr()`/`iterateArray()` while retaining their owner. Name generation borrows immutable Markov tables; animation transforms borrow property maps without changing arithmetic order or eager fallback evaluation. ItemBag loading borrows its input array only for the synchronous item conversion.
- Inventory and const ItemBag slot queries return owning observations without normalizing stored slots. Externally shared Items can become empty; reads report an empty slot without releasing its stored owner, while mutation boundaries normalize it. Equipment placement through `take(1)` may clone the Item: observe or consume the actual stored alias, not the original input pointer.
- For C-ABI strings in trailing storage, pass `std::string_view` with the API-provided byte length, such as `SYMBOL_INFO::Name`/`NameLen`. The bundled libc++ bounds a `char[N]` argument by its declared extent; it is not an unbounded C-string pointer.
- Lua userdata must retain `std::max_align_t` alignment for stored C++ objects. Liquid network updates saturate their byte representation without clamping physical levels or pressure; hue conversion truncates while signed before byte wrapping. Check a positive remaining tick budget before converting it to unsigned sleep milliseconds.
- Prefer standard index sequences, `std::apply`, constrained templates and direct standard algorithms over duplicate generic helpers. Tuple serialization uses a comma fold to preserve left-to-right stream operations; enum serialization retains its underlying width and signedness.
- Use `std::identity` instead of a duplicate identity functor. Fixed-array/vector copies use `std::ranges::copy_n` to preserve bounded writes, value-initialized tails and the original final increment of single-pass input iterators.
- Use `std::numeric_limits<T>::lowest()`/`max()` and double `std::numbers` constants directly. Preserve intermediate precision and arithmetic order; replacing a constant does not authorize changing a numerical algorithm.
- Prefer standard algorithms and views over duplicate sort, reverse, equality and iterator wrappers. `std::views::enumerate` yields `{index, value}`; owning `enumerate` results retain `{value, std::size_t index}`. Owning zip/enumerate helpers copy from their named input ranges, even when passed rvalues. Ordered folds retain left-to-right evaluation, first-element initialization and empty-input results.
- Use native cursors instead of the retired Java-style iterator wrappers. Preserve parameter-token consumption, packet grouping and adjacent-item scans; use the iterator returned by erase, especially when a flat-hash bucket moves.
- Use `min_element`, `max_element`, `find_if`, `count_if` or `none_of` when only a query result is needed, rather than materializing sorted/filtered copies. Keep owners alive, preserve empty-input errors and do not short-circuit predicates with required side effects. World-generation placements compact only after queued pointers are no longer used; generic list removal retains per-element erasure because its argument may alias the list and teardown can be observable.
- Borrow AI mission state and compare its ordered ranges before materializing changed caches. Prune quest indicators in their ordered map and render borrowed values rather than rebuilding maps or copying value lists. Tile damage keeps the occupancy-query phase, then uses stable partition-copy for protected/unprotected groups; preserve protected-first packets, optional source fields and mixed-result precedence. Use one search when both membership and index are needed.
- `StringList::sorted(comparator)` returns a sorted copy without changing the input or dropping duplicates. Byte-array prefix comparison uses a bounded mismatch search with an explicit empty-storage case; lexical byte ordering remains unsigned. Tag copying appends UTF-8 byte substrings after the existing bounds check, while character membership searches decoded code-point iterators.
- Free `any`/`all`, `RangeIterator`/`range`, `compose` and `zipWith` are retired. List predicate members use standard algorithms with explicit boolean conversion, const callable invocation and short-circuit behavior. Keep meaningful owning zip/enumerate and slicing APIs; integer views do not mechanically reproduce negative-step or floating-point ranges.
- Static-random comma folds preserve hash seeds, primitive widths, ADL and left-to-right push order. Each push has its own full expression so ADL return temporaries are destroyed before the next push.
- Use `std::exchange` for equivalent value-and-reset operations. Generic `take` deliberately moves first and only then default-constructs the replacement; preserve that construction order. Unique owners with guaranteed empty moved-from state can be moved directly.
- Use `requires` and `if constexpr` instead of SFINAE scaffolding where the original participation rules can be preserved. Numeric helpers retain their `numeric_limits::is_integer` classification, precision selection, conversions and arithmetic order; dimension and contiguous-storage constraints must not change the supported layouts.
- Import `star.unique_function` for the move-only `UniqueFunction<R(Args...)>` used by threads and worker queues. The pinned libc++ does not implement `std::move_only_function`; do not inject a replacement into `std`. Small, suitably aligned, nothrow-movable callables use three pointer-sized inline slots; other callables use matching typed allocation/deallocation. Invocation returns an expected result, flattening callbacks that already return expected and representing reference results with pointers; empty invocation returns an error. A void signature must not erase an error-returning callback. Thread invocation retains stored-argument lvalue semantics; move-capture ownership explicitly.
- Fixed JSON/command dispatch stores actual function or member-function pointers rather than type-erased wrappers. Shared client command adapters receive their instance explicitly; preserve ignored arguments for no-argument commands, case-insensitive lookup, aliases, builtin precedence and script fallback. A pointer table does not by itself eliminate dynamic map initialization.
- Key-chord modifier lookup uses a constexpr table. Derive the specificity mask from that table before `std::popcount`: Caps/Num/Scroll lock-state bits do not increase binding specificity. Keep tied actions and modifier-key release behavior unchanged.
- Worker completion publishes its value or owning error before a release store, then wakes atomic and condition-variable waiters. Unbounded waits use acquire atomic waiting; timed waits retain the mutex/condition-variable protocol and one steady-clock deadline. Publish under the same mutex used by timed waiters to prevent lost wakeups. Do not replace spin waiting without representative contention evidence.
- WorkerPool owns threads that borrow its locks and queues, so it is non-movable and must retain a stable address. Share task handles rather than relocating the pool. Root starts its CPU pool lazily; HTTP uses a separate I/O pool. Rejection/cancellation must settle task promises, parallel column work must drain submitted promises even after a task fails, and nested work on the same pool uses the serial path. Check explicit stop/join results before destroying borrowed resources. Destructors must not release storage still borrowed by an unjoined thread; that internal lifetime violation is fatal, not an ordinary recoverable work error.
- Retain scoped ListenerSubscription tokens and disconnect raw-this subscriptions before owner teardown. Dispatch serializes callbacks outside the registry lock; disconnect waits for in-flight callbacks except when invoked by that callback itself. Root reload runs synchronously on its caller's thread. RpcPromise retains shared observation, delegated chains, lazy wrapped-result caching and last-keeper AutoFail; remote JsonRpc `fail` still resolves to an empty Json.
- Bulk filtering moves survivors and erases once only where callbacks do not depend on per-element teardown; generic side-effect-sensitive erase helpers retain their original sequencing. Scalar byte conversion uses `std::byteswap` and floating-point `std::bit_cast`, while generic object conversion preserves the existing byte-copy path.
- `StringView` borrows UTF-8 bytes, not UTF-32 storage, and does not guarantee a terminating zero. Convert UTF-32 through an owning `String` first. Tag lookup appends each replacement while its owner is alive; callbacks may return owning strings or valid borrowed views.
- UDP parsing and RCON output may borrow spans or string views only while their receive buffer or response owner remains alive. Preserve packet acceptance and framing, including empty responses and RCON's existing 4096-codepoint chunks; UTF-8 chunks are not necessarily limited to 4096 bytes.
- Connection workers snapshot only their fixed ID shard. Prepare thread storage before starting workers and roll back partial startup by stopping, waking and joining every started worker. TCP waits use native readiness and release the connection lock; Local waits observe queue delivery and peer/local close. Windows uses a retained native wake socket alongside select so local shutdown wakes every waiter without descriptor reuse. P2P and heterogeneous server scheduling retain bounded polling; a scheduler condition variable is not network readiness. Pending compressed output remains pending even after disabling new stream compression.
- Encode input/output and `ExternalBuffer` storage use bounded spans; spans borrow and do not extend the owner's lifetime. Preserve partial writes, permissive hex/base64 decoding and EOF/empty-buffer behavior. Keep the IODevice virtual interface and owning codec wrappers distinct from borrowed storage.
- Custom containers retain their allocation and wire contracts. Aligned byte storage must preserve object lifetime, alignment and slot stride; capacity failures are explicit results, while allocation exhaustion is not a recoverable container transaction.
- Small/static-vector insertion consumes input ranges once and constructs appended elements before relocating the suffix. Fixed-capacity insertion checks capacity before consuming emplace arguments; check its result before using the returned iterator.
- Small/static-vector assignment constructs only newly needed elements and destroys only the unused tail; erasure never routes through growth-oriented resize. Preserve `resize(n, value = Element())` construction semantics. Same-specialization SmallVector moves take over heap storage or move inline elements; after a successful move the source is empty and reusable.
- Call database `commit()` or `close()` explicitly where persistence failures can be reported; destructors release resources without retrying failed transactions or committing pending changes. A failed sync leaves durability uncertain, not necessarily rolled back. World-thread `stop()` only stops execution: export snapshots and deliver final client packets before final `close()`. Retired worlds remain owned until their clients actually depart.
- Resource acquisition must roll back on partial failure. Asset workers are stopped, awakened and joined on startup failure; libpng jumps stay inside trivial call frames and return owning errors at the C++ boundary. SDL audio streams own their device and are destroyed only after callbacks are detached; asynchronous SDK callbacks must validate their service and operation lifetimes.
- Delayed projectile explosions capture their action parameters and source entity ID by value when queued. World timers can run after the projectile has been removed and destroyed; never capture its `this` pointer or extend its entity lifetime just to read that metadata.
- zlib stream cleanup is owned immediately after successful initialization; failed initialization must not be treated as an initialized stream. ZSTD contexts have typed unique owners and cannot be shallow-copied. Preserve ongoing stream state, flush mode, output append behavior and the existing zlib/gzip limit-check differences.
- Build configurations are `Debug`, `RelWithAsserts`, `RelWithDebInfo` and `Release`; `STAR_DEBUG_SYMBOL_LEVEL` accepts `default`, `minimal` or `full`. Caller compiler flags are preserved.
- Retain compatible objects/BMIs for incremental builds. Use fresh build directories after language-standard, compiler, ABI or sanitizer changes, and separate checkouts when retaining native/cross outputs: in-tree builds share `dist/`. Adjust parallelism to available memory.

## Community

[Issues and contributions](https://github.com/OpenStarbound/OpenStarbound/issues) are welcome.

<details>
<summary>Discord invite</summary>

Reverse the text in this image to obtain the invite code (spam prevention):

<img width="380" height="70" alt="Reversed Discord invite code" src="https://github.com/user-attachments/assets/9db11a89-0c32-4034-8555-68b54e5918a8" />

</details>
