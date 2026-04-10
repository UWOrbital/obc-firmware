from fastapi import APIRouter
from fastapi_cache.decorator import cache

from gs.backend.api.v1.mcc.models.responses import TelemetryResponse
from gs.backend.data.data_wrappers.mcc_wrappers.telemetry_wrapper import (
    get_all_telemetries,
)

telemetry_router = APIRouter(tags=["MCC", "Telemetry"])


@telemetry_router.get("/")
@cache(expire=300)
async def get_all_telemetry() -> TelemetryResponse:
    items = get_all_telemetries()
    return TelemetryResponse(data=items)
