from pydantic import BaseModel

from gs.backend.data.tables.main_tables import MainCommand
from gs.backend.data.tables.transactional_tables import ARORequest


class MainCommandsResponse(BaseModel):
    """
    The main commands response model.
    """

    data: list[MainCommand]


class ARORequestsResponse(BaseModel):
    """
    The ARO requests response model.
    """

    data: list[ARORequest]
