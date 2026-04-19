# TODO:(335) Improve loading the configuration

from gs.backend.config.cors_config import CORSConfig
from gs.backend.config.database_config import DatabaseConfig
from gs.backend.config.logger_config import LoggerConfig


class BackendConfiguration:
    """
    Class for storing backend configuration settings
    """

    def __init__(self) -> None:
        self.cors = CORSConfig()
        self.logger = LoggerConfig()
        self.db = DatabaseConfig()


settings = BackendConfiguration()
