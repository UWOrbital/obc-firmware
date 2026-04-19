from pydantic import BaseModel

from gs.backend.data.tables.aro_user_tables import AROUsers


class AllUsersResponse(BaseModel):
    """
    The users response model.
    """

    data: list[AROUsers]


class UserResponse(BaseModel):
    """
    Single user response model.
    """

    data: AROUsers
