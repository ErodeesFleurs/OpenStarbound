#!/usr/bin/env python3
"""Check changed C++ lines without reformatting legacy code outside the patch."""
import argparse
import difflib
import os
from pathlib import Path
import re
import subprocess
import sys

FORMAT_VERSION = "21.1.8"
HUNK = re.compile(r"^@@ -\d+(?:,\d+)? \+(\d+)(?:,(\d+))? @@")
GENERATED_NAME = re.compile(r"\b(?:[A-Za-z_]\w*Result\d+|condition\d+)\b")


def git(root, *arguments):
    return subprocess.check_output(["git", "-C", str(root), *arguments], text=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument("--check", action="store_true", help="report changes without writing (default)")
    mode.add_argument("--apply", action="store_true", help="format changed lines in place")
    parser.add_argument("--base", default="HEAD", help="commit to compare against (default: HEAD)")
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    base = git(root, "rev-parse", "--verify", "--end-of-options", args.base + "^{commit}").strip()
    paths = git(root, "diff", "--name-only", "-z", "--diff-filter=ACMR", base, "--", "source").split("\0")
    formatter = os.environ.get("CLANG_FORMAT", "clang-format")
    checked_version = False
    failed = False
    checked_files = 0
    for name in paths:
        if Path(name).suffix not in (".cpp", ".cppm", ".hpp", ".h"):
            continue
        patch = git(root, "diff", "--no-ext-diff", "--no-color", "--unified=0", base, "--", name)
        ranges = []
        line_number = 0
        for line in patch.splitlines():
            if match := HUNK.match(line):
                line_number = int(match[1])
                count = int(match[2]) if match[2] is not None else 1
                if count:
                    ranges.append(f"--lines={line_number}:{line_number + count - 1}")
            elif line.startswith("+") and line_number:
                for match in GENERATED_NAME.finditer(line[1:]):
                    print(f"{name}:{line_number}: replace generated temporary '{match[0]}' with a domain name", file=sys.stderr)
                    failed = True
                line_number += 1
        if not ranges:
            continue
        if not checked_version:
            version = subprocess.check_output([formatter, "--version"], text=True)
            if not re.search(r"\bversion " + re.escape(FORMAT_VERSION) + r"\b", version):
                raise RuntimeError(f"Use clang-format {FORMAT_VERSION}; found {version.strip()}")
            checked_version = True
        path = root / name
        original = path.read_bytes().decode("utf-8")
        formatted = subprocess.check_output(
            [formatter, "--style=file", "--fallback-style=none", *ranges, str(path)], text=True
        )
        checked_files += 1
        if original != formatted:
            if args.apply:
                path.write_bytes(formatted.encode("utf-8"))
            else:
                sys.stdout.writelines(difflib.unified_diff(original.splitlines(True), formatted.splitlines(True), name, name))
                failed = True
    print(f"{'Formatted' if args.apply else 'Checked'} changed lines in {checked_files} C++ files.")
    return 1 if failed else 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (OSError, subprocess.CalledProcessError, RuntimeError) as error:
        print(f"Source quality check failed: {error}", file=sys.stderr)
        sys.exit(2)
