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

Collect a CPU profile with the tool available on the host, for example:

```sh
perf record -g -- build/linux-profile-clang/starbound_server
perf report
```

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
