#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
case "$(uname -m)" in
  arm64|aarch64) ;;
  *) echo "Only Apple Silicon macOS is supported" >&2; exit 1 ;;
esac

if [ -z "${XCLANG_ROOT:-}" ]; then
  cmake "-DXCLANG_INSTALL_DIR=$PWD/build/xclang-sdk" -P scripts/ci/install-xclang.cmake
  XCLANG_ROOT="$PWD/build/xclang-sdk/xclang"
fi
export XCLANG_ROOT
export PATH="$XCLANG_ROOT/bin:$PATH"
mkdir -p dist
cp scripts/osx/sbinit.config dist/
cmake -S source --preset macos-arm-release
