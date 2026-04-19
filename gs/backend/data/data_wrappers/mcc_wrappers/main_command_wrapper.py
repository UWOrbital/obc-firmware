from typing import Any

from sqlmodel import select

from gs.backend.data.database.engine import get_db_session
from gs.backend.data.tables.main_tables import MainCommand


def get_all_main_commands() -> list[MainCommand]:
    """
    Get all data wrapper for MainCommand

    :return: a list of all main_commands
    """
    with get_db_session() as session:
        commands = list(session.exec(select(MainCommand)).all())
        return commands


def create_main_command(command_data: dict[str, Any]) -> MainCommand:
    """
    Post data wrapper for MainCommand

    :param command_data: the JSON object of the main_command to be created
    :return: the newly created main_command
    """
    with get_db_session() as session:
        command = MainCommand(**command_data)
        session.add(command)
        session.commit()
        session.refresh(command)
        return command


def delete_main_command_by_id(command_id: int) -> MainCommand:
    """
    Delete data wrapper for MainCommand

    :param command_id: id of main_command to be deleted
    :return: the deleted main_command
    """
    with get_db_session() as session:
        command = session.get(MainCommand, command_id)
        if not command:
            raise ValueError("Main command not found.")
        session.delete(command)
        session.commit()
        return command
