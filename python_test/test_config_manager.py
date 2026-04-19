import importlib

import pytest
from gs.backend.config.config import BackendConfiguration, settings
from gs.backend.config.cors_config import CORSConfig
from gs.backend.config.database_config import DatabaseConfig
from gs.backend.config.logger_config import LoggerConfig
from pydantic import ValidationError


def test_logger_config_default():
    cfg = LoggerConfig()

    assert cfg.excluded_endpoints == []


def test_cors_config_default():
    cfg = CORSConfig()

    assert cfg.allow_origins == ["http://localhost:5173"]
    assert cfg.allow_credentials == True
    assert cfg.allow_methods == ["*"]
    assert cfg.allow_headers == ["*"]


def test_backend_configuration_from_env(monkeypatch):
    monkeypatch.setenv("LOGGER_EXCLUDED_ENDPOINTS", '["/test"]')

    monkeypatch.setenv("CORS_ALLOW_ORIGINS", '["http://localhost:5173"]')
    monkeypatch.setenv("CORS_ALLOW_CREDENTIALS", "True")
    monkeypatch.setenv("CORS_ALLOW_METHODS", '["*"]')
    monkeypatch.setenv("CORS_ALLOW_HEADERS", '["*"]')

    monkeypatch.setenv("GS_DATABASE_USER", "testuser")
    monkeypatch.setenv("GS_DATABASE_PASSWORD", "testpassword")
    monkeypatch.setenv("GS_DATABASE_LOCATION", "localhost")
    monkeypatch.setenv("GS_DATABASE_PORT", "5432")
    monkeypatch.setenv("GS_DATABASE_NAME", "testdb")

    cfg = BackendConfiguration()

    assert "/test" in cfg.logger.excluded_endpoints
    assert "http://localhost:5173" in cfg.cors.allow_origins
    assert cfg.db.user == "testuser"
    assert cfg.db.password.get_secret_value() == "testpassword"
    assert cfg.db.location == "localhost"
    assert cfg.db.port == 5432
    assert cfg.db.name == "testdb"


def test_database_connection_string(monkeypatch):
    monkeypatch.setenv("GS_DATABASE_USER", "testuser")
    monkeypatch.setenv("GS_DATABASE_PASSWORD", "testpassword")
    monkeypatch.setenv("GS_DATABASE_LOCATION", "localhost")
    monkeypatch.setenv("GS_DATABASE_PORT", "5432")
    monkeypatch.setenv("GS_DATABASE_NAME", "testdb")

    cfg = BackendConfiguration()
    db = cfg.db

    assert db.password.get_secret_value() == "testpassword"
    expected_url = "postgresql://testuser:testpassword@localhost:5432/testdb"
    assert db.connection_string == expected_url


def test_database_missing_env(monkeypatch):
    monkeypatch.setenv("GS_DATABASE_USER", "testuser")
    monkeypatch.setenv("GS_DATABASE_PASSWORD", "testpassword")
    monkeypatch.setenv("GS_DATABASE_LOCATION", "localhost")
    monkeypatch.setenv("GS_DATABASE_PORT", "5432")
    monkeypatch.setenv("GS_DATABASE_NAME", "testdb")

    monkeypatch.delenv("GS_DATABASE_PASSWORD")

    with pytest.raises(ValidationError):
        DatabaseConfig()


def test_invalid_env(monkeypatch):
    monkeypatch.setenv("GS_DATABASE_PORT", "test")
    monkeypatch.setenv("CORS_ALLOW_CREDENTIALS", "3")
    monkeypatch.setenv("LOGGER_EXCLUDED_ENDPOINTS", "3")

    with pytest.raises(ValidationError):
        DatabaseConfig()

    with pytest.raises(ValidationError):
        CORSConfig()

    with pytest.raises(ValidationError):
        LoggerConfig()
