from uuid import UUID

from gs.backend.data.data_wrappers.abstract_wrapper import AbstractWrapper  # SEE abstract_wrapper.py FOR LOGIC
from gs.backend.data.enums.aro_requests import ARORequestStatus
from gs.backend.data.tables.aro_user_tables import AROUserAuthToken, AROUserLogin, AROUsers
from gs.backend.data.tables.main_tables import MainCommand, MainTelemetry
from gs.backend.data.tables.transactional_tables import (
    ARORequest,
    Commands,
    CommsSession,
    Packet,
    PacketCommands,
    PacketTelemetry,
    Telemetry,
)


class AROUsersWrapper(AbstractWrapper[AROUsers, UUID]):
    """
    Data wrapper for AROUsers table.
    """

    model = AROUsers


class AROUserAuthTokenWrapper(AbstractWrapper[AROUserAuthToken, UUID]):
    """
    Data wrapper for AROUserAuthToken table.
    """

    model = AROUserAuthToken


class AROUserLoginWrapper(AbstractWrapper[AROUserLogin, UUID]):
    """
    Data wrapper for AROUserLogin table.
    """

    model = AROUserLogin


class ARORequestWrapper(AbstractWrapper[ARORequest, UUID]):
    """
    Data wrapper for ARORequest table.
    """

    model = ARORequest

    def get_requests(
        self,
        count: int = 100,
        offset: int = 0,
        filters: list[ARORequestStatus] | None = None,
    ) -> list[ARORequest]:
        """
        Get recent ARO requests, optionally filtered by status.

        :param count: Number of most recent requests to return. If count <= 0, returns all.
        :param offset: Starting index (for paging) in the descending-by-created_on request list.
        :param filters: Optional list of statuses to include.
        :return: A list of ARO requests matching criteria.
        """
        requests = self.get_all()

        if filters:
            filter_set = set(filters)
            requests = [request for request in requests if request.status in filter_set]

        requests.sort(key=lambda request: request.created_on, reverse=True)

        start = max(offset, 0)
        if count <= 0:
            return requests[start:]

        end = start + count
        return requests[start:end]


class MainCommandWrapper(AbstractWrapper[MainCommand, int]):
    """
    Data wrapper for MainCommand table.
    """

    model = MainCommand


class MainTelemetryWrapper(AbstractWrapper[MainTelemetry, int]):
    """
    Data wrapper for MainTelemetry table.
    """

    model = MainTelemetry


class CommsSessionWrapper(AbstractWrapper[CommsSession, UUID]):
    """
    Data wrapper for CommsSession table.
    """

    model = CommsSession


class PacketWrapper(AbstractWrapper[Packet, UUID]):
    """
    Data wrapper for Packet table.
    """

    model = Packet


class PacketCommandsWrapper(AbstractWrapper[PacketCommands, UUID]):
    """
    Data wrapper for PacketCommands table.
    """

    model = PacketCommands


class PacketTelemetryWrapper(AbstractWrapper[PacketTelemetry, UUID]):
    """
    Data wrapper for PacketTelemetry table.
    """

    model = PacketTelemetry


class CommandsWrapper(AbstractWrapper[Commands, UUID]):
    """
    Data wrapper for Commands table.
    """

    model = Commands

    def retrieve_floating_commands(self) -> list[Commands]:
        """
        Retrieves all commands which do not have a valid entry in
        the packet_commands table.
        A command which is not valid is considered as any command whose ID
        does not match with any command_id in the packet_commands table
        """
        packet_commands = PacketCommandsWrapper().get_all()
        packet_ids = {packet_command.command_id for packet_command in packet_commands}

        commands = self.get_all()
        floating_commands = [fc for fc in commands if fc.id not in packet_ids]

        return floating_commands


class TelemetryWrapper(AbstractWrapper[Telemetry, UUID]):
    """
    Data wrapper for Telemetry table.
    """

    model = Telemetry
