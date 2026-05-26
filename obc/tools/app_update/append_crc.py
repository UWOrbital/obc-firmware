from pathlib import Path
from sys import argv, exit
from zlib import crc32


def append_crc(input_path: str | Path, output_path: str | Path) -> None:
    """
    Read a file, calculate its CRC32 checksum, and create a new file with the checksum appended.

    :param input_path: Path to the input file to read
    :param output_path: Path where the output file (original data + CRC32) will be written
    :return: None
    """
    input_path = Path(input_path)
    output_path = Path(output_path)

    with input_path.open("rb") as f:
        data = f.read()

    crc = crc32(data) & 0xFFFFFFFF
    crc_bytes = crc.to_bytes(4, byteorder="little")

    with output_path.open("wb") as f:
        f.write(data)
        f.write(crc_bytes)

    print(f"CRC32: 0x{crc:08X} appended to {output_path.name}")


if __name__ == "__main__":
    """
    Entry point to program
    """
    # Checks for valid commands
    if len(argv) != 3:
        print("python append_crc.py")
        print('Invalid Arguments. Expected Command: "python3 append_crc.py input_path output_path"')
        exit(1)

    # Check if the file provided file exists and has a .bin suffix
    path = Path(argv[1]).resolve()
    if not path.is_file() or path.suffix != ".bin":
        print("Invalid file path")
        exit(1)

    # Run the program!
    append_crc(argv[1], argv[2])
