from collections.abc import Awaitable, Callable, Sequence
from datetime import datetime
from sys import getsizeof
from time import perf_counter
from uuid import uuid4

from fastapi import FastAPI, Request, Response
from loguru import logger
from starlette.middleware.base import BaseHTTPMiddleware
from starlette.status import HTTP_400_BAD_REQUEST, HTTP_500_INTERNAL_SERVER_ERROR


class LoggerMiddleware(BaseHTTPMiddleware):
    """
    Middleware that logs the request and response

    :param excluded_endpoints: endpoints that won't be logged
    :type excluded_endpoints: Sequence[str]
    """

    def __init__(self, app: FastAPI, excluded_endpoints: Sequence[str] = ()) -> None:
        super().__init__(app)
        self.excluded_endpoints = excluded_endpoints

    async def dispatch(self, request: Request, call_next: Callable[[Request], Awaitable[Response]]) -> Response:
        """Logs the request and response"""
        request_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

        request_body = await request.body()
        body_text = request_body.decode()
        if not body_text:
            body_text = "None"

        request_size = getsizeof(request_body)

        # TODO: update this based on userID header name
        request_user_id = request.headers.get("user_id", "Anonymous")
        request_params = dict(request.query_params)
        request_id = str(uuid4())

        logger.info(
            " | ".join(
                [
                    f"REQUEST | Method: {request.method}",
                    f"Request ID: {request_id}",
                    f"URL: {request.url.path}",
                    f"User id: {request_user_id}",
                    f"Request Body: {body_text}",
                    f"Params: {request_params}",
                    f"Time: {request_time}",
                    f"Bytes: {request_size}.",
                ]
            )
        )

        start_time = perf_counter()
        response = await call_next(request)
        process_time = perf_counter() - start_time

        if request.url.path in self.excluded_endpoints:
            return response

        if response.status_code >= HTTP_500_INTERNAL_SERVER_ERROR:
            logger_severity = logger.critical
        elif response.status_code >= HTTP_400_BAD_REQUEST:
            logger_severity = logger.error
        else:
            logger_severity = logger.info

        # not all responses have a body_iterator attribute
        has_body_iterator = hasattr(response, "body_iterator")

        if has_body_iterator:
            response_body_bytes = b"".join([chunk async for chunk in response.body_iterator])  # type: ignore[attr-defined]
            response_size = str(getsizeof(response_body_bytes))
            response_body = response_body_bytes.decode(errors="ignore")
        else:
            response_body = "Error logging response body"
            response_size = "Error logging response size"

        logger_severity(
            " | ".join(
                [
                    f"RESPONSE | Status: {response.status_code}",
                    f"Request ID: {request_id}",
                    f"Response: {response_body}",
                    f"Bytes: {response_size}",
                    f"Seconds Elapsed: {process_time:.3f}.",
                ]
            )
        )

        if has_body_iterator:
            return Response(
                content=response_body,
                status_code=response.status_code,
                headers=dict(response.headers),
                media_type=response.media_type,
            )
        return response
