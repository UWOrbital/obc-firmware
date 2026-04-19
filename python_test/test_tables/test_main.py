import pytest
from gs.backend.data.tables.main_tables import MainCommand, MainTelemetry
from gs.backend.exceptions.exceptions import DatabaseError
from sqlmodel import Session, select


def test_main_command_basic(db_session: Session):
    test1 = MainCommand(id=1, name="Test 1", data_size=1, total_size=2, format="int 7 bytes", params="time")
    db_session.add(test1)
    db_session.commit()
    query = select(MainCommand)
    items = db_session.exec(query).all()
    assert len(items) == 1
    returned_item1 = items[0]
    assert returned_item1.id == 1
    assert returned_item1.data_size == 1
    assert returned_item1.total_size == 2
    assert returned_item1.name == "Test 1"
    assert returned_item1.format == "int 7 bytes"
    assert returned_item1.params == "time"


def test_main_telemetry_basic(db_session: Session):
    test1 = MainTelemetry(id=1, name="Test 1", data_size=1, total_size=2, format="int 7 bytes")
    db_session.add(test1)
    db_session.commit()
    query = select(MainTelemetry)
    items = db_session.exec(query).all()
    assert len(items) == 1
    returned_item1 = items[0]
    assert returned_item1.id == 1
    assert returned_item1.data_size == 1
    assert returned_item1.total_size == 2
    assert returned_item1.name == "Test 1"
    assert returned_item1.format == "int 7 bytes"


def test_main_command_valid():
    main_command = MainCommand(
        id=1, name="Test", params="param1,param2,param3", format="int,int,int", data_size=3, total_size=3
    )
    assert main_command.name == "Test"
    assert main_command.params == "param1,param2,param3"
    assert main_command.format == "int,int,int"
    assert main_command.data_size == 3
    assert main_command.total_size == 3


def test_main_command_no_params_and_no_format():
    main_command = MainCommand(
        id=1,
        name="Test",
        data_size=2,
        total_size=2,
    )
    assert main_command.name == "Test"
    assert main_command.params is None
    assert main_command.format is None
    assert main_command.data_size == 2
    assert main_command.total_size == 2


def test_main_command_no_format(db_session):
    with pytest.raises(DatabaseError, match="Missing format"):
        db_session.add(
            MainCommand(
                id=1,
                name="Test",
                params="param1,param2",
                data_size=2,
                total_size=2,
            )
        )
        db_session.commit()


def test_main_command_no_params(db_session):
    with pytest.raises(DatabaseError, match="Missing params"):
        db_session.add(
            MainCommand(
                id=1,
                name="Test",
                format="int,int",
                data_size=2,
                total_size=2,
            )
        )
        db_session.commit()


def test_main_command_params_format_mismatch(db_session):
    with pytest.raises(DatabaseError, match="Params and format do not have the same number of values"):
        db_session.add(
            MainCommand(
                id=1,
                name="Test",
                params="param1,param2,param3",
                format="int,int",
                data_size=2,
                total_size=2,
            )
        )
        db_session.commit()
