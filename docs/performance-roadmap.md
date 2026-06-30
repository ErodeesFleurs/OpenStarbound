# Performance Roadmap

This project now has release and profiling presets for both GCC and Clang. Use
the profiling presets first when validating a suspected hotspot, because they
keep frame pointers while preserving release-like optimization.

## Baseline Commands

Configure and build a profiling binary:

```sh
nix run .#profile-clang
```

Run the focused test suite before collecting profiles:

```sh
ctest --test-dir build/linux-profile-clang --output-on-failure
```

Use a dated local directory for machine-specific profiles. The profile output is
not committed, but keeping the command and scenario stable makes later deltas
meaningful:

```sh
mkdir -p profiles/$(date +%Y%m%d)
perf record -F 997 -g -o profiles/$(date +%Y%m%d)/server-startup.data -- dist/starbound_server
perf report -i profiles/$(date +%Y%m%d)/server-startup.data
```

Record the baseline context next to each profile:

- Preset and compiler, for example `linux-profile-clang` with the exact compiler
  version from CMake configure output.
- Allocator options, especially `STAR_USE_JEMALLOC` or `STAR_USE_RPMALLOC`.
- Asset revision or local asset pack used for the run.
- Exact command line and scenario notes.
- Top functions from `perf report` before making optimization changes.

Recommended baseline scenarios:

- `core_tests`, to catch core container, JSON, string, and Lua changes.
- `game_tests`, to cover asset loading with the checked-in asset set.
- `starbound_server` startup, to profile server-side initialization without GUI
  frame noise.

For a bounded server startup wall-time sample, build the clang profiling preset
and run the server under a timeout:

```sh
STAR_PROFILE_SERVER_TIMEOUT=20 nix run .#profile-server-startup
```

The command treats a timeout-terminated server as a successful bounded sample
and prints wall/user/sys/maxrss timing. Keep the timeout, asset set, allocator,
and command line stable when comparing runs.

## Local Baseline

2026-06-03, source `8845f5eff9d35d7d072ca95e64ca2c5e8c85ade3`,
`linux-profile-clang`, clang 21.1.8, `STAR_USE_JEMALLOC=ON`,
`STAR_ENABLE_FRAME_POINTERS=ON`:

- `core_tests`: 7.01 seconds before the allocation pass, 7.03 seconds after it.
- `game_tests`: 19.29 seconds before and after the allocation pass.
- Total `ctest`: 26.30 seconds before, 26.32 seconds after.
- Final post-change verification run: `core_tests` 7.09 seconds,
  `game_tests` 19.73 seconds, total 26.82 seconds.

This machine did not have `perf` available in the host PATH or the Nix dev
shell, so this baseline is a coarse correctness and wall-time guard rather than
a symbol-level hotspot profile. Treat small deltas here as noise.

Allocator comparison, single run each on the same source and clang profiling
configuration:

- jemalloc: `core_tests` 7.09 seconds, `game_tests` 19.73 seconds, total
  26.82 seconds.
- system allocator: `core_tests` 7.32 seconds, `game_tests` 20.18 seconds,
  total 27.51 seconds.
- mimalloc: `core_tests` 7.17 seconds, `game_tests` 19.11 seconds, total
  26.28 seconds.

Do not change the default allocator from this single run. Repeat the same
commands several times on a quiet machine and prefer symbol-level profiles once
`perf` or another profiler is available.

Repeated allocator comparison, 2026-06-03, source
`b88ecabea037ba635eb477e00437702d826f4a4d`, five `ctest` runs per allocator on
the same clang profiling configuration:

- jemalloc: `core_tests` average 7.28 seconds, `game_tests` average
  19.98 seconds, total average 27.05 seconds, total range 26.43-28.10 seconds.
- system allocator: `core_tests` average 7.42 seconds, `game_tests` average
  20.39 seconds, total average 27.65 seconds, total range 26.90-29.71 seconds.
- mimalloc: `core_tests` average 7.30 seconds, `game_tests` average
  19.54 seconds, total average 26.40 seconds, total range 25.85-27.54 seconds.

Mimalloc is the best candidate from this repeated `ctest` sample, but one
`game_tests` outlier still appeared. Keep jemalloc as the default until a real
startup or asset-heavy scenario confirms the same trend.

## Current Priorities

1. Asset loading and JSON parsing
   - Validate with asset-heavy startup and universe loading scenarios.
   - Existing `fast_float` usage in lexical casting should remain vendored until
     standard-library floating point parsing is proven at least as fast across
     supported compilers.

2. Formatting in hot paths
   - Prefer `format(std::ostream&, ...)` for direct stream output when the
     formatted result is not needed as a `std::string`.
   - The stream helper writes with `std::vformat_to`, avoiding the temporary
     string allocation that `strf` necessarily creates.

3. Pathfinding and world geometry
   - Profile `PlatformerAStar` scenarios with high node counts.
   - Keep formatting out of debug-heavy path dumps unless they are explicitly
     requested.

4. Allocation-heavy containers
   - Compare `STAR_USE_JEMALLOC`, `STAR_USE_RPMALLOC`, and system allocation on
     the same workload before changing defaults.
   - Keep allocator decisions preset-driven so CI and developer shells stay
     reproducible.
   - Use the separate clang profiling presets to avoid allocator cache
     cross-contamination:
     `nix run .#profile-clang`,
     `nix run .#profile-clang-system-alloc`, and
     `nix run .#profile-clang-mimalloc`.
   - Use `STAR_PROFILE_SERVER_TIMEOUT=20 nix run .#profile-server-startup`
     for a repeatable bounded server startup scenario before changing the
     default allocator.

5. Link and build time
   - `STAR_USE_LLD` is enabled by default for GCC/Clang non-macOS builds.
   - CMake presets do not enable verbose builds by default, so CI logs keep
     warnings and errors visible without dumping every compile/link command.
   - If an existing build directory still runs `ninja -v`, clean stale cache
     entries with `nix run .#clean-cmake-cache`.
   - `STAR_ENABLE_UNITY_BUILD` is available for opt-in clean-build experiments:
     `cmake --preset=linux-release-clang -S source -DSTAR_ENABLE_UNITY_BUILD=ON`.
     The 2026-06-03 validation passed for clang and gcc release builds with
     `star_extern`, `core_tests`, and `game_tests` explicitly kept out of unity
     builds.
   - Keep `star_extern` out of unity because some vendored C translation units
     reuse local typedef and helper names that collide when merged. Keep the test
     targets out because several test files intentionally reuse local helper
     names, and merged test translation units also interact poorly with the
     project memory wrappers around system allocation names.
   - Restore a build directory after a unity experiment with
     `cmake --preset=linux-release-clang -S source -DSTAR_ENABLE_UNITY_BUILD=OFF`
     before running normal release validation.
   - When comparing build changes, record `sccache --show-stats` before and
     after a clean configure/build so cache effects are visible.
