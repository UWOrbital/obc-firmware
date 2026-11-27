from dataclasses import dataclass
from datetime import datetime

from sgp4.api import SGP4_ERRORS, Satrec, jday
from sgp4.api import WGS72 as GRAVITY_MODEL

from gs.backend.positioning.tle import AFSPC_MODE_IMPROVED, TLEData


@dataclass
class SGP4Data:
    """Data structure representing the satellite's position and velocity."""

    position_km: tuple[float, float, float]
    velocity_km_sec: tuple[float, float, float]


def setup_spg4(tle: TLEData) -> Satrec:
    """
    Initialize the SGP4 satellite model using TLE data.  Formatting and SGP4 initialization pulled from link below
        https://pypi.org/project/sgp4/
    """
    sat = Satrec()
    sat.sgp4init(
        GRAVITY_MODEL,  # gravity model
        AFSPC_MODE_IMPROVED,  # propagation mode
        tle.satellite_number,  # satellite number
        tle.convert_epoch_values_to_jd(),  # epoch (Julian date)
        tle.drag_term,  # BSTAR drag term
        tle.first_derivative_mean_motion,  # first time derivative of mean motion
        tle.second_derivative_mean_motion,  # second time derivative of mean motion
        tle.eccentricity,  # eccentricity
        tle.argument_of_perigee,  # argument of perigee (radians)
        tle.inclination,  # inclination (radians)
        tle.mean_anomaly,  # mean anomaly (radians)
        tle.mean_motion,  # mean motion (radians/min)
        tle.right_ascension,  # RA of ascending node (radians)
    )
    return sat


def get_sat_position(tle: TLEData, dt: datetime) -> SGP4Data:
    """
    Compute the satellite's position and velocity at a given time.

        Arguments are
        tle(TLEData): Two-line element set representing the satellite.
        dt(datetime): The timestamp for which to calculate the position.

    """
    sat = setup_spg4(tle)
    jd, fr = jday(dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second)
    error_code, position, velocity = sat.sgp4(jd, fr)

    error_message = SGP4_ERRORS.get(error_code, None)
    if error_message is not None:
        raise RuntimeError(error_message)

    return SGP4Data(
        position_km=tuple(position),
        velocity_km_sec=tuple(velocity),
    )
