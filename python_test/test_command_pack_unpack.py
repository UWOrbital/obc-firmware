from interfaces.obc_gs_interface.commands.python import (
    CmdCallbackId,
    pack_command,
    unpack_command,
)
from interfaces.obc_gs_interface.commands.python.command_factories import (
    create_cmd_ping,
    create_cmd_rtc_sync,
)


def test_command_pack_unpack():
    cmd_ping = create_cmd_ping()
    cmd_rtc_sync = create_cmd_rtc_sync(1234567)
    cmd_ping_packed = bytearray(pack_command(cmd_ping))
    cmd_rtc_sync_packed = bytearray(pack_command(cmd_rtc_sync))
    command_data = bytes(cmd_ping_packed + cmd_rtc_sync_packed + b"\x00")
    cmd_list = unpack_command(bytes(command_data))[0]

    assert cmd_list[0].id == CmdCallbackId.CMD_PING.value
    assert cmd_list[1].id == CmdCallbackId.CMD_RTC_SYNC.value

    # Check if the base parameters for each command are set correctly
    assert cmd_list[0].isTimeTagged == False
    assert cmd_list[1].isTimeTagged == False

    assert cmd_list[0].timestamp == 0
    assert cmd_list[1].timestamp == 0

    # Check if the time for RTC is correct
    assert cmd_list[1].rtcSync.unixTime == 1234567


def test_time_tagged_pack_unpack():
    cmd_ping = create_cmd_ping(1234567)
    cmd_ping_packed = bytearray(pack_command(cmd_ping))
    command_data = bytes(cmd_ping_packed + b"\x00")
    cmd_list = unpack_command(bytes(command_data))[0]

    assert cmd_list[0].id == CmdCallbackId.CMD_PING.value

    # Check if the time tagged ccommand is set correctly
    assert cmd_list[0].isTimeTagged == True

    assert cmd_list[0].timestamp == 1234567
