"""Verify that the decoder CLI uses the selected firmware's mapping."""

import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

REPO_ROOT = Path(__file__).resolve().parents[2]
SCRIPT = REPO_ROOT / "scripts" / "decode_binary_logs.py"


class TestBinaryLogDecoder(unittest.TestCase):
    def run_decoder(self, mapping, data):
        with tempfile.TemporaryDirectory() as directory:
            directory = Path(directory)
            (directory / "map.json").write_text(json.dumps(mapping))
            (directory / "log.bin").write_bytes(data)
            return subprocess.run(
                [
                    sys.executable,
                    str(SCRIPT),
                    "--mapping",
                    str(directory / "map.json"),
                    str(directory / "log.bin"),
                ],
                capture_output=True,
                text=True,
            )

    def test_golden_error_uses_selected_mapping_instead_of_bundled_path(self):
        # ERROR, file 0, line 42, error 0x12345678, no timestamp.
        result = self.run_decoder(
            {"format_version": 1, "files": ["firmware-specific/location.c"]},
            bytes.fromhex("a8 04 00 00 2a 00 78 56 34 12"),
        )
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(
            result.stdout, "ERROR -> firmware-specific/location.c:42 - 305419896\n"
        )

    def test_unknown_id_never_falls_back_to_bundled_mapping(self):
        # ID 1 exists in the bundled map but not in this build's map.
        result = self.run_decoder(
            {"files": ["only/source.c"]},
            bytes.fromhex("a8 0a 01 00 07 00 02 6f 6b"),
        )
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stdout, "INFO  -> <file:1>:7 - ok\n")

    def test_corrupt_stream_resynchronizes_using_existing_codec(self):
        valid = bytes.fromhex("a8 04 ff ff 2a 00 01 00 00 00")
        result = self.run_decoder(
            {"files": []},
            b"noise" + bytes.fromhex("a8 07") + valid + bytes.fromhex("a8 04"),
        )
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stdout, "ERROR -> <file:65535>:42 - 1\n")

    def test_rejects_invalid_mapping(self):
        for mapping in ({"files": "wrong"}, {"files": [None]}, {"files": [""]}, {}):
            with self.subTest(mapping=mapping):
                result = self.run_decoder(mapping, b"")
                self.assertEqual(result.returncode, 1)
                self.assertEqual(result.stdout, "")
                self.assertIn("Could not decode logs:", result.stderr)


if __name__ == "__main__":
    unittest.main()
