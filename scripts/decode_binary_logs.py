#!/usr/bin/env python3
"""Decode binary logs using the source mapping saved with their firmware build."""

from __future__ import annotations

import argparse
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "interfaces"))
from obc_gs_interface.logging.log_codec import (  # noqa: E402
    decode_log_stream,
    entry_to_text,
    load_file_id_mapping,
)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--mapping", type=Path, required=True, help="Firmware build's log_file_ids.json"
    )
    parser.add_argument(
        "binary_log", type=Path, help="Binary log captured from that firmware"
    )
    args = parser.parse_args(argv)
    try:
        paths = load_file_id_mapping(args.mapping)
        if not isinstance(paths, list) or any(
            not isinstance(path, str) or not path for path in paths
        ):
            raise ValueError("Mapping 'files' must be a list of nonempty source paths.")
        for entry in decode_log_stream(args.binary_log.read_bytes()):
            # Override the codec's bundled mapping, including its fallback.
            entry.file_path = (
                paths[entry.file_id]
                if entry.file_id < len(paths)
                else f"<file:{entry.file_id}>"
            )
            print(entry_to_text(entry))
    except (OSError, ValueError, KeyError, TypeError) as error:
        print(f"Could not decode logs: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
