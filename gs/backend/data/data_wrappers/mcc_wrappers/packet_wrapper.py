from typing import Any
from uuid import UUID

from sqlmodel import select

from gs.backend.data.database.engine import get_db_session
from gs.backend.data.tables.transactional_tables import Packet


def get_all_packets() -> list[Packet]:
    """
    Get all data wrapper for Packet

    :return: a list of all packets
    """
    with get_db_session() as session:
        packets = list(session.exec(select(Packet)).all())
        return packets


def create_packet(packet_data: dict[str, Any]) -> Packet:
    """
    Post data wrapper for Packet

    :param packet_data: the JSON object of the packet to be created
    :return: the newly created packet
    """
    with get_db_session() as session:
        packet = Packet(**packet_data)
        session.add(packet)
        session.commit()
        session.refresh(packet)
        return packet


def delete_packet_by_id(packet_id: UUID) -> Packet:
    """
    Delete data wrapper for Packet

    :param packet_id: UUID of packet to be deleted
    :return: the deleted packet
    """
    with get_db_session() as session:
        packet = session.get(Packet, packet_id)
        if not packet:
            raise ValueError("Packet not found.")
        session.delete(packet)
        session.commit()
        return packet
