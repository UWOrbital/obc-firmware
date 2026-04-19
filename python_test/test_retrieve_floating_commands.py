from datetime import datetime
from uuid import uuid4

from gs.backend.data.data_wrappers.wrappers import (
    CommandsWrapper,
    CommsSessionWrapper,
    MainCommandWrapper,
    PacketCommandsWrapper,
    PacketWrapper,
)
from gs.backend.data.enums.transactional import MainPacketType


def test_retrieve_floating_commands_filters():
    pcw = PacketCommandsWrapper()
    cw = CommandsWrapper()
    mc = MainCommandWrapper()
    pw = PacketWrapper()
    csw = CommsSessionWrapper()

    packet_id = uuid4()
    cmd_type = mc.create(
        dict(
            id=1,
            name="test",
            data_size=1,
            total_size=1,
        )
    ).id

    comms_session = csw.create({"id": uuid4(), "start_time": datetime.now()})
    packet = pw.create(
        dict(
            id=packet_id,
            session_id=comms_session.id,
            raw_data=b"\x00",
            type_=MainPacketType.UPLINK,
            payload_data=b"\x00",
            offset=0,
        )
    )

    cmd_in_packet = cw.create(dict(id=uuid4(), type_=cmd_type))
    cmd_free = cw.create(dict(id=uuid4(), type_=cmd_type))
    cmd_free2 = cw.create(dict(id=uuid4(), type_=cmd_type))

    pcw.create(dict(packet_id=packet.id, command_id=cmd_in_packet.id))

    result = cw.retrieve_floating_commands()
    for command in result:
        assert command.id in [cmd_free.id, cmd_free2.id]


def test_retrieve_floating_commands_no_packet_commands():
    cw = CommandsWrapper()
    mc = MainCommandWrapper()
    cmd_type = mc.create(
        dict(
            id=2,
            name="test",
            data_size=1,
            total_size=1,
        )
    ).id

    cw.create(dict(id=uuid4(), type_=cmd_type))
    cw.create(dict(id=uuid4(), type_=cmd_type))

    result = cw.retrieve_floating_commands()
    expected = cw.get_all()
    assert {c.id for c in result} == {c.id for c in expected}
