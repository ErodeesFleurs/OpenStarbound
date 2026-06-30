#!/usr/bin/env bash
set -euo pipefail

build_dir="build/linux-release-clang"
binary="game_tests"
filter="SpawnTest.RandomCelestialWorld"
repeat="10"
seed=""
shuffle="false"

usage() {
  cat <<EOF
Usage: $0 [options] [-- extra gtest args...]

Repeatedly run a GoogleTest binary from the CTest runtime directory so tests
that require sbinit.config use the same environment as ctest.

Example:
  nix develop -c $0 --repeat 50 --seed 12345 --shuffle
  nix develop -c $0 --filter SpawnTest.* --repeat 20 -- --gtest_break_on_failure

Options:
  --build-dir DIR   Build directory containing test/game-test-runtime
                    (default: ${build_dir})
  --binary NAME     Test binary in dist/ to run (default: ${binary})
  --filter FILTER   GoogleTest filter (default: ${filter})
  --repeat N        GoogleTest repeat count (default: ${repeat})
  --seed N          GoogleTest random seed
  --shuffle         Shuffle test order on each repeat
  -h, --help        Show this help
EOF
}

extra_args=()
while (($#)); do
  case "$1" in
    --build-dir)
      build_dir="${2:?missing value for --build-dir}"
      shift 2
      ;;
    --binary)
      binary="${2:?missing value for --binary}"
      shift 2
      ;;
    --filter)
      filter="${2:?missing value for --filter}"
      shift 2
      ;;
    --repeat)
      repeat="${2:?missing value for --repeat}"
      shift 2
      ;;
    --seed)
      seed="${2:?missing value for --seed}"
      shift 2
      ;;
    --shuffle)
      shuffle="true"
      shift
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    --)
      shift
      extra_args+=("$@")
      break
      ;;
    *)
      extra_args+=("$1")
      shift
      ;;
  esac
done

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
runtime_dir="${repo_root}/${build_dir}/test/game-test-runtime"
test_binary="${repo_root}/dist/${binary}"

if [[ ! -x "${test_binary}" ]]; then
  echo "error: test binary not found or not executable: ${test_binary}" >&2
  echo "hint: build first, for example: nix run .#build-clang" >&2
  exit 1
fi

if [[ ! -f "${runtime_dir}/sbinit.config" ]]; then
  echo "error: runtime sbinit.config not found: ${runtime_dir}/sbinit.config" >&2
  echo "hint: configure/build with assets present so game_tests are registered" >&2
  exit 1
fi

args=(
  "--gtest_filter=${filter}"
  "--gtest_repeat=${repeat}"
)

if [[ -n "${seed}" ]]; then
  args+=("--gtest_random_seed=${seed}")
fi

if [[ "${shuffle}" == "true" ]]; then
  args+=("--gtest_shuffle")
fi

echo "build dir: ${build_dir}"
echo "runtime:   ${runtime_dir}"
echo "binary:    ${test_binary}"
echo "filter:    ${filter}"
echo "repeat:    ${repeat}"
if [[ -n "${seed}" ]]; then
  echo "seed:      ${seed}"
fi
if [[ "${shuffle}" == "true" ]]; then
  echo "shuffle:   enabled"
fi

cd "${runtime_dir}"
exec "${test_binary}" "${args[@]}" "${extra_args[@]}"
