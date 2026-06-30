#!/bin/sh

set -eu

repo_root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
source_dir="$repo_root/source"

: "${CLANG_FORMAT:=clang-format}"

if ! command -v "$CLANG_FORMAT" >/dev/null 2>&1; then
  echo "Could not find clang-format: $CLANG_FORMAT" >&2
  exit 1
fi

format_file() {
  case "$1" in
    *.cpp | *.hpp)
      "$CLANG_FORMAT" -fallback-style=none -i "$1"
      ;;
  esac
}

format_dir() {
  find "$1" \
    \( -path "$source_dir/extern" -o -path "$source_dir/extern/*" \) -prune -o \
    -type f \( -name '*.cpp' -o -name '*.hpp' \) \
    -exec "$CLANG_FORMAT" -fallback-style=none -i {} +
}

if [ "$#" -eq 0 ]; then
  for path in "$source_dir"/*; do
    if [ "$(basename -- "$path")" != "extern" ] && [ -d "$path" ]; then
      format_dir "$path"
    fi
  done
else
  for path do
    case "$path" in
      /*)
        target=$path
        ;;
      source/*)
        target=$repo_root/$path
        ;;
      *)
        target=$source_dir/$path
        if [ ! -e "$target" ]; then
          target=$repo_root/$path
        fi
        ;;
    esac

    if [ ! -e "$target" ]; then
      echo "No such file or directory: $path" >&2
      exit 1
    fi

    case "$target" in
      "$source_dir"/extern | "$source_dir"/extern/*)
        continue
        ;;
    esac

    if [ -d "$target" ]; then
      format_dir "$target"
    elif [ -f "$target" ]; then
      format_file "$target"
    fi
  done
fi
