from typing import Any
from uuid import UUID

from sqlmodel import select

from gs.backend.data.database.engine import get_db_session
from gs.backend.data.tables.transactional_tables import Telemetry


def get_all_telemetries() -> list[Telemetry]:
    """
    Get all data wrapper for Telemetry

    :return: a list of all telemetries
    """
    with get_db_session() as session:
        telemetries = list(session.exec(select(Telemetry)).all())
        return telemetries


def create_telemetry(telemetry_data: dict[str, Any]) -> Telemetry:
    """
    Post data wrapper for Telemetry

    :param telemetry_data: the JSON object of the telemetry to be created
    :return: the newly created telemetry
    """
    with get_db_session() as session:
        telemetry = Telemetry(**telemetry_data)
        session.add(telemetry)
        session.commit()
        session.refresh(telemetry)
        return telemetry


def delete_telemetry_by_id(telemetry_id: UUID) -> Telemetry:
    """
    Delete data wrapper for Telemetry

    :param telemetry_id: UUID of telemetry to be deleted
    :return: the deleted telemetry
    """
    with get_db_session() as session:
        telemetry = session.get(Telemetry, telemetry_id)
        if not telemetry:
            raise ValueError("Telemetry not found.")
        session.delete(telemetry)
        session.commit()
        return telemetry
