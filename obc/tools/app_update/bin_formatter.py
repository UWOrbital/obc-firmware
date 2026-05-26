from argparse import ArgumentParser
from dataclasses import dataclass
from pathlib import Path
from struct import calcsize, pack
from time import sleep
from typing import Final

from serial import PARITY_NONE, STOPBITS_TWO, Serial

from interfaces import OBC_UART_BAUD_RATE

# Total app header size (includes unused bytes)
APP_HEADER_SIZE: Final = 32


@dataclass
class BootloaderHeader:
    """Header for the application binary"""

    version: int
    bin_size: int
    board_type: int

    HEADER_FMT: Final[str] = "<III"

    def serialize(self) -> bytes:
        """Returns the serialized version of the object with extra padding to reach specified APP_HEADER_SIZE"""
        header_data = pack(BootloaderHeader.HEADER_FMT, self.version, self.bin_size, self.board_type)
        unused_bytes = self.get_unused_size()
        # Add padding for the unused bytes
        padding = bytes(unused_bytes)
        header_full = header_data + padding
        return header_full

    @staticmethod
    def get_used_size() -> int:
        """Returns the total size of the used header (equal to size of all BootloaderHeader fields)"""
        return calcsize(BootloaderHeader.HEADER_FMT)

    def get_unused_size(self) -> int:
        """Returns how much of the header size is unused"""
        return APP_HEADER_SIZE - BootloaderHeader.get_used_size()

    def __str__(self) -> str:
        """Returns a string representation of the object"""
        return (
            f"Header Size: {BootloaderHeader.get_used_size()}, Header Padding Size: "
            f"{BootloaderHeader.get_unused_size(self)}, Version: {self.version}, "
            f"Bin Size: {self.bin_size}, Board Type: {self.board_type}"
        )


# More fields will be added later
@dataclass
class CMakeData:
    """Build variables from CMake"""

    board_type: int


def create_bin(input_path: str, input_version: int) -> str:
    """
    Appends custom header data to a .bin file

    :param input_path: Path to .bin file to be modified and sent
    :param input_version: Header version
    :return: The output file path as a string
    """
    program_bin = Path(input_path).read_bytes()
    program_size_bytes = len(program_bin)

    # metadata binary should be in the same directory as the OBC binary
    metadata_file_path = input_path.replace("OBC-firmware.bin", "OBC-metadata.bin")
    cmake_data = read_metadata_file(metadata_file_path)

    header = BootloaderHeader(version=input_version, bin_size=program_size_bytes, board_type=cmake_data.board_type)
    header_bytes = header.serialize()

    print(header)  # TODO: Replace with logging

    output_path = input_path.replace(".bin", "_formatted.bin")
    output_obj = Path(output_path)
    output_obj.write_bytes(header_bytes + program_bin)

    return output_path


def read_metadata_file(metadata_path: str) -> CMakeData:
    """
    Creates CMakeData object from metadata file

    :param file_path: Path to metadata bin file to be parsed
    :return: CMakeData object
    """
    file_obj = Path(metadata_path)
    if not file_obj.exists():
        raise FileNotFoundError(f"Metadata file not found: {metadata_path}")
    data = file_obj.read_bytes()
    # The outputted CMake file will have the variables separated by \n
    cmake_variables = data.decode().split("\n")

    board_type = int(cmake_variables[0])
    # Extract other fields from cmake_variables if more data is written to OBC-metadata.bin in obc/CMakeLists.txt

    cmake_data = CMakeData(board_type=board_type)
    return cmake_data


def send_bin(file_path: str, com_port: str) -> None:
    """
    Sends .bin file over UART serial port

    :param file_path: Path to .bin file to be sent
    :param com_port: Com port for UART communication
    """

    file_obj = Path(file_path)
    data = file_obj.read_bytes()
    if len(data) < BootloaderHeader.get_used_size():
        print("File too small to contain header. Exiting...")
        return

    # Open serial port and write binary to device via UART
    with Serial(
        com_port,
        baudrate=OBC_UART_BAUD_RATE,
        parity=PARITY_NONE,
        stopbits=STOPBITS_TWO,
        timeout=1,
    ) as ser:
        # TODO: Improve transfer protocol

        # Start program download
        ser.write("d".encode("ascii"))
        sleep(0.1)

        # Send header
        ser.write(data[0:APP_HEADER_SIZE])
        sleep(0.1)

        # Wait for user to initiate transfer
        while input("Enter 1 to start program transfer: ") != "1":
            pass

        # Bootloader expects a 'D' to be sent before the app
        ser.write("D".encode("ascii"))
        sleep(0.1)

        # Send app in chunks of 128 bytes
        total_bytes_to_write = len(data) - APP_HEADER_SIZE
        num_bytes_written = 0
        while num_bytes_written < total_bytes_to_write:
            chunk_size = 128

            if total_bytes_to_write - num_bytes_written >= chunk_size:
                ser.write(data[APP_HEADER_SIZE + num_bytes_written : APP_HEADER_SIZE + num_bytes_written + chunk_size])
                num_bytes_written += chunk_size
                sleep(0.1)
            else:
                ser.write(data[num_bytes_written + APP_HEADER_SIZE :])
                num_bytes_written += total_bytes_to_write - num_bytes_written

            print(f"{num_bytes_written}/{total_bytes_to_write} bytes sent")

        print("Done writing app")


def arg_parse() -> ArgumentParser:
    """
    Returns the argument parser

    :return: Parser object
    """
    parser = ArgumentParser(description="Append custom data to .bin and send")

    # Add arguments
    parser.add_argument(
        "-i",
        required=True,
        dest="input_path",
        type=str,
        help="Path to the input .bin file.",
    )
    parser.add_argument("-p", required=True, dest="port", type=str, help="Serial port number")
    parser.add_argument(
        "-v",
        dest="version",
        type=int,
        default=0,
        help="Version of the application. Default is 0",
    )

    return parser


def main() -> None:
    """Entry point to script"""
    arg_parser = arg_parse()
    args = arg_parser.parse_args()

    output_file = create_bin(args.input_path, args.version)
    send_bin(output_file, args.port)


if __name__ == "__main__":
    main()
