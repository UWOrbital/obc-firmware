from typing import Any
from uuid import UUID

from sqlmodel import select

from gs.backend.data.database.engine import get_db_session
from gs.backend.data.tables.transactional_tables import CommsSession


def get_all_comms_sessions() -> list[CommsSession]:
    """
    Get all data wrapper for CommsSession

    :return: a list of all sessions
    """
    with get_db_session() as session:
        sessions = list(session.exec(select(CommsSession)).all())
        return sessions


def create_comms_session(session_data: dict[str, Any]) -> CommsSession:
    """
    Post data wrapper for CommsSession

    :param session_data: the JSON object of the comms_session to be created
    :return: the newly created comms_session
    """
    with get_db_session() as session:
        comms_session = CommsSession(**session_data)
        session.add(comms_session)
        session.commit()
        session.refresh(comms_session)
        return comms_session


def delete_telemetry_by_id(session_id: UUID) -> CommsSession:
    """
    Delete data wrapper for CommsSession

    :param session_id: UUID of session to be deleted
    :return: the deleted session
    """
    with get_db_session() as session:
        comms_session = session.get(CommsSession, session_id)
        if not comms_session:
            raise ValueError("Comms session not found.")
        session.delete(comms_session)
        session.commit()
        return comms_session
