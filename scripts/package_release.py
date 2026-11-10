#!/usr/bin/env python3

from __future__ import annotations

import argparse
import hashlib
import json
import pathlib
import shutil
import subprocess
import os
from datetime import datetime, UTC

from version import generate_version


def sha256(path: pathlib.Path):

    h = hashlib.sha256()

    with open(path, "rb") as f:
        while chunk := f.read(8192):
            h.update(chunk)

    return h.hexdigest()


def git_commit():

    try:
        return subprocess.check_output(
            ["git", "rev-parse", "--short", "HEAD"],
            text=True,
        ).strip()

    except Exception:
        return "unknown"


def main():

    parser = argparse.ArgumentParser(
        description="Create a versioned firmware release package."
    )

    parser.add_argument("--binary", required=True)
    parser.add_argument("--board", required=True)
    parser.add_argument("--build-type", required=True)

    args = parser.parse_args()

    firmware = pathlib.Path(args.binary)

    if not firmware.exists():
        raise FileNotFoundError(f"Firmware binary not found: {firmware}")

    # -------------------------------------------------------
    # Generate firmware version
    # -------------------------------------------------------

    version = generate_version()

    # -------------------------------------------------------
    # Create release directory
    # release/
    #    260700/
    # -------------------------------------------------------

    root_release = pathlib.Path("release")
    release = root_release / version
    release.mkdir(parents=True, exist_ok=True)

    # -------------------------------------------------------
    # Store version for CI/CD
    # -------------------------------------------------------

    (release / "version.txt").write_text(version)

    # -------------------------------------------------------
    # Copy firmware
    # -------------------------------------------------------

    firmware_name = f"OBC-firmware-{version}.bin"

    destination = release / firmware_name

    shutil.copy2(firmware, destination)

    # -------------------------------------------------------
    # SHA256
    # -------------------------------------------------------

    digest = sha256(destination)

    with open(release / "firmware.sha256", "w") as f:
        f.write(f"{digest}  {firmware_name}\n")

    # -------------------------------------------------------
    # Manifest
    # -------------------------------------------------------

    manifest = {
        "schema_version": 1,
        "artifact_type": "firmware",
        "version": version,
        "board": args.board,
        "build_type": args.build_type,
        "firmware": firmware_name,
        "git_commit": git_commit(),
        "timestamp_utc": datetime.now(UTC).isoformat(),
        "sha256": digest,
        "size_bytes": destination.stat().st_size,
        "generated_by": "package_release.py",
    }

    with open(release / "manifest.json", "w") as f:
        json.dump(manifest, f, indent=4)

    print(f"Release package created: {release.resolve()}")

    github_output = os.getenv("GITHUB_OUTPUT")

    if github_output:
        with open(github_output, "a") as f:
            f.write(f"version={version}\n")


if __name__ == "__main__":
    main()
