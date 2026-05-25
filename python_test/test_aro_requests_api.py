from datetime import datetime, timedelta
from decimal import Decimal

import pytest
from fastapi.testclient import TestClient
from gs.backend.data.enums.aro_requests import ARORequestStatus
from gs.backend.data.tables.aro_user_tables import AROUsers
from gs.backend.data.tables.transactional_tables import ARORequest
from gs.backend.main import app


@pytest.fixture
def client():
    return TestClient(app)


@pytest.fixture
def seeded_aro_requests(db_session):
    user = AROUsers(
        call_sign="ABC123",
        email="aro-user@test.com",
        first_name="ARO",
        last_name="User",
        phone_number="1234567890",
    )
    db_session.add(user)
    db_session.commit()

    base_time = datetime(2025, 1, 1, 0, 0, 0)
    requests = [
        ARORequest(
            aro_id=user.id,
            latitude=Decimal("10.000"),
            longitude=Decimal("20.000"),
            created_on=base_time,
            status=ARORequestStatus.PENDING,
        ),
        ARORequest(
            aro_id=user.id,
            latitude=Decimal("11.000"),
            longitude=Decimal("21.000"),
            created_on=base_time + timedelta(minutes=1),
            status=ARORequestStatus.COMPLETED,
        ),
        ARORequest(
            aro_id=user.id,
            latitude=Decimal("12.000"),
            longitude=Decimal("22.000"),
            created_on=base_time + timedelta(minutes=2),
            status=ARORequestStatus.CANCELLED,
        ),
    ]

    for request in requests:
        db_session.add(request)
    db_session.commit()

    return requests


def test_get_aro_requests_returns_data_field(client, seeded_aro_requests):
    response = client.get("/api/v1/mcc/requests/")

    assert response.status_code == 200
    body = response.json()
    assert "data" in body
    assert len(body["data"]) == 3


def test_get_aro_requests_count_and_order(client, seeded_aro_requests):
    response = client.get("/api/v1/mcc/requests/?count=2")

    assert response.status_code == 200
    data = response.json()["data"]
    assert len(data) == 2
    assert data[0]["status"] == ARORequestStatus.CANCELLED.value
    assert data[1]["status"] == ARORequestStatus.COMPLETED.value


def test_get_aro_requests_count_le_zero_returns_all(client, seeded_aro_requests):
    response = client.get("/api/v1/mcc/requests/?count=0")

    assert response.status_code == 200
    data = response.json()["data"]
    assert len(data) == 3


def test_get_aro_requests_offset(client, seeded_aro_requests):
    response = client.get("/api/v1/mcc/requests/?offset=1")

    assert response.status_code == 200
    data = response.json()["data"]
    assert len(data) == 2
    assert data[0]["status"] == ARORequestStatus.COMPLETED.value


def test_get_aro_requests_filters(client, seeded_aro_requests):
    response = client.get(f"/api/v1/mcc/requests/?filters={ARORequestStatus.PENDING.value}")

    assert response.status_code == 200
    data = response.json()["data"]
    assert len(data) == 1
    assert data[0]["status"] == ARORequestStatus.PENDING.value
