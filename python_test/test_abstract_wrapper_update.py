from typing import Optional
from unittest.mock import MagicMock, patch

import pytest
from gs.backend.data.data_wrappers.abstract_wrapper import AbstractWrapper
from gs.backend.data.tables.base_model import BaseSQLModel
from sqlmodel import Field


class TestModel(BaseSQLModel, table=True):
    id: Optional[int] = Field(default=None, primary_key=True)
    name: Optional[str] = Field(default=None, nullable=True)
    age: int
    address: Optional[str] = Field(default=None, nullable=True)


class TestWrapper(AbstractWrapper[TestModel, int]):
    model = TestModel


@pytest.fixture
def mock_session():
    with patch("gs.backend.data.data_wrappers.abstract_wrapper.get_db_session") as mock_get_sess:
        mock_sess_instance = MagicMock()
        mock_get_sess.return_value.__enter__.return_value = mock_sess_instance
        yield mock_sess_instance


@pytest.fixture
def wrapper():
    return TestWrapper()


@pytest.fixture
def obj():
    return TestModel(id=1, name="old", age=25)


def test_update_success(wrapper, mock_session, obj):
    mock_session.get.return_value = obj

    result = wrapper.update(obj.id, {"name": "new", "age": 18})

    mock_session.get.assert_called_once_with(TestModel, 1)

    assert result.name == "new"
    assert result.age == 18
    assert result.id == 1

    mock_session.commit.assert_called_once()


def test_update_not_found(wrapper, mock_session, obj):
    mock_session.get.return_value = None

    with pytest.raises(ValueError):
        wrapper.update(obj.id, {"name": "new"})

    mock_session.commit.assert_not_called()


def test_update_field_not_found(wrapper, mock_session, obj):
    mock_session.get.return_value = obj

    with pytest.raises(ValueError):
        wrapper.update(obj.id, {"test": "test"})

    mock_session.commit.assert_not_called()


def test_update_uneditable_field(wrapper, mock_session, obj):
    mock_session.get.return_value = obj

    with pytest.raises(ValueError):
        wrapper.update(obj.id, {"id": 2})

    mock_session.commit.assert_not_called()


def test_update_type_mismatch(wrapper, mock_session, obj):
    mock_session.get.return_value = obj

    with pytest.raises(TypeError):
        wrapper.update(obj.id, {"age": "five"})

    mock_session.commit.assert_not_called()


def test_update_partial(wrapper, mock_session, obj):
    mock_session.get.return_value = obj

    result = wrapper.update(obj.id, {"name": "new"})

    assert result.name == "new"
    assert result.age == 25

    mock_session.commit.assert_called_once()


def test_update_nullify(wrapper, mock_session, obj):
    mock_session.get.return_value = obj

    result = wrapper.update(obj.id, {"name": None})

    assert not result.name
    assert result.age == 25

    mock_session.commit.assert_called_once()


def test_update_unnullify(wrapper, mock_session, obj):
    mock_session.get.return_value = obj

    result = wrapper.update(obj.id, {"address": "home"})

    assert result.name == "old"
    assert result.age == 25
    assert result.address == "home"

    mock_session.commit.assert_called_once()


def test_update_unnullify_error(wrapper, mock_session, obj):
    mock_session.get.return_value = obj

    with pytest.raises(RuntimeError):
        wrapper.update(obj.id, {"address": 5})

    mock_session.commit.assert_not_called()
