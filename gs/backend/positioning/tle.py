from dataclasses import dataclass
from datetime import datetime, timedelta
from typing import Final

import requests

from gs.backend.sun.ephemeris import convert_date_to_jd

TLE_FAILURE_RETURN_MESSAGE: Final[str] = "No GP data found"
AFSPC_MODE_IMPROVED: Final[str] = "i"


@dataclass
class TLEData:
    """Class for TLE data."""

    name: str
    satellite_number: int
    classification: str
    launch_year: int
    launch_number: int
    launch_piece: str
    epoch_year: int  # Last two digits of the year
    epoch_day: float  # Day of the year and fractional portion of the day
    first_derivative_mean_motion: float
    second_derivative_mean_motion: float
    drag_term: float
    ephemeris_type: int
    element_number: int
    inclination: float  # In degrees
    right_ascension: float  # In degrees
    eccentricity: float
    argument_of_perigee: float  # In degrees
    mean_anomaly: float  # In degrees
    mean_motion: float  # In revolutions per day
    revolution_number: int

    def to_tle(self) -> str:
        """
        Convert the TLE data into a valid 2-line TLE string format
        Formatting for this is pulled from this resource: https://www.researchgate.net/figure/Two-line-Element-Set-Format-An-example-TLE-is-shown-with-descriptions-and-units-of-each_fig3_289774073
        """
        first_line = (
            f"1 {self.satellite_number:05d}{self.classification} "
            f"{self.launch_year:02d}{self.launch_number:03d}{self.launch_piece:<3} "
            f"{self.epoch_year:02d}{self.epoch_day:012.8f} "
            f".{self.first_derivative_mean_motion:.8f}".split(".")[1]
            + " "
            + f"{self.second_derivative_mean_motion: 8.5e} "
            f"{self.drag_term: 8.5e} "
            f"{self.ephemeris_type} "
            f"{self.element_number:4d}"
        )
        second_line = (
            f"2 {self.satellite_number:05d} "
            f"{self.inclination:8.4f} "
            f"{self.right_ascension:8.4f} "
            f"{self.eccentricity:.7f}".split(".")[1]
            + " "
            + f"{self.argument_of_perigee:8.4f} "
            f"{self.mean_anomaly:8.4f} "
            f"{self.mean_motion:11.8f}{self.revolution_number:5d}"
        )
        first_line += str(calculate_checksum(first_line))
        second_line += str(calculate_checksum(second_line))
        return f"{first_line}\n{second_line}"

    def format_epoch_to_date(self, epoch_year: int, epoch_day: float) -> str:
        """
        Convert epoch year and day-of-year to a calendar date (YYYY-MM-DD).

        :param epoch_year (int): Last two digits of the epoch year.
        :param epoch_day (float): Day of the year, including fraction.

        :return: Returns formatted date string in YYYY-MM-DD format as a string.
        """
        full_year = 2000 + epoch_year if epoch_year < 57 else 1900 + epoch_year
        base_date = datetime(full_year, 1, 1) + timedelta(days=epoch_day - 1)
        return base_date.strftime("%Y-%m-%d")

    def convert_epoch_values_to_jd(self) -> float:
        """converting epoch values to julian date time"""
        date = self.format_epoch_to_date(self.epoch_year, self.epoch_day)
        # if(is_valid_date(date)):
        return convert_date_to_jd(date)


def calculate_checksum(line: str) -> int:
    """
    Calculate the checksum for a line of TLE data.
    
    :param line(str): The single line of TLE data to calculate the checksum for
    :return: Returns the checksum integer

    """
    output = 0
    for i in line[:68]:
        if i.isdigit():
            output += int(i)
        if i == "-":
            output += 1
    return output % 10


def convert_decimal_point_assumed(value: str) -> float:
    """
    Convert a string to a float, assuming a decimal point before the last digit.

    :param value(str): the string to convert
    :returns: data from string converted into float
    """
    return float(f"{value[0]}.{value[1:6]}e{value[6:]}")


def parse_tle_data(tle_data: str) -> TLEData:
    """
    @brief Parse TLE data from the argument into an object. Validates the data.

    @throw ValueError if the TLE data is invalid.

    @param tle_data: The TLE data to parse.
    @return A TLEData object.
    """
    # Check for failure
    if tle_data == TLE_FAILURE_RETURN_MESSAGE:
        raise ValueError("No GP data found")

    lines = tle_data.splitlines()
    name, line1, line2 = lines

    # Validate the lines
    if (
        len(lines) != 3
        or not line1.startswith("1")
        or not line2.startswith("2")
        or len(line1) != 69
        or len(line2) != 69
    ):
        raise ValueError("Invalid TLE data")

    # Get the checksums
    checksum_1 = int(line1[68])
    checksum_2 = int(line2[68])

    # Parse the data
    output = TLEData(
        name=name.strip(),
        satellite_number=int(line1[2:7]),
        classification=line1[7],
        launch_year=int(line1[9:11]),
        launch_number=int(line1[11:14]),
        launch_piece=line1[14:17].strip(),
        epoch_year=int(line1[18:20]),
        epoch_day=float(line1[20:32]),
        first_derivative_mean_motion=float(line1[33:43]),
        second_derivative_mean_motion=convert_decimal_point_assumed(line1[44:52]),
        drag_term=convert_decimal_point_assumed(line1[53:61]),
        ephemeris_type=int(line1[62]),
        element_number=int(line1[64:68]),
        inclination=float(line2[8:16]),
        right_ascension=float(line2[17:25]),
        eccentricity=float(f"0.{line2[26:33]}"),
        argument_of_perigee=float(line2[34:42]),
        mean_anomaly=float(line2[43:51]),
        mean_motion=float(line2[52:63]),
        revolution_number=int(line2[63:68]),
    )

    # Validate the checksums
    if checksum_1 != calculate_checksum(line1):
        raise ValueError("Invalid checksum for line 1")
    if checksum_2 != calculate_checksum(line2):
        raise ValueError("Invalid checksum for line 2")
    return output


def get_tle_data(object_id: int) -> TLEData:
    """
    @brief Get TLE data for a satellite from Celestrak based on the object ID.

    @throw ValueError if the object ID is invalid or the TLE data is invalid.
    """
    url = f"https://celestrak.org/NORAD/elements/gp.php?CATNR={object_id}&FORMAT=tle"
    response = requests.get(url)
    output = response.text
    return parse_tle_data(output)




def id_from_user() -> int:
    """
    Get the object ID from the user.
    This function will keep asking for input until a valid number is entered.
    Used for testing purposes.
    """
    while True:
        try:
            object_id = int(input("Enter the satellite ID: "))
            return object_id
        except ValueError:
            print("Invalid input. Please enter a valid number.")


def main() -> None:
    """Example usage of the get_tle_data function."""
    object_id = id_from_user()
    try:
        tle_data = get_tle_data(object_id)
        print(tle_data)
    except ValueError as e:
        print(e)


if __name__ == "__main__":
    main()
