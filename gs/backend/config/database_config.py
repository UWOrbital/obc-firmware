from pydantic import SecretStr
from pydantic_settings import BaseSettings, SettingsConfigDict


class DatabaseConfig(BaseSettings):
    """
    Pydantic class for storing database configuration settings
    """

    model_config = SettingsConfigDict(env_prefix="GS_DATABASE_")

    user: str
    password: SecretStr
    location: str
    port: int
    name: str

    @property
    def connection_string(self) -> str:
        """
        Returns the database connection string
        """

        pwd = self.password.get_secret_value()
        return f"postgresql://{self.user}:{pwd}@{self.location}:{self.port}/{self.name}"
