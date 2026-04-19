from pydantic_settings import BaseSettings, SettingsConfigDict


class CORSConfig(BaseSettings):
    """
    Pydantic class for storing CORS middleware configuration settings
    """

    model_config = SettingsConfigDict(env_prefix="CORS_")

    allow_origins: list[str] = ["http://localhost:5173"]
    allow_credentials: bool = True
    allow_methods: list[str] = ["*"]
    allow_headers: list[str] = ["*"]
