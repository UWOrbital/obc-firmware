#!/usr/bin/env python3
"""Compare equivalent filename (OFF) and compact file-ID (ON) firmware builds.

Both directories must contain OBC-firmware.bin, OBC-firmware.out, CMakeCache.txt,
generated/logging/log_file_ids.json, and the ARM toolchain used by CMake.
Pass BOARD_TYPE, DEBUG, and LOG_DEFAULT_OUTPUT_FORMAT explicitly when configuring
both builds so their values can be checked in CMakeCache.txt.
"""

import argparse
from collections import defaultdict
import json
from pathlib import Path
import re
import subprocess
import sys


MATCHED_OPTIONS = (
    "CMAKE_BUILD_TYPE",
    "BOARD_TYPE",
    "DEBUG",
    "LOG_DEFAULT_OUTPUT_FORMAT",
)
REMOVED_SYMBOLS = {"LOG_FILE_PATHS", "logFileIdFromPath", "logFilePathFromId"}


def read_cache(directory: Path) -> dict[str, str]:
    values = {}
    for line in (directory / "CMakeCache.txt").read_text().splitlines():
        match = re.match(r"([^/#][^:]*):[^=]+=(.*)", line)
        if match:
            values[match[1]] = match[2]
    return values


def check_options(before: Path, after: Path) -> dict[str, str]:
    old, new = read_cache(before), read_cache(after)
    for key in MATCHED_OPTIONS:
        if key not in old or key not in new:
            raise ValueError(f"Configure both builds with an explicit -D{key}=value.")
        if old[key] != new[key]:
            raise ValueError(f"{key} differs: {old[key]!r} versus {new[key]!r}.")
    if old["CMAKE_BUILD_TYPE"] != "OBC":
        raise ValueError("Both builds must use -DCMAKE_BUILD_TYPE=OBC.")
    for directory, cache, allowed, expected in (
        (before, old, {"OFF", "FALSE", "NO", "0"}, "OFF"),
        (after, new, {"ON", "TRUE", "YES", "1"}, "ON"),
    ):
        if cache.get("LOG_USE_FILE_IDS", "").upper() not in allowed:
            raise ValueError(f"{directory}: expected -DLOG_USE_FILE_IDS={expected}.")
    return old


def read_mapping(directory: Path) -> dict:
    mapping = json.loads(
        (directory / "generated/logging/log_file_ids.json").read_text()
    )
    files = mapping.get("files") if isinstance(mapping, dict) else None
    if (
        not isinstance(mapping, dict)
        or mapping.get("format_version") != 1
        or not isinstance(files, list)
        or not files
        or any(not isinstance(path, str) or not path for path in files)
        or len(files) != len(set(files))
    ):
        raise ValueError(f"{directory}: invalid logging file-ID mapping.")
    return mapping


def run_tool(directory: Path, name: str, *arguments: str) -> str:
    tool = directory / "toolchain/bin" / f"arm-none-eabi-{name}"
    result = subprocess.run(
        [str(tool), *arguments, str(directory / "OBC-firmware.out")],
        check=True,
        capture_output=True,
        text=True,
    )
    return result.stdout


def loadable_sections(directory: Path) -> dict[str, int]:
    # size -A includes debug/NOBITS sections too. Use objdump's LOAD flag to
    # select the sections which actually have loadable contents.
    sizes = []
    for line in run_tool(directory, "size", "-A").splitlines():
        match = re.fullmatch(r"\s*(\S+)\s+(\d+)\s+(\d+)\s*", line)
        if match:
            sizes.append((match[1], int(match[2])))

    headers = run_tool(directory, "objdump", "-h").splitlines()
    loaded = defaultdict(int)
    section_index = 0
    for index, line in enumerate(headers):
        match = re.match(r"\s*\d+\s+(\S+)\s+([0-9a-fA-F]+)\s+", line)
        if not match:
            continue
        entry = (match[1], int(match[2], 16))
        if section_index >= len(sizes) or sizes[section_index] != entry:
            raise ValueError(f"{directory}: size and objdump section lists disagree.")
        section_index += 1
        flags = (
            headers[index + 1].strip().split(", ") if index + 1 < len(headers) else []
        )
        if "LOAD" in flags:
            loaded[entry[0]] += sizes[section_index - 1][1]
    if not loaded or section_index != len(sizes):
        raise ValueError(f"{directory}: could not identify all ELF loadable sections.")
    return dict(loaded)


def compare(before: Path, after: Path) -> None:
    options = check_options(before, after)
    mapping = read_mapping(before)
    if mapping != read_mapping(after):
        raise ValueError("Builds have different logging file-ID dictionaries.")
    old_binary = (before / "OBC-firmware.bin").read_bytes()
    new_binary = (after / "OBC-firmware.bin").read_bytes()
    if not old_binary or not new_binary:
        raise ValueError("Firmware .bin files must not be empty.")

    old_sections, new_sections = loadable_sections(before), loadable_sections(after)
    symbols = {
        line.split()[-1]
        for line in run_tool(after, "nm", "--defined-only").splitlines()
        if line.split()
    }
    remaining_symbols = sorted(
        symbol
        for symbol in symbols
        if any(
            symbol == name or symbol.startswith(name + ".") for name in REMOVED_SYMBOLS
        )
    )
    remaining_paths = [
        path for path in mapping["files"] if path.encode("utf-8") in new_binary
    ]
    rows = [("Firmware .bin", len(old_binary), len(new_binary))]
    rows += [
        (name, old_sections.get(name, 0), new_sections.get(name, 0))
        for name in sorted(old_sections.keys() | new_sections.keys())
    ]
    rows.append(
        ("ELF LOAD total", sum(old_sections.values()), sum(new_sections.values()))
    )
    print(
        "Configuration: "
        + ", ".join(f"{key}={options[key]}" for key in MATCHED_OPTIONS)
    )
    print(f"Matching dictionary: {len(mapping['files'])} source paths")
    print(f"{'Bytes':<22} {'Filenames':>14} {'File IDs':>14} {'Saved':>12}")
    for name, old_size, new_size in rows:
        print(f"{name:<22} {old_size:>14,} {new_size:>14,} {old_size - new_size:>12,}")
    saved = len(old_binary) - len(new_binary)
    print(f"Firmware savings: {saved:,} bytes ({saved / len(old_binary):.2%})")

    failures = []
    if saved <= 0 or sum(new_sections.values()) >= sum(old_sections.values()):
        failures.append("Compact firmware must reduce both .bin and ELF LOAD sizes.")
    if remaining_symbols:
        failures.append("Lookup symbols remain: " + ", ".join(remaining_symbols))
    if remaining_paths:
        failures.append("Mapped source paths remain: " + ", ".join(remaining_paths))
    if failures:
        raise ValueError("\n".join(failures))
    print(
        "PASS: smaller firmware; no logging path table, lookup functions, or mapped paths."
    )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("filenames_build", type=Path, help="LOG_USE_FILE_IDS=OFF build")
    parser.add_argument("file_ids_build", type=Path, help="LOG_USE_FILE_IDS=ON build")
    args = parser.parse_args()
    try:
        compare(args.filenames_build.resolve(), args.file_ids_build.resolve())
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        if isinstance(error, subprocess.CalledProcessError) and error.stderr:
            print(error.stderr.strip(), file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
