from interfaces import RS_DECODED_DATA_SIZE
from interfaces.obc_gs_interface.commands.python import (
    CmdMsg,
    pack_command,
)


def command_multi_pack(commands: list[CmdMsg]) -> list[bytes]:
    """
    A function that generates bytes based on the commands passed in

    :param command: A list of commands to convert to bytes
    :return: The commands fully packed in sequence and ready to be encoded. Depending on the number of commands, this
             function will return a list of byte strings, each containing the maximum possible amount of commands. Note,
             this function will always pad each byte string with 0x00 to 223 bytes
    """
    data: bytearray = bytearray(b"")
    command_list: list[bytes] = []

    for command in commands:
        command_packed = bytearray(pack_command(command))
        # NOTE: We check its greater than the max size - 1 as we need the last byte to be 0x00
        if (len(data) + len(command_packed)) > RS_DECODED_DATA_SIZE - 1:
            command_list.append(bytes(data).ljust(RS_DECODED_DATA_SIZE, b"\x00"))
            data = bytearray(b"")

        data += command_packed

    # This is for commands that have not been padded at the end of the for loop
    if data != b"":
        command_list.append(bytes(data).ljust(RS_DECODED_DATA_SIZE, b"\x00"))

    return command_list
