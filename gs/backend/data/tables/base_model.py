from typing import Any

from sqlmodel import SQLModel
from sqlmodel._compat import SQLModelConfig, get_config_value, set_config_value


class BaseSQLModel(SQLModel):
    """
    Base SQL Model class. It performs validation on the model unlike the default SQLModel class with table=True.
    """

    model_config = SQLModelConfig(validate_assignment=True)

    def __init__(self, **data: dict[str, Any]) -> None:
        is_table = get_config_value(model=self, parameter="table", default=False)
        set_config_value(model=self, parameter="table", value=False)  # Makes it validate the model
        super().__init__(**data)
        set_config_value(model=self, parameter="table", value=is_table)
