import pytest
from fastapi import FastAPI
from fastapi.testclient import TestClient
from gs.backend.exceptions.exception_handlers import setup_exception_handlers
from gs.backend.exceptions.exceptions import (
    BaseOrbitalError,
    DatabaseError,
    InvalidArgumentError,
    InvalidStateError,
    NotFoundError,
    ServiceError,
    SunPositionError,
    UnauthorizedError,
    UnknownError,
)
from starlette.status import (
    HTTP_400_BAD_REQUEST,
    HTTP_401_UNAUTHORIZED,
    HTTP_404_NOT_FOUND,
    HTTP_409_CONFLICT,
    HTTP_422_UNPROCESSABLE_ENTITY,
)


def create_exception_handler_client() -> TestClient:
    app = FastAPI()
    setup_exception_handlers(app)

    @app.get("/test_base_orbital_exceptions")
    async def test_base_orbital_exceptions():
        raise BaseOrbitalError("Test BaseOrbitalError")

    @app.get("/test_database_exceptions")
    async def test_database_exceptions():
        raise DatabaseError("Test DatabaseError")

    @app.get("/test_invalid_argument_exceptions")
    async def test_invalid_argument_exceptions():
        raise InvalidArgumentError("Test InvalidArgumentError")

    @app.get("/test_invalid_state_exceptions")
    async def test_invalid_state_exceptions():
        raise InvalidStateError("Test InvalidStateError")

    @app.get("/test_not_found_exceptions")
    async def test_not_found_exceptions():
        raise NotFoundError("Test NotFoundError")

    @app.get("/test_service_exceptions")
    async def test_service_exceptions():
        raise ServiceError("Test ServiceError")

    @app.get("/test_sun_position_exceptions")
    async def test_sun_position_exceptions():
        raise SunPositionError("Test SunPositionError")

    @app.get("/test_unauthorized_exceptions")
    async def test_unauthorized_exceptions():
        raise UnauthorizedError("Test UnauthorizedError")

    @app.get("/test_unknown_exceptions")
    async def test_unknown_exceptions():
        raise UnknownError("Test UnknownError")

    return TestClient(app)


@pytest.mark.parametrize(
    "endpoint, status_code, response_key, expected_message",
    [
        ("/test_base_orbital_exceptions", HTTP_400_BAD_REQUEST, "message", "Test BaseOrbitalError"),
        ("/test_database_exceptions", HTTP_400_BAD_REQUEST, "message", "Test DatabaseError"),
        ("/test_invalid_argument_exceptions", HTTP_422_UNPROCESSABLE_ENTITY, "message", "Test InvalidArgumentError"),
        ("/test_invalid_state_exceptions", HTTP_409_CONFLICT, "message", "Test InvalidStateError"),
        ("/test_not_found_exceptions", HTTP_404_NOT_FOUND, "message", "Test NotFoundError"),
        ("/test_service_exceptions", HTTP_400_BAD_REQUEST, "message", "Test ServiceError"),
        ("/test_sun_position_exceptions", HTTP_400_BAD_REQUEST, "message", "Test SunPositionError"),
        ("/test_unauthorized_exceptions", HTTP_401_UNAUTHORIZED, "message", "Test UnauthorizedError"),
        ("/test_unknown_exceptions", HTTP_400_BAD_REQUEST, "detail", "Test UnknownError"),
    ],
)
def test_custom_exceptions(endpoint, status_code, response_key, expected_message):
    fastapi_test_client = create_exception_handler_client()
    response = fastapi_test_client.get(endpoint)
    assert response.status_code == status_code
    assert response.json().get(response_key) == expected_message
