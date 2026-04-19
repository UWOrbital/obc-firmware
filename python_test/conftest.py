import os
import subprocess
from datetime import datetime

os.environ.setdefault("GS_DATABASE_USER", "testuser")
os.environ.setdefault("GS_DATABASE_PASSWORD", "testpassword")
os.environ.setdefault("GS_DATABASE_LOCATION", "localhost")
os.environ.setdefault("GS_DATABASE_PORT", "5432")
os.environ.setdefault("GS_DATABASE_NAME", "testdb")

import pytest
from gs.backend.data.database.engine import setup_database
from gs.backend.data.tables.transactional_tables import CommsSession
from sqlalchemy import Engine, NullPool
from sqlmodel import Session, create_engine


@pytest.fixture
def db_engine(postgresql) -> Engine:
    """
    Creates a database engine fixture for the postgresql.
    This is a function level fixture.
    """
    connection = f"postgresql+psycopg://{postgresql.info.user}:@{postgresql.info.host}:{postgresql.info.port}/{postgresql.info.dbname}"
    return create_engine(connection, echo=False, poolclass=NullPool)


@pytest.fixture
def db_session(db_engine: Engine) -> Session:
    """
    Creates a database session fixture for the postgresql.
    This is a function level fixture.
    """
    with Session(db_engine) as session:
        setup_database(session)

        # Run Alembic migrations to create tables
        repo_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
        env = os.environ.copy()
        env["SQLALCHEMY_DATABASE_URL"] = str(db_engine.url)
        subprocess.run(["alembic", "upgrade", "head"], cwd=repo_root, env=env, check=True, capture_output=True)

        return session


@pytest.fixture
def default_start_time() -> datetime:
    return datetime(2025, 1, 1, 12, 25, 38)


@pytest.fixture
def default_comms_session(default_start_time: datetime) -> CommsSession:
    """
    Creates the comms session
    This is a function level fixture.
    """
    comms_session_item = CommsSession(start_time=default_start_time)
    return comms_session_item


@pytest.fixture(autouse=True)
def test_get_db_session(monkeypatch, db_session: Session):
    """
    When testing any database function that requires the `get_db_session()` function, you must add the module path to the list below.
    """
    path_list: list[str] = [
        "gs.backend.data.data_wrappers.abstract_wrapper",
        "gs.backend.data.data_wrappers.aro_wrapper.aro_request_wrapper",
        "gs.backend.data.data_wrappers.aro_wrapper.aro_user_data_wrapper",
        "gs.backend.data.data_wrappers.aro_wrapper.aro_user_auth_token_wrapper",
        "gs.backend.data.data_wrappers.aro_wrapper.aro_user_login_wrapper",
        "gs.backend.data.data_wrappers.mcc_wrappers.commands_wrapper",
        "gs.backend.data.data_wrappers.mcc_wrappers.comms_session_wrapper",
        "gs.backend.data.data_wrappers.mcc_wrappers.main_command_wrapper",
        "gs.backend.data.data_wrappers.mcc_wrappers.main_telemetry_wrapper",
        "gs.backend.data.data_wrappers.mcc_wrappers.packet_commands_wrapper",
        "gs.backend.data.data_wrappers.mcc_wrappers.packet_telemetry_wrapper",
        "gs.backend.data.data_wrappers.mcc_wrappers.packet_wrapper",
        "gs.backend.data.data_wrappers.mcc_wrappers.telemetry_wrapper",
    ]

    for path in path_list:
        monkeypatch.setattr(path + ".get_db_session", lambda: db_session, raising=True)
