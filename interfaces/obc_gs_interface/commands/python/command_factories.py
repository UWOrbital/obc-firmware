from collections.abc import Callable
from ctypes import (
    c_uint,
    c_uint16,
    c_uint32,
)

from interfaces.obc_gs_interface.commands.python import CmdCallbackId, CmdMsg, ProgrammingSession

# ######################################################################
# ||                                                                  ||
# ||                        Command Factories                         ||
# ||                                                                  ||
# ######################################################################
# NOTE: Update these when adding in new commands


def create_cmd_end_of_frame(unixtime_of_execution: int | None = None) -> CmdMsg:
    """
    Function to create a CmdMsg structure for CMD_END_OF_FRAME

    :param unixtime_of_execution: A time of when to execute a certain event,
                                  by default, it is set to None (i.e. a specific
                                  time is not needed)
    :return: CmdMsg structure for CMD_END_OF_FRAME
    """
    cmd_msg = CmdMsg(unixtime_of_execution)
    cmd_msg.id = CmdCallbackId.CMD_END_OF_FRAME
    return cmd_msg


def create_cmd_exec_obc_reset(unixtime_of_execution: int | None = None) -> CmdMsg:
    """
    Function to create a CmdMsg structure for CMD_EXEC_OBC_RESET

    :param unixtime_of_execution: A time of when to execute a certain event,
                                  by default, it is set to None (i.e. a specific
                                  time is not needed)
    :return: CmdMsg structure for CMD_EXEC_OBC_RESET
    """
    cmd_msg = CmdMsg(unixtime_of_execution)
    cmd_msg.id = CmdCallbackId.CMD_EXEC_OBC_RESET
    return cmd_msg


def create_cmd_rtc_sync(time: int, unixtime_of_execution: int | None = None) -> CmdMsg:
    """
    Function to create a CmdMsg structure for CMD_RTC_SYNC

    :param time: Unixtime as an integer
    :param unixtime_of_execution: A time of when to execute a certain event,
                                  by default, it is set to None (i.e. a specific
                                  time is not needed)
    :return: CmdMsg structure for CMD_RTC_SYNC
    """
    cmd_msg = CmdMsg(unixtime_of_execution)
    cmd_msg.id = CmdCallbackId.CMD_RTC_SYNC
    cmd_msg.rtcSync.unixTime = c_uint32(time)
    return cmd_msg


def create_cmd_downlink_logs_next_pass(log_level: int, unixtime_of_execution: int | None = None) -> CmdMsg:
    """
    Function to create a CmdMsg structure for CMD_DOWNLINK_LOGS_NEXT_PASS

    :param log_level: The Log Level for the logs
    :param unixtime_of_execution: A time of when to execute a certain event,
                                  by default, it is set to None (i.e. a specific
                                  time is not needed)
    :return: CmdMsg structure for CMD_DOWNLINK_LOGS_NEXT_PASS
    """
    if log_level > 255:
        raise ValueError("Log level passed is too large (cannot be encoded into a c_uint8)")
    cmd_msg = CmdMsg(unixtime_of_execution)
    cmd_msg.id = CmdCallbackId.CMD_DOWNLINK_LOGS_NEXT_PASS
    cmd_msg.downlinkLogsNextPass.logLevel = log_level
    return cmd_msg


def create_cmd_mirco_sd_format(unixtime_of_execution: int | None = None) -> CmdMsg:
    """
    Function to create a CmdMsg structure for CMD_MICRO_SD_FORMAT

    :param unixtime_of_execution: A time of when to execute a certain event,
                                  by default, it is set to None (i.e. a specific
                                  time is not needed)
    :return: CmdMsg structure for CMD_MICRO_SD_FORMAT
    """
    cmd_msg = CmdMsg(unixtime_of_execution)
    cmd_msg.id = CmdCallbackId.CMD_MICRO_SD_FORMAT
    return cmd_msg


def create_cmd_ping(unixtime_of_execution: int | None = None) -> CmdMsg:
    """
    Function to create a CmdMsg structure for CMD_PING

    :param unixtime_of_execution: A time of when to execute a certain event,
                                  by default, it is set to None (i.e. a specific
                                  time is not needed)
    :return: CmdMsg structure for CMD_PING
    """
    cmd_msg = CmdMsg(unixtime_of_execution)
    cmd_msg.id = CmdCallbackId.CMD_PING
    return cmd_msg


def create_cmd_downlink_telem(unixtime_of_execution: int | None = None) -> CmdMsg:
    """
    Function to create a CmdMsg structure for CMD_DOWNLINK_TELEM

    :param unixtime_of_execution: A time of when to execute a certain event,
                                  by default, it is set to None (i.e. a specific
                                  time is not needed)
    :return: CmdMsg structure for CMD_DOWNLINK_TELEM
    """
    cmd_msg = CmdMsg(unixtime_of_execution)
    cmd_msg.id = CmdCallbackId.CMD_DOWNLINK_TELEM
    return cmd_msg


def create_cmd_uplink_disc(unixtime_of_execution: int | None = None) -> CmdMsg:
    """
    Function to create a CmdMsg structure for CMD_UPLINK_DISC

    :param unixtime_of_execution: A time of when to execute a certain event,
                                  by default, it is set to None (i.e. a specific
                                  time is not needed)
    :return: CmdMsg structure for CMD_UPLINK_DISC
    """
    cmd_msg = CmdMsg(unixtime_of_execution)
    cmd_msg.id = CmdCallbackId.CMD_UPLINK_DISC
    return cmd_msg


def create_cmd_set_programming_session(
    programming_session: ProgrammingSession, unixtime_of_execution: int | None = None
) -> CmdMsg:
    """
    Function to create a CmdMsg structure for CMD_SET_PROGRAMMING_SESSION

    :param programming_session: The programming session to set the bootloader to
    :param unixtime_of_execution: A time of when to execute a certain event,
                                  by default, it is set to None (i.e. a specific
                                  time is not needed)
    :return: CmdMsg structure for CMD_SET_PROGRAMMING_SESSION
    """
    cmd_msg = CmdMsg(unixtime_of_execution)
    cmd_msg.id = CmdCallbackId.CMD_SET_PROGRAMMING_SESSION
    cmd_msg.setProgrammingSession.programmingSession = c_uint(programming_session.value)
    return cmd_msg


def create_cmd_erase_app(unixtime_of_execution: int | None = None) -> CmdMsg:
    """
    Function to create a CmdMsg structure for CMD_ERASE_APP

    :param unixtime_of_execution: A time of when to execute a certain event,
                                  by default, it is set to None (i.e. a specific
                                  time is not needed)
    :return: CmdMsg structure for CMD_ERASE_APP
    """
    cmd_msg = CmdMsg(unixtime_of_execution)
    cmd_msg.id = CmdCallbackId.CMD_ERASE_APP
    return cmd_msg


def create_cmd_download_data(
    programming_session: ProgrammingSession, length: int, address: int, unixtime_of_execution: int | None = None
) -> CmdMsg:
    """
    Function to create a CmdMsg structure for CMD_DOWNLOAD_DATA

    :param programming_session: Defines the programming session of the current packet
    :param length: The length of the data to be downloaded
    :param address: The address on the board where the data should be written to
    :param unixtime_of_execution: A time of when to execute a certain event,
                                  by default, it is set to None (i.e. a specific
                                  time is not needed)
    :return: CmdMsg structure for CMD_DOWNLOAD_DATA
    """
    if length > 65535:
        raise ValueError("Length for download data command too large (cannot be encoded into a c_uint16)")
    if address > 4294967295:
        raise ValueError("Invalid address download data command (cannot be encoded into a c_uint32)")

    cmd_msg = CmdMsg(unixtime_of_execution)
    cmd_msg.id = CmdCallbackId.CMD_DOWNLOAD_DATA
    cmd_msg.downloadData.programmingSession = c_uint(programming_session.value)
    cmd_msg.downloadData.length = c_uint16(length)
    cmd_msg.downloadData.address = c_uint32(address)
    return cmd_msg


def create_cmd_verify_crc(unixtime_of_execution: int | None = None) -> CmdMsg:
    """
    Function to create a CmdMsg structure for CMD_VERIFY_CRC

    :param unixtime_of_execution: A time of when to execute a certain event,
                                  by default, it is set to None (i.e. a specific
                                  time is not needed)
    :return: CmdMsg structure for CMD_VERIFY_CRC
    """
    cmd_msg = CmdMsg(unixtime_of_execution)
    cmd_msg.id = CmdCallbackId.CMD_VERIFY_CRC
    return cmd_msg


def create_cmd_i2c_probe(unixtime_of_execution: int | None = None) -> CmdMsg:
    """
    Function to create a CmdMsg structure for CMD_I2C_PROBE

    :param unixtime_of_execution: A time of when to execute a certain event,
                                  by default, it is set to None (i.e. a specific
                                  time is not needed)
    :return: CmdMsg structure for CMD_I2C_PROBE
    """
    cmd_msg = CmdMsg(unixtime_of_execution)
    cmd_msg.id = CmdCallbackId.CMD_I2C_PROBE
    return cmd_msg


COMMAND_FACTORIES: list[Callable[..., CmdMsg]] = [
    create_cmd_end_of_frame,
    create_cmd_exec_obc_reset,
    create_cmd_rtc_sync,
    create_cmd_downlink_logs_next_pass,
    create_cmd_mirco_sd_format,
    create_cmd_ping,
    create_cmd_downlink_telem,
    create_cmd_uplink_disc,
    create_cmd_set_programming_session,
    create_cmd_erase_app,
    create_cmd_download_data,
    create_cmd_verify_crc,
    create_cmd_i2c_probe,
]
