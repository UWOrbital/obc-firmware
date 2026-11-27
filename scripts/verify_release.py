#!/usr/bin/env python3

from __future__ import annotations

import argparse
import hashlib
import json
import pathlib
import sys


def sha256(path: pathlib.Path) -> str:

    h = hashlib.sha256()

    with open(path, "rb") as f:
        while chunk := f.read(8192):
            h.update(chunk)

    return h.hexdigest()


def main():

    parser = argparse.ArgumentParser()

    parser.add_argument(
        "--release",
        required=True,
        help="Release directory (example: release/260700)",
    )

    args = parser.parse_args()

    release = pathlib.Path(args.release)

    if not release.exists():
        sys.exit("Release directory not found")

    manifest = release / "manifest.json"

    sha = release / "firmware.sha256"

    if not manifest.exists():
        sys.exit("manifest.json missing")

    if not sha.exists():
        sys.exit("firmware.sha256 missing")

    firmware = next(release.glob("*.bin"), None)

    if firmware is None:
        sys.exit("Firmware binary missing")

    with open(manifest) as f:
        data = json.load(f)

    digest = sha256(firmware)

    if digest != data["sha256"]:
        sys.exit("Manifest SHA mismatch")

    expected = sha.read_text().split()[0]

    if digest != expected:
        sys.exit("SHA256 mismatch")

    print("Release package verified successfully.")


if __name__ == "__main__":
    main()
