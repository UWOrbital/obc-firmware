from typing import Any
from uuid import UUID

from fastapi import APIRouter

from gs.backend.data.data_wrappers.mcc_wrappers.commands_wrapper import (
    create_commands,
    delete_commands_by_id,
)
from gs.backend.data.tables.transactional_tables import Commands

commands_router = APIRouter(tags=["MCC", "Commands"])


@commands_router.post("/create")
async def create_command(payload: dict[str, Any]) -> Commands:
    """
    Create a new command.

    :param payload: The data used to create a command
    :return: returns the created command object
    """
    return create_commands(payload)


@commands_router.delete("/delete/{command_id}")
async def delete_command(command_id: UUID) -> dict[str, Any]:
    """
    Delete a command by ID.

    :param command_id: The id which is to be deleted.
    :return: returns a dict giving confirmation that command with id of command_id has been deleted.
    """
    delete_commands_by_id(command_id)
    return {"message": f"Command with id {command_id} deleted successfully"}
