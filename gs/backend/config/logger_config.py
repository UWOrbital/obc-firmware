from collections.abc import Sequence

from pydantic_settings import BaseSettings, SettingsConfigDict


class LoggerConfig(BaseSettings):
    """
    Pydantic class for storing logger middleware configuration settings
    """

    model_config = SettingsConfigDict(env_prefix="LOGGER_")

    excluded_endpoints: Sequence[str] = []
