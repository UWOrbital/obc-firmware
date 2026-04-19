from typing import Any
from uuid import UUID

from sqlmodel import select

from gs.backend.data.database.engine import get_db_session
from gs.backend.data.tables.transactional_tables import PacketTelemetry


def get_all_packet_telemetries() -> list[PacketTelemetry]:
    """
    Get all data wrapper for PacketTelemetry

    :return: a list of all packet_telemetries
    """
    with get_db_session() as session:
        telemetries = list(session.exec(select(PacketTelemetry)).all())
        return telemetries


def create_packet_telemetry(telemetry_data: dict[str, Any]) -> PacketTelemetry:
    """
    Post data wrapper for PacketTelemetry

    :param command_data: the JSON object of the packet_telemetry to be created
    :return: the newly created packet_telemetry
    """
    with get_db_session() as session:
        telemetry = PacketTelemetry(**telemetry_data)
        session.add(telemetry)
        session.commit()
        session.refresh(telemetry)
        return telemetry


def delete_packet_telemetry_by_id(telemetry_id: UUID) -> PacketTelemetry:
    """
    Delete data wrapper for PacketTelemetry

    :param command_id: UUID of packet_telemetry to be deleted
    :return: the deleted packet_telemetry
    """
    with get_db_session() as session:
        telemetry = session.get(PacketTelemetry, telemetry_id)
        if not telemetry:
            raise ValueError("Packet telemetry not found.")
        session.delete(telemetry)
        session.commit()
        return telemetry
