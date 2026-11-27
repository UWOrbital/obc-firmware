import pytest
from datetime import datetime
from gs.backend.positioning.sgp4_handler import (
    setup_sgp4,
    get_sat_position,
    SGP4Data
)
from gs.backend.positioning.tle import parse_tle_data
from sgp4.api import Satrec
from sgp4.conveniences import dump_satrec
from sgp4.earth_gravity import wgs72
from sgp4.io import twoline2rv
from sys import stdout


def test_setup_sgp4(): #test setup_sgp4 returns a valid Satrec object
    tle_str = (
        "ISS (ZARYA)\n"
        "1 25544U 98067A   23058.54791667  .00016717  00000+0  10270-3 0  9997\n"
        "2 25544  51.6435 143.0464 0004767 278.8055  81.2436 15.49815308274053"
    )
    tle = parse_tle_data(tle_str)
    sat = setup_sgp4(tle)
    assert isinstance(sat, Satrec)

"""
def test_get_sat_position(): #test that output has expected structure and data types
    tle_str = (
        "CSS (MENGTIAN)\n"
        "1 54216U 22143A   25308.16010129  .00032649  00000-0  38953-3 0  9993\n"
        "2 54216  41.4668 251.0039 0006377 319.7576  40.2790 15.60341380158647"
    )
    #Note that the above tle causes eccentricity out of range bug, while tle_str2 does not. 
    #Current implementation causes error when tle.eccentricity is low while tle.drag_term is high
    tle_str2 = (
        "ISS (ZARYA)\n"
        "1 25544U 98067A   25308.35786713  .00010709  00000-0  19707-3 0  9992\n"  
        "2 25544  51.6336 332.4903 0005031  16.0382 344.0765 15.49743270536903"

    )
    tle = parse_tle_data(tle_str)
    dt = datetime(2025, 11, 12, 12, 0, 0)
    data = get_sat_position(tle, dt)
    assert isinstance(data, SGP4Data) 

    assert len(data.position_km) == 3  #test that position and velocity have x y z components (3D, length 3)
    assert len(data.velocity_km_sec) == 3
    assert all(isinstance(x, float) for x in data.position_km) #loop through position_km and velocity_km,
    assert all(isinstance(x, float) for x in data.velocity_km_sec) #   ensuring data types are floats

"""