#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
: "${XCLANG_ROOT:=$PWD/build/xclang-sdk/xclang}"
export XCLANG_ROOT
export PATH="$XCLANG_ROOT/bin:$PATH"
case "$(uname -m)" in
  arm64|aarch64) preset=macos-arm-release ;;
  x86_64) preset=macos-release ;;
  *) echo "Unsupported macOS architecture" >&2; exit 1 ;;
esac
cmake --build "build/$preset" --parallel "$@"
