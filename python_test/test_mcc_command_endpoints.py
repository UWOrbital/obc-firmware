from uuid import UUID, uuid4

import pytest
from fastapi.testclient import TestClient
from gs.backend.data.enums.transactional import CommandStatus
from gs.backend.data.tables.main_tables import MainCommand
from gs.backend.main import app


@pytest.fixture
def client():
    return TestClient(app)


@pytest.fixture(autouse=True)
def setup_main_commands(db_session):
    """Setup MainCommand records needed for testing"""
    # Create MainCommand entries that will be referenced by Commands
    main_commands = [
        MainCommand(id=1, name="TestCmd1", params=None, format=None, data_size=4, total_size=4),
        MainCommand(id=2, name="TestCmd2", params=None, format=None, data_size=4, total_size=4),
        MainCommand(id=3, name="TestCmd3", params=None, format=None, data_size=4, total_size=4),
        MainCommand(id=4, name="TestCmd4", params=None, format=None, data_size=4, total_size=4),
        MainCommand(id=5, name="TestCmd5", params=None, format=None, data_size=4, total_size=4),
        MainCommand(id=6, name="TestCmd6", params=None, format=None, data_size=4, total_size=4),
        MainCommand(id=10, name="TestCmd10", params=None, format=None, data_size=4, total_size=4),
        MainCommand(id=11, name="TestCmd11", params=None, format=None, data_size=4, total_size=4),
    ]
    for cmd in main_commands:
        db_session.add(cmd)
    db_session.commit()


# ---------------------------------------------Testing the POST endpoint--------------------------------------------- #


def test_create_command_success(client):
    """Test successful creation of a new command"""
    payload = {
        "status": CommandStatus.PENDING,
        "type_": 1,  # Assuming valid MainCommand ID
        "params": "test_params",
    }

    response = client.post("/api/v1/mcc/commands/create", json=payload)

    assert response.status_code == 200
    data = response.json()
    assert "id" in data
    assert data["status"] == CommandStatus.PENDING
    assert data["type_"] == 1
    assert data["params"] == "test_params"
    # Validate that id is a valid UUID
    UUID(data["id"])


def test_create_command_duplicate(client):
    """Test that creating a duplicate command is allowed and succeeds"""
    payload = {"status": CommandStatus.PENDING, "type_": 2, "params": "duplicate_test"}

    # Create the first command
    response1 = client.post("/api/v1/mcc/commands/create", json=payload)
    assert response1.status_code == 200
    command1_id = response1.json()["id"]

    # Create duplicate command (same payload except id will be different)
    response2 = client.post("/api/v1/mcc/commands/create", json=payload)
    assert response2.status_code == 200
    command2_id = response2.json()["id"]

    # Both commands should exist but have different IDs
    assert command1_id != command2_id
    assert response2.json()["status"] == CommandStatus.PENDING
    assert response2.json()["type_"] == 2
    assert response2.json()["params"] == "duplicate_test"


def test_create_command_with_null_params(client):
    """Test creating a command with null params"""
    payload = {"status": CommandStatus.SCHEDULED, "type_": 3, "params": None}

    response = client.post("/api/v1/mcc/commands/create", json=payload)

    assert response.status_code == 200
    data = response.json()
    assert data["params"] is None
    assert data["status"] == CommandStatus.SCHEDULED


def test_create_command_different_status(client):
    """Test creating commands with different status values"""
    statuses = [CommandStatus.PENDING, CommandStatus.SCHEDULED, CommandStatus.SENT]

    for idx, status in enumerate(statuses):
        payload = {
            "status": status,
            "type_": 4 + idx,  # Use different type_ to avoid duplicates
            "params": f"params_{status}",
        }

        response = client.post("/api/v1/mcc/commands/create", json=payload)
        assert response.status_code == 200
        assert response.json()["status"] == status


# ---------------------------------------------Testing the DELETE endpoint--------------------------------------------- #


def test_delete_command_success(client):
    """Test successful deletion of an existing command"""
    # First create a command to delete
    payload = {"status": CommandStatus.PENDING, "type_": 10, "params": "to_be_deleted"}

    create_response = client.post("/api/v1/mcc/commands/create", json=payload)
    assert create_response.status_code == 200
    command_id = create_response.json()["id"]

    # Delete the command
    delete_response = client.delete(f"/api/v1/mcc/commands/delete/{command_id}")

    assert delete_response.status_code == 200
    data = delete_response.json()
    assert data["message"] == f"Command with id {command_id} deleted successfully"


def test_delete_command_not_found(client):
    """Test deleting a non-existent command raises ValueError (unhandled exception)"""
    # Generate a random UUID that doesn't exist
    non_existent_id = uuid4()

    # The endpoint raises ValueError which is not caught by FastAPI
    # This causes the test client to raise an exception
    with pytest.raises(ValueError, match="Command not found."):
        client.delete(f"/api/v1/mcc/commands/delete/{non_existent_id}")


def test_delete_command_invalid_uuid(client):
    """Test deleting with an invalid UUID format returns 422 error"""
    invalid_uuid = "not-a-valid-uuid"

    response = client.delete(f"/api/v1/mcc/commands/delete/{invalid_uuid}")

    # FastAPI validation error for invalid UUID format
    assert response.status_code == 422


def test_delete_command_twice(client):
    """Test that deleting the same command twice fails on second attempt"""
    # Create a command
    payload = {"status": CommandStatus.PENDING, "type_": 11, "params": "delete_twice_test"}

    create_response = client.post("/api/v1/mcc/commands/create", json=payload)
    command_id = create_response.json()["id"]

    # First deletion should succeed
    delete_response1 = client.delete(f"/api/v1/mcc/commands/delete/{command_id}")
    assert delete_response1.status_code == 200

    # Second deletion should raise ValueError
    with pytest.raises(ValueError, match="Command not found."):
        client.delete(f"/api/v1/mcc/commands/delete/{command_id}")
