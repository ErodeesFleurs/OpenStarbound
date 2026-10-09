#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."

if [ -z "${XCLANG_ROOT:-}" ]; then
  cmake "-DXCLANG_INSTALL_DIR=$PWD/build/xclang-sdk" -P scripts/ci/install-xclang.cmake
  XCLANG_ROOT="$PWD/build/xclang-sdk/xclang"
fi
export XCLANG_ROOT
export PATH="$XCLANG_ROOT/bin:$PATH"
case "$(uname -m)" in
  arm64|aarch64) preset=macos-arm-release ;;
  x86_64) preset=macos-release ;;
  *) echo "Unsupported macOS architecture" >&2; exit 1 ;;
esac
mkdir -p dist
cp scripts/osx/sbinit.config dist/
cmake -S source --preset "$preset"
