from typing import Any
from uuid import UUID

from sqlmodel import select

from gs.backend.data.database.engine import get_db_session
from gs.backend.data.tables.transactional_tables import PacketCommands


def get_all_packet_commands() -> list[PacketCommands]:
    """
    Get all data wrapper for PacketCommands

    :return: a list of all packet_commands
    """
    with get_db_session() as session:
        commands = list(session.exec(select(PacketCommands)).all())
        return commands


def create_packet_command(command_data: dict[str, Any]) -> PacketCommands:
    """
    Post data wrapper for PacketCommands

    :param command_data: the JSON object of the packet_command to be created
    :return: the newly created packet_command
    """
    with get_db_session() as session:
        command = PacketCommands(**command_data)
        session.add(command)
        session.commit()
        session.refresh(command)
        return command


def delete_packet_command_by_id(command_id: UUID) -> PacketCommands:
    """
    Delete data wrapper for PacketCommands

    :param command_id: UUID of packet_command to be deleted
    :return: the deleted packet_command
    """
    with get_db_session() as session:
        command = session.get(PacketCommands, command_id)
        if not command:
            raise ValueError("Packet command not found.")
        session.delete(command)
        session.commit()
        return command
