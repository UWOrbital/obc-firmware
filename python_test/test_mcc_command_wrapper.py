import pytest
from gs.backend.data.data_wrappers.mcc_wrappers import main_command_wrapper as wrapper
from gs.backend.data.database import engine as prod_engine
from gs.backend.data.tables.main_tables import MainCommand
from sqlmodel import Session, SQLModel, create_engine


def test_create_main_command(db_session):
    data = {"id": 1, "name": "Test Command", "params": None, "format": None, "data_size": 4, "total_size": 4}
    command = wrapper.create_main_command(data)

    assert command.id == 1
    assert command.name == "Test Command"

    # Verify it was actually inserted
    stored = db_session.get(MainCommand, 1)
    assert stored is not None
    assert stored.name == "Test Command"


def test_get_all_main_commands(db_session):
    # Insert a couple of rows manually
    db_session.add(MainCommand(id=1, name="CmdA", params=None, format=None, data_size=1, total_size=1))
    db_session.add(MainCommand(id=2, name="CmdB", params="x", format="y", data_size=2, total_size=2))
    db_session.commit()

    commands = wrapper.get_all_main_commands()

    assert len(commands) == 2
    names = {c.name for c in commands}
    assert names == {"CmdA", "CmdB"}


def test_delete_main_command_by_id(db_session):
    # Insert one command
    db_session.add(MainCommand(id=1, name="CmdX", params=None, format=None, data_size=1, total_size=1))
    db_session.commit()

    # Should delete successfully
    result = wrapper.delete_main_command_by_id(1)
    assert isinstance(result, MainCommand)
    assert db_session.get(MainCommand, 1) is None
