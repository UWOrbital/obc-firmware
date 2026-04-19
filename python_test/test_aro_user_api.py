import pytest
from fastapi.testclient import TestClient
from gs.backend.main import app


@pytest.fixture
def client():
    return TestClient(app)


# Test data for user 1
@pytest.fixture
def user1_data():
    return {
        "call_sign": "ABCDEF",
        "email": "bob@test.com",
        "first_name": "Bob",
        "last_name": "Smith",
        "phone_number": "123456789",
    }


# Test data for user 2
@pytest.fixture
def user2_data():
    return {
        "call_sign": "KEVWAN",
        "email": "kevian@gmail.com",
        "first_name": "kevin",
        "last_name": "wan",
        "phone_number": "8888888888",
    }


# Test creating user1
@pytest.fixture
def test_user1_creation(client, user1_data):
    response = client.post(
        "/api/v1/aro/user/create_user", json=user1_data, headers={"Content-Type": "application/json"}
    )

    assert response.status_code == 200
    user = response.json()["data"]
    assert user["email"] == user1_data["email"]
    assert user["call_sign"] == user1_data["call_sign"]
    assert user["first_name"] == user1_data["first_name"]
    assert user["last_name"] == user1_data["last_name"]
    assert user["phone_number"] == user1_data["phone_number"]

    return user


# Test creating user2
@pytest.fixture
def test_user2_creation(client, user2_data):
    response = client.post(
        "/api/v1/aro/user/create_user", json=user2_data, headers={"Content-Type": "application/json"}
    )

    assert response.status_code == 200
    user = response.json()["data"]
    assert user["email"] == user2_data["email"]
    assert user["call_sign"] == user2_data["call_sign"]
    assert user["first_name"] == user2_data["first_name"]
    assert user["last_name"] == user2_data["last_name"]
    assert user["phone_number"] == user2_data["phone_number"]

    return user


# Test getting user1 by ID
def test_get_user1_by_id(client, test_user1_creation):
    user_id = test_user1_creation["id"]
    res = client.get(f"/api/v1/aro/user/get_user/{user_id}")
    assert res.status_code == 200
    assert res.json()["data"]["id"] == user_id


# Test getting user2 by ID
def test_get_user2_by_id(client, test_user2_creation):
    user_id = test_user2_creation["id"]
    res = client.get(f"/api/v1/aro/user/get_user/{user_id}")
    assert res.status_code == 200
    assert res.json()["data"]["id"] == user_id


# Test getting all users
def test_get_all_users(client, test_user1_creation, test_user2_creation):
    res = client.get("/api/v1/aro/user/get_all_users")
    assert res.status_code == 200
    all_users = res.json()["data"]
    assert len(all_users) == 2

    # Check user1
    user1_id = test_user1_creation["id"]
    user1_from_response = next(user for user in all_users if user["id"] == user1_id)
    assert user1_from_response["call_sign"] == test_user1_creation["call_sign"]
    assert user1_from_response["email"] == test_user1_creation["email"]
    assert user1_from_response["first_name"] == test_user1_creation["first_name"]
    assert user1_from_response["last_name"] == test_user1_creation["last_name"]
    assert user1_from_response["phone_number"] == test_user1_creation["phone_number"]

    # Check user2
    user2_id = test_user2_creation["id"]
    user2_from_response = next(user for user in all_users if user["id"] == user2_id)
    assert user2_from_response["email"] == test_user2_creation["email"]
    assert user2_from_response["call_sign"] == test_user2_creation["call_sign"]
    assert user2_from_response["first_name"] == test_user2_creation["first_name"]
    assert user2_from_response["last_name"] == test_user2_creation["last_name"]
    assert user2_from_response["phone_number"] == test_user2_creation["phone_number"]


# Test deleting user1
def test_user1_deletion(client, test_user1_creation, test_user2_creation):
    user_id = test_user1_creation["id"]
    res = client.delete(f"/api/v1/aro/user/delete_user/{user_id}", headers={"Content-Type": "application/json"})

    assert res.status_code == 200
    deleted_user = res.json()["data"]
    assert deleted_user["id"] == user_id
    assert deleted_user["email"] == test_user1_creation["email"]
    assert deleted_user["call_sign"] == test_user1_creation["call_sign"]
    assert deleted_user["first_name"] == test_user1_creation["first_name"]
    assert deleted_user["last_name"] == test_user1_creation["last_name"]
    assert deleted_user["phone_number"] == test_user1_creation["phone_number"]
