from datetime import datetime
from uuid import UUID

from sqlmodel import select

from gs.backend.data.database.engine import get_db_session
from gs.backend.data.enums.aro_auth_token import AROAuthToken as AROEnums
from gs.backend.data.tables.aro_user_tables import AROUserAuthToken


def get_all_auth_tokens() -> list[AROUserAuthToken]:
    """
    Get all the auth tokens
    """
    with get_db_session() as session:
        auth_tokens = list(session.exec(select(AROUserAuthToken)).all())
        return auth_tokens


def add_auth_token(token: str, user_data_id: UUID, expiry: datetime, auth_type: AROEnums) -> AROUserAuthToken:
    """
    Add auth token to the db

    :param token: the auth token we want to add
    :param user_data_id: UUID which identifies which user this auth token belongs to
    :param expiry: the date in which this token expires
    :param auth_type: the type of auth token this is, can only be from AROAuthToken
    """
    with get_db_session() as session:
        auth_token = AROUserAuthToken(token=token, user_data_id=user_data_id, expiry=expiry, auth_type=auth_type)

        session.add(auth_token)
        session.commit()
        session.refresh(auth_token)
        return auth_token


def delete_auth_token_by_id(token_id: UUID) -> list[AROUserAuthToken]:
    """
    Delete the auth token based on the token id

    :param token_id: the unique identifier for a particular auth token
    """

    with get_db_session() as session:
        auth_token = session.get(AROUserAuthToken, token_id)

        if auth_token:
            session.delete(auth_token)
            session.commit()
        else:
            print("Token does not exist")

        return get_all_auth_tokens()
