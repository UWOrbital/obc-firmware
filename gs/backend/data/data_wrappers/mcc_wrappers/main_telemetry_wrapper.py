from typing import Any

from sqlmodel import select

from gs.backend.data.database.engine import get_db_session
from gs.backend.data.tables.main_tables import MainTelemetry


def get_all_main_telemetries() -> list[MainTelemetry]:
    """
    Get all data wrapper for MainTelemetry

    :return: a list of all main_telemetries
    """
    with get_db_session() as session:
        telemetries = list(session.exec(select(MainTelemetry)).all())
        return telemetries


def create_main_telemetry(telemetry_data: dict[str, Any]) -> MainTelemetry:
    """
    Post data wrapper for MainTelemetry

    :param command_data: the JSON object of the main_telemetry to be created
    :return: the newly created main_telemetry
    """
    with get_db_session() as session:
        telemetry = MainTelemetry(**telemetry_data)
        session.add(telemetry)
        session.commit()
        session.refresh(telemetry)
        return telemetry


def delete_main_telemetry_by_id(telemetry_id: int) -> MainTelemetry:
    """
    Delete data wrapper for MainTelemetry

    :param command_id: id of main_telemetry to be deleted
    :return: the deleted main_telemetry
    """
    with get_db_session() as session:
        telemetry = session.get(MainTelemetry, telemetry_id)
        if not telemetry:
            raise ValueError("Main telemetry not found.")
        session.delete(telemetry)
        session.commit()
        return telemetry
