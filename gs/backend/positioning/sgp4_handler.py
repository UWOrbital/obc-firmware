from dataclasses import dataclass
from datetime import datetime

from sgp4.api import SGP4_ERRORS, Satrec, jday
from sgp4.api import WGS72 as GRAVITY_MODEL
from sgp4.api import accelerated
from loguru import logger
from math import radians

from gs.backend.positioning.tle import AFSPC_MODE_IMPROVED, TLEData

#log whether accelerated mode is used
if not accelerated:
    logger.warning(
        "SGP4 accelerated mode is not available. "
        "Falling back to pure Python implementation (slower)."
    )
else:
    logger.info("SGP4 accelerated mode is enabled.")


@dataclass
class SGP4Data:
    """
    Data structure representing the satellite's position and velocity.
    :param position_km: tuple representing the position of the satellite in km
    :param velocity_km_sec: tuple representing velocity of the satellite in km/second
    

    """

    position_km: tuple[float, float, float]
    velocity_km_sec: tuple[float, float, float]


def setup_sgp4(tle: TLEData) -> Satrec:
    """
    Initialize the SGP4 satellite model using TLE data.  Formatting and SGP4 initialization pulled from link below
        https://pypi.org/project/sgp4/

            :warning: currently broken for TLEs where eccentricity is low relative to drag term
    :param tle(TLEData): The TLE string used to initialize Satrec
    :return: Returns initialized Satrec
    """

    
    sat = Satrec()

    
    sat.sgp4init( #causes error when tle.eccentricity is low while tle.drag_term is high)
        GRAVITY_MODEL,  # gravity model
        AFSPC_MODE_IMPROVED,  # propagation mode
        tle.satellite_number,  # satellite number
        tle.convert_epoch_values_to_jd(),  # epoch (Julian date)
        tle.drag_term,  # BSTAR drag term
        #6.2485e-05,
        tle.first_derivative_mean_motion,  # first time derivative of mean motion
        tle.second_derivative_mean_motion,  # second time derivative of mean motion
        tle.eccentricity,  # eccentricity
        radians(tle.argument_of_perigee),  # argument of perigee (radians)
        radians(tle.inclination),  # inclination (radians) 
        radians(tle.mean_anomaly),  # mean anomaly (radians)
        tle.mean_motion * (2 * 3.141592653589793 / 1440.0),  # mean motion (radians/min)
        radians(tle.right_ascension),  # RA of ascending node (radians)
    )
    """  These values where used for debugging to find source of eccentricity value, can use to test if issue is fixed
    sat.sgp4init(
        GRAVITY_MODEL,                # gravity model
        'i',                  # 'a' = old AFSPC mode, 'i' = improved mode
        25544,                # satnum: Satellite number
        25545.69339541,       # epoch: days since 1949 December 31 00:00 UT
        3.8792e-05,           # bstar: drag coefficient (1/earth radii)
        0.0,                  # ndot: first time derivative of mean motion (radians/min^2)
        0.0,                  # nddot: second derivative of mean motion (radians/min^3)
        0.0007417,            # ecco: eccentricity (0..1)
        0.3083420829620822,   # argpo: argument of perigee (radians)
        0.9013560935706996,   # inclo: inclination (radians)
        1.4946964807494398,   # mo: mean anomaly (radians)
        0.06763602333248933,  # no_kozai: mean motion (radians/min)
        3.686137125541276,    # nodeo: R.A. of ascending node (radians)
    )
    """

    """
    print("GRAVITY_MODEL:", GRAVITY_MODEL)
    print("Propagation mode:", AFSPC_MODE_IMPROVED)
    print("Satellite number:", tle.satellite_number)
    print("Epoch (Julian date):", tle.convert_epoch_values_to_jd())
    print("BSTAR drag term:", tle.drag_term)
    print("First derivative of mean motion:", tle.first_derivative_mean_motion)
    print("Second derivative of mean motion:", tle.second_derivative_mean_motion)
    print("Eccentricity:", tle.eccentricity)
    print("Argument of perigee (rad):", radians(tle.argument_of_perigee))
    print("Inclination (rad):", radians(tle.inclination))
    print("Mean anomaly (rad):", radians(tle.mean_anomaly))
    print("Mean motion (rad/min):", tle.mean_motion * (2 * 3.141592653589793 / 1440.0))
    print("RA of ascending node (rad):", radians(tle.right_ascension))
    """

    return sat


def get_sat_position(tle: TLEData, dt: datetime) -> SGP4Data:
    """
    Compute the satellite's position and velocity at a given time.

        :warning: currently broken for TLEs where eccentricity is low relative to drag term
        
    :param tle(TLEData): Two-line element set representing the satellite.
    :param dt(datetime): The timestamp for which to calculate the position.
    :return: Returns location data with custom SGP4Data object
    """

    sat = setup_sgp4(tle)
    jd, fr = jday(dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second)
    error_code, position, velocity = sat.sgp4(jd, fr)

    if 0.0 <= tle.eccentricity and tle.eccentricity <= 1.0:
        print("tle.eccentricity within expected bounds")
        
    error_message = SGP4_ERRORS.get(error_code, None)
    if error_message is not None:
        raise RuntimeError(error_message)

    return SGP4Data(
        position_km=tuple(position),
        velocity_km_sec=tuple(velocity),
    )
