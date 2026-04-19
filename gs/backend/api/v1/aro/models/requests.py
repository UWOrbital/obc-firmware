from pydantic import BaseModel, EmailStr


class UserRequest(BaseModel):
    """
    Model representing the user to be created.
    """

    call_sign: str
    email: EmailStr
    first_name: str
    last_name: str
    phone_number: str
