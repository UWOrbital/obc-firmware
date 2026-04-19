from uuid import UUID

from fastapi import APIRouter

from gs.backend.api.v1.aro.models.requests import UserRequest
from gs.backend.api.v1.aro.models.responses import AllUsersResponse, UserResponse
from gs.backend.data.data_wrappers.wrappers import AROUsersWrapper

aro_user_router = APIRouter(tags=["ARO", "User Information"])


@aro_user_router.get("/get_all_users", response_model=AllUsersResponse)
async def get_all_users() -> AllUsersResponse:
    """
    Gets all users

    :return: all users
    """
    users = AROUsersWrapper().get_all()
    return AllUsersResponse(data=users)


@aro_user_router.get("/get_user/{userid}", response_model=UserResponse)
def get_user(userid: str) -> UserResponse:
    """
    Gets a user by ID

    :param userid: The unique identifier of the user
    :return: the user
    """
    user = AROUsersWrapper().get_by_id(UUID(userid))
    return UserResponse(data=user)


@aro_user_router.post("/create_user", response_model=UserResponse)
def create_user(payload: UserRequest) -> UserResponse:
    """
    Creates a user with the given payload
    :param payload: The data used to create a user
    :return: returns the user created
    """

    user = AROUsersWrapper().create(
        data={
            "call_sign": payload.call_sign,
            "email": payload.email,
            "first_name": payload.first_name,
            "last_name": payload.last_name,
            "phone_number": payload.phone_number,
        }
    )

    return UserResponse(data=user)


@aro_user_router.delete("/delete_user/{userid}", response_model=UserResponse)
def delete_user(userid: str) -> UserResponse:
    """
    Deletes a user based on the user ID
    :param userid: The unique identifier of the user to be deleted
    :return: returns the deleted user
    """
    deleted_user = AROUsersWrapper().delete_by_id(UUID(userid))
    return UserResponse(data=deleted_user)
