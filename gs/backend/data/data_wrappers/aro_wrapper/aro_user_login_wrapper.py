from uuid import UUID

from sqlmodel import select

from gs.backend.data.database.engine import get_db_session
from gs.backend.data.tables.aro_user_tables import AROUserLogin


def get_all_logins() -> list[AROUserLogin]:
    """
    Gets all the logins
    """
    with get_db_session() as session:
        user_logins = list(session.exec(select(AROUserLogin)).all())
        return user_logins


def add_login(email: str, pwd: str, hash_algo: str, user_data_id: UUID, email_verification_token: str) -> AROUserLogin:
    """
    Add a new user login

    :param email: the email which the user used to sign up
    :param pwd: the password the user set
    :hash_algo: the hashing algorithm used to encrypt the password w the salt
    :user_data_id: the unique identifier which binds this login to the user which created it
    :email_verification_token: email verification token
    """
    with get_db_session() as session:
        # check if the user exists already
        existing_login = session.exec(select(AROUserLogin).where(AROUserLogin.email == email)).first()

        if existing_login:
            raise ValueError("User login already exists based on email")

        user_login = AROUserLogin(
            email=email,
            password=pwd,
            hashing_algorithm_name=hash_algo,
            user_data_id=user_data_id,
            email_verification_token=email_verification_token,
        )

        session.add(user_login)
        session.commit()
        session.refresh(user_login)
        return user_login


def delete_login_by_id(loginid: UUID) -> list[AROUserLogin]:
    """
    Use the .id to delete a user from table

    :param loginid: unique identifier of the target login
    """
    with get_db_session() as session:
        user_login = session.get(AROUserLogin, loginid)
        if user_login:
            session.delete(user_login)
            session.commit()
        else:
            raise ValueError("Login ID does not exist")

        return get_all_logins()
