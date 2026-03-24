from fastapi import APIRouter, Query

from gs.backend.api.v1.mcc.models.responses import ARORequestsResponse
from gs.backend.data.data_wrappers.wrappers import ARORequestWrapper
from gs.backend.data.enums.aro_requests import ARORequestStatus

aro_requests_router = APIRouter(tags=["MCC", "ARO Requests"])


@aro_requests_router.get("/", response_model=ARORequestsResponse)
async def get_aro_requests(
    count: int = 100,
    offset: int = 0,
    filters: list[ARORequestStatus] | None = Query(default=None),
) -> ARORequestsResponse:
    """
    Retrieve ARO requests.

    :param count: Number of most recent requests to return. If <= 0, all requests are returned.
    :param offset: Starting index in descending request order (for paging).
    :param filters: List of statuses to filter by. If empty, no filtering is applied.
    :return: ARO requests matching the criteria.
    """
    requests = ARORequestWrapper().get_requests(count=count, offset=offset, filters=filters)
    return ARORequestsResponse(data=requests)
