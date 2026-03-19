#!/usr/bin/env python3

import os
import sys
from pathlib import Path

from serial import Serial

from obc.tools.python.app_update import send_bin

EXIT_OK = 0
EXIT_BAD_ARGS = 1
EXIT_FW_NOT_FOUND = 2
EXIT_FW_INVALID = 3
EXIT_SERIAL_NOT_FOUND = 4
EXIT_SERIAL_NO_PERMISSION = 5


# Helper logger function for this
def error_log(msg: str, code: int) -> None:
    """
    Print an error message and terminate execution with a specific exit code

    Args:
        msg: Human-readable error message.
        code: Process exit code.
    """
    print(f"ERROR: {msg}")
    sys.exit(code)


def main() -> int:
    """
    Validate inputs and flash firmware onto the target hardware

    Returns:
        Exit status code.
    """
    if len(sys.argv) != 3:
        print("Usage: hil_main.py <serial_device> <firmware.bin>")
        return EXIT_BAD_ARGS

    serial_dev = sys.argv[1]
    firmware_path = Path(sys.argv[2]).resolve()

    # Firmware validation
    if not firmware_path.exists():
        error_log(f"Firmware file not found: {firmware_path}", EXIT_FW_NOT_FOUND)

    if firmware_path.suffix != ".bin":
        error_log("Firmware must be a .bin file", EXIT_FW_INVALID)

    # Serial device validation (filesystem level)
    if not os.path.exists(serial_dev):
        error_log(f"Serial device does not exist: {serial_dev}", EXIT_SERIAL_NOT_FOUND)

    if not os.access(serial_dev, os.R_OK | os.W_OK):
        error_log(f"No permission to access serial device: {serial_dev}", EXIT_SERIAL_NO_PERMISSION)

    print(f"Serial device path validated: {serial_dev}")

    # Open + close once to ensure kernel-level readiness
    # If this fails, Python will terminate immediately (desired)
    ser = Serial(serial_dev, timeout=2)
    ser.close()

    print("[1] Flashing Hardware In The Loop")

    # Flashing (any failure aborts process)
    send_bin(str(firmware_path), serial_dev)

    print("[2] Flashing Completed Successfully")
    return EXIT_OK


if __name__ == "__main__":
    sys.exit(main())
