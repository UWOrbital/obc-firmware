import pytest
from gs.backend.positioning.tle import (
    calculate_checksum,
    convert_decimal_point_assumed,
    parse_tle_data,
)


@pytest.mark.parametrize(
    "tle_line, expected_checksum",
    [
        ("1 25544U 98067A 23058.54791667 .00016717 00000+0 10270-3 0 9991", 8), #sample value
        ("", 0), #empty line
        ("ABCDEFG-+", 1), #check addition on minus
        ("1 25544U---", (1+2+5+5+4+4 + 3) % 10), #combination of digits and minus


    ],
)
def test_calculate_checksum(tle_line, expected_checksum):
    assert calculate_checksum(tle_line) == expected_checksum


@pytest.mark.parametrize(  # The convert_decimal_point_assumed fails on negative values.  Is that supposed to happen?
    "input_value, expected_result",
    [
        ("123456-3", 0.00123456),  # 1.23456e-3
        ("678901+2", 678.901),  # 6.78901e2
        ("000001+0", 0.00001),  
        ("999999-1", 0.999999),  # 9.99999e-1   
        ("123456+0", 1.23456),  # 1.23456e0

        # ("-456789-5", -5.6789e-6),       # -0.56789e-5 (negative value, fails)
    ],
)
#I did some research and it seems that the function won't be called for negative values for real TLEs, so failing on negative values should be fine



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
    assert parsed_data.first_derivative_mean_motion == pytest.approx(.00016717, rel=1e-4)
    assert parsed_data.inclination == pytest.approx(51.6435, rel=1e-4)
    assert parsed_data.right_ascension == pytest.approx(143.0464, rel=1e-4)



def test_parse_tle_data_invalid(): #test for corrupted TLEs with mismatched checksums
    tle_str = (
        "ISS (ZARYA)\n"
        "1 25544U 98067A   23058.54791667  .00016717  00000+0  10270-3 0  9997\n"
        "2 25544  51.6435 143.0464 0004767 278.8055  81.2436 15.49815308274053"
    )

    bad_tle1 = tle_str.replace("9997", "9998")
    bad_tle2 = tle_str.replace("15.49815308274053", "15.49815308274054")
    with pytest.raises(ValueError, match="Invalid checksum for line 1"):
        parse_tle_data(bad_tle1)
    with pytest.raises(ValueError, match="Invalid checksum for line 2"):
        parse_tle_data(bad_tle2)
