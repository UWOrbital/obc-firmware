from collections.abc import AsyncGenerator
from contextlib import asynccontextmanager

from fastapi import FastAPI
from fastapi_cache import FastAPICache
from fastapi_cache.backends.inmemory import InMemoryBackend

from gs.backend.data.database.engine import get_db_session, setup_database


@asynccontextmanager
async def lifespan(_: FastAPI) -> AsyncGenerator[None, None]:
    """Lifecycle event for the FastAPI app."""
    # Initialize FastAPI Cache (in memory cache)
    FastAPICache.init(InMemoryBackend())

    # Must all the get_db_session each time when pass it into a separate function.
    # Otherwise, will get transaction is inactive error
    setup_database(get_db_session())
    yield
