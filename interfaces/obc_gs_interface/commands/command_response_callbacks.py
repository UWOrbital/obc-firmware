from collections import defaultdict
from collections.abc import Callable

from interfaces.obc_gs_interface.commands import CmdCallbackId, unpack_command_response
from interfaces.obc_gs_interface.commands.command_response_classes import (
    CmdI2CProbeRes,
    CmdRes,
    CmdRtcSyncRes,
    CmdVerifyCrcRes,
)


# Command specific parsing
def parse_cmd_with_no_data(cmd_response: CmdRes, data: bytes) -> CmdRes:
    """
    A function to parse the raw data from the response of CMD_EXEC_OBC_RESET

    :param cmd_response: Basic command response
    :param data: The raw bytes containing the data that needs to be parsed
    :return: CmdRes (i.e. A command response with no data for CMD_EXEC_OBC_RESET)
    """
    return cmd_response


def parse_cmd_rtc_sync(cmd_response: CmdRes, data: bytes) -> CmdRtcSyncRes:
    """
    A function to parse the raw data from the response of CMD_RTC_SYNC

    :param cmd_response: Basic command response
    :param data: The raw bytes containing the data that needs to be parsed
    :return: CmdRes (i.e. A command response with no data for CMD_RTC_SYNC)
    """
    # TODO: Implement this callback properly
    if cmd_response.cmd_id != CmdCallbackId.CMD_RTC_SYNC:
        raise ValueError("Wrong command id for parsing the rtc sync command")

    board_unixtime = int.from_bytes(data[:4], "little")

    return CmdRtcSyncRes(cmd_response.cmd_id, cmd_response.error_code, cmd_response.response_length, board_unixtime)


def parse_cmd_verify_crc(cmd_response: CmdRes, data: bytes) -> CmdVerifyCrcRes:
    """
    A function to parse the raw data from the response of CMD_VERIFY_CRC

    :param cmd_response: Basic command response
    :param data: The raw bytes containing the data that needs to be parsed
    :return: CmdVerifyCrcRes (i.e. The command response for CMD_VERIFY_CRC)
    """
    if cmd_response.cmd_id != CmdCallbackId.CMD_VERIFY_CRC:
        raise ValueError("Wrong command id for parsing the verify crc command")

    crc = int.from_bytes(data[:4], "little")
    return CmdVerifyCrcRes(cmd_response.cmd_id, cmd_response.error_code, cmd_response.response_length, crc)


def parse_cmd_i2c_probe(cmd_response: CmdRes, data: bytes) -> CmdI2CProbeRes:
    """
    A function to parse the raw data from the response of CMD_I2C_PROBE

    :param cmd_response: Basic command response
    :param data: The raw bytes containing the data that needs to be parsed
    :return: CmdVerifyCrcRes (i.e. The command response for CMD_I2C_PROBE)
    """
    if cmd_response.cmd_id != CmdCallbackId.CMD_I2C_PROBE:
        raise ValueError("Wrong command id for parsing the i2c probe command")

    valid_addresses: list[int] = []

    for i in range(cmd_response.response_length):
        valid_addresses.append(int.from_bytes(data[i : i + 1], "little"))

    return CmdI2CProbeRes(cmd_response.cmd_id, cmd_response.error_code, cmd_response.response_length, valid_addresses)


# Function array where each index corresponds to the command enum value + 1

parse_func_dict: dict[CmdCallbackId, Callable[..., CmdRes]] = defaultdict(lambda: parse_cmd_with_no_data)
parse_func_dict[CmdCallbackId.CMD_VERIFY_CRC] = parse_cmd_verify_crc
parse_func_dict[CmdCallbackId.CMD_RTC_SYNC] = parse_cmd_rtc_sync
parse_func_dict[CmdCallbackId.CMD_I2C_PROBE] = parse_cmd_i2c_probe


def parse_command_response(data: bytes) -> CmdRes:
    """
    A function that unpacks a command response and returns a response structure

    :param data: The bytes of data the command is packed in
    """
    cmd_response_raw, data_bytes = unpack_command_response(data)
    cmd_response = CmdRes(CmdCallbackId(cmd_response_raw.cmdId), cmd_response_raw.errCode, cmd_response_raw.dataLen)
    cmd_parsed = parse_func_dict[cmd_response.cmd_id](cmd_response, data_bytes)

    return cmd_parsed


if __name__ == "__main__":
    data_bytes = b"\x0b\x01\x04\x78\x56\x34\x12\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    print(parse_command_response(data_bytes))
