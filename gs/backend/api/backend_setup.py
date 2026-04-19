import logging

from fastapi import FastAPI
from loguru import logger

from gs.backend.api.middleware.auth_middleware import AuthMiddleware
from gs.backend.api.middleware.cors_middleware import add_cors_middleware
from gs.backend.api.middleware.logger_middleware import LoggerMiddleware
from gs.backend.api.v1.aro.endpoints.picture_requests import picture_requests_router
from gs.backend.api.v1.aro.endpoints.user import aro_user_router
from gs.backend.api.v1.mcc.endpoints.aro_requests import aro_requests_router
from gs.backend.api.v1.mcc.endpoints.commands import commands_router
from gs.backend.api.v1.mcc.endpoints.main_commands import main_commands_router
from gs.backend.api.v1.mcc.endpoints.telemetry import telemetry_router
from gs.backend.config.config import settings


def setup_routes(app: FastAPI) -> None:
    """Adds the routes to the app"""
    version_1 = "/api/v1"

    # ARO routes
    aro_prefix = f"{version_1}/aro"
    app.include_router(aro_user_router, prefix=f"{aro_prefix}/user")
    app.include_router(picture_requests_router, prefix=f"{aro_prefix}/requests")

    # MCC routes
    mcc_prefix = f"{version_1}/mcc"
    app.include_router(commands_router, prefix=f"{mcc_prefix}/commands")
    app.include_router(telemetry_router, prefix=f"{mcc_prefix}/telemetry")
    app.include_router(aro_requests_router, prefix=f"{mcc_prefix}/requests")
    app.include_router(main_commands_router, prefix=f"{mcc_prefix}/main-commands")


def setup_middlewares(app: FastAPI) -> None:
    """Adds the middlewares to the app"""
    add_cors_middleware(app)  # Cors middleware should be added first
    app.add_middleware(AuthMiddleware)
    app.add_middleware(
        LoggerMiddleware,
        excluded_endpoints=settings.logger.excluded_endpoints,
    )


def setup_logging() -> None:
    """Sets all logs from SQLAlchemy to the custom logger level VERBOSE"""
    verbose_level = 15  # DEBUG=10,  INFO=20
    logger.level("VERBOSE", no=verbose_level, color="<blue>")

    class SQLAlchemyHandler(logging.Handler):
        def emit(self, record: logging.LogRecord) -> None:
            logger.log("VERBOSE", record.getMessage())

    sqlalchemy_logger = logging.getLogger("sqlalchemy.engine")
    sqlalchemy_logger.setLevel(verbose_level)
    sqlalchemy_logger.addHandler(SQLAlchemyHandler())
    sqlalchemy_logger.propagate = False
