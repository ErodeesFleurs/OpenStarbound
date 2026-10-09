#!/usr/bin/env bash
set -euo pipefail

# Run with: nix develop -c bash scripts/ide/configure-clangd.sh
# CMake omits Nix wrapper include paths as implicit. Preserve the wrapper flags
# in the database so clangd also sees dependency headers outside nix develop.
: "${NIX_CFLAGS_COMPILE:?Run this script through nix develop}"
: "${XCLANG_ROOT:?Run this script through nix develop}"
compiler_root="$(dirname -- "$(dirname -- "$(command -v clang++)")")"
cxx_stdlib_flags="$(<"$compiler_root/nix-support/libcxx-cxxflags")"
root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd)"
cmake -S "$root/source" -B "$root/build/clangd" -G Ninja \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DSTAR_PRECOMPILED_HEADERS=OFF \
  -DBUILD_TESTING=ON \
  -DSTAR_BUILD_DEV_TOOLS=ON \
  "-DCMAKE_C_FLAGS=$NIX_CFLAGS_COMPILE" \
  "-DCMAKE_CXX_FLAGS=$NIX_CFLAGS_COMPILE $cxx_stdlib_flags"
