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

Recommended baseline scenarios:

- `core_tests`, to catch core container, JSON, string, and Lua changes.
- `game_tests`, to cover asset loading with the checked-in asset set.
- `starbound_server` startup, to profile server-side initialization without GUI
  frame noise.

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

5. Link and build time
   - `STAR_USE_LLD` is enabled by default for GCC/Clang non-macOS builds.
   - CMake presets do not enable verbose builds by default, so CI logs keep
     warnings and errors visible without dumping every compile/link command.
   - If an existing build directory still runs `ninja -v`, clean stale cache
     entries with `nix run .#clean-cmake-cache`.
   - `STAR_ENABLE_UNITY_BUILD` is available for opt-in clean-build experiments:
     `cmake --preset=linux-release-clang -S source -DSTAR_ENABLE_UNITY_BUILD=ON`.
     Keep it off by default until both clang and gcc warning output stay clean.
   - When comparing build changes, record `sccache --show-stats` before and
     after a clean configure/build so cache effects are visible.
