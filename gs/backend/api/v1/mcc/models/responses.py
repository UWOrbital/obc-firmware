from pydantic import BaseModel

from gs.backend.data.tables.main_tables import MainCommand
from gs.backend.data.tables.transactional_tables import Telemetry


class MainCommandsResponse(BaseModel):
    """
    The main commands response model.
    """

    data: list[MainCommand]


class TelemetryResponse(BaseModel):
    """
    The telemetry response model.
    """

    data: list[Telemetry]
