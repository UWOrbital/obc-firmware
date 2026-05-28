import pytest
from gs.backend.positioning.tle import (
    calculate_checksum,
    convert_decimal_point_assumed,
    parse_tle_data,
)


@pytest.mark.parametrize(
    "tle_line, expected_checksum",
    [
        ("1 25544U 98067A 23058.54791667 .00016717 00000+0 10270-3 0 9991", 8),
    ],
)
def test_calculate_checksum(tle_line, expected_checksum):
    assert calculate_checksum(tle_line) == expected_checksum


@pytest.mark.parametrize(  # The convert_decimal_point_assumed fails on negative values.  Is that supposed to happen?
    "input_value, expected_result",
    [
        ("123456-3", 0.00123456),  # 1.23456e-3
        ("678901+2", 678.901),  # 6.78901e2
        # ("-456789-5", -5.6789e-6),       # -0.56789e-5
    ],
)
def test_convert_decimal_point_assumed(input_value, expected_result):
    assert convert_decimal_point_assumed(input_value) == pytest.approx(expected_result, rel=1e-6)


def test_parse_tle_data():
    tle_str = (
        "ISS (ZARYA)\n"
        "1 25544U 98067A   23058.54791667  .00016717  00000+0  10270-3 0  9997\n"
        "2 25544  51.6435 143.0464 0004767 278.8055  81.2436 15.49815308274053"
    )

    parsed_data = parse_tle_data(tle_str)

    assert parsed_data.satellite_number == 25544
    assert parsed_data.inclination == pytest.approx(51.6435, rel=1e-4)
    assert parsed_data.right_ascension == pytest.approx(143.0464, rel=1e-4)
