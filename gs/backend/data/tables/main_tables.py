from typing import Final, TypeAlias

from pydantic import model_validator
from sqlalchemy import Integer
from sqlmodel import Field

from gs.backend.data.tables.base_model import BaseSQLModel
from gs.backend.exceptions.exceptions import DatabaseError

# Schema information
MAIN_SCHEMA_NAME: Final[str] = "main"

# Table names in database
MAIN_COMMAND_TABLE_NAME: Final[str] = "commands"
MAIN_TELEMETRY_TABLE_NAME: Final[str] = "telemetry"


MainTableID: TypeAlias = int
MainTableIDDatabase: TypeAlias = Integer


class MainCommand(BaseSQLModel, table=True):
    """
    Main command model.
    This table represents all the possible commands that can be issued.

    List of commands: `https://docs.google.com/spreadsheets/d/1XWXgp3--NHZ4XlxOyBYPS-M_LOU_ai-I6TcvotKhR1s/edit?gid=564815068#gid=564815068`
    """

    id: MainTableID = Field(primary_key=True, index=True)  # NOTE: Must be synced with obc_gs_command_id
    name: str
    params: str | None = None  # None if no params needed
    format: str | None = None  # None if no format needed
    data_size: int = Field(ge=0)
    total_size: int = Field(gt=0)

    # table information
    __tablename__ = MAIN_COMMAND_TABLE_NAME
    __table_args__ = {"schema": MAIN_SCHEMA_NAME}

    @model_validator(mode="after")
    def validate_params_format(self) -> "MainCommand":
        """
        Returns self if params and format are both None or have the same number
        of comma-separated values. If one of params or format is missing, or the
        numbers of comma-separated values do not match, raise DatabaseError.
        """
        if (
            self.format is None
            and self.params is None
            or (
                # TODO: Check if the params have valid types
                self.params is not None and self.format is not None and self.params.count(",") == self.format.count(",")
            )
        ):
            return self

        if self.params is None:
            raise DatabaseError("Missing params")

        elif self.format is None:
            raise DatabaseError("Missing format")

        else:
            raise DatabaseError("Params and format do not have the same number of values")


class MainTelemetry(BaseSQLModel, table=True):
    """
    Main telemetry model.
    This table represents all the possible telemetry that can be issued.

    List of telemetry: `https://docs.google.com/spreadsheets/d/1XWXgp3--NHZ4XlxOyBYPS-M_LOU_ai-I6TcvotKhR1s/edit?gid=0#gid=0`
    """

    id: MainTableID = Field(primary_key=True, index=True)  # NOTE: Must be synced with obc_gs_telemetry_id
    name: str
    format: str | None = None  # None if no format needed
    data_size: int = Field(gt=0)
    total_size: int = Field(gt=0)

    # table information
    __tablename__ = MAIN_TELEMETRY_TABLE_NAME
    __table_args__ = {"schema": MAIN_SCHEMA_NAME}
