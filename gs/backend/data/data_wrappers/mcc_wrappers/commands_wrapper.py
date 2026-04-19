from typing import Any
from uuid import UUID

from sqlmodel import select

from gs.backend.data.database.engine import get_db_session
from gs.backend.data.tables.transactional_tables import Commands


def get_all_commands() -> list[Commands]:
    """
    Get all data wrapper for Commands

    :return: a list of all commands
    """
    with get_db_session() as session:
        commands = list(session.exec(select(Commands)).all())
        return commands


def create_commands(command_data: dict[str, Any]) -> Commands:
    """
    Post data wrapper for Commands

    :param command_data: the JSON object of the command to be created
    :return: the newly created command
    """
    with get_db_session() as session:
        command = Commands(**command_data)
        session.add(command)
        session.commit()
        session.refresh(command)
        return command


def delete_commands_by_id(command_id: UUID) -> Commands:
    """
    Delete data wrapper for Commands

    :param command_id: UUID of command to be deleted
    :return: the deleted command
    """
    with get_db_session() as session:
        command = session.get(Commands, command_id)
        if not command:
            raise ValueError("Command not found.")
        session.delete(command)
        session.commit()
        return command
