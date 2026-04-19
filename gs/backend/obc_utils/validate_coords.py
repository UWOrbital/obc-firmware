# Data Validation for Latitude and Longitude
def validate_coords(latitude: float, longitude: float) -> bool:
    """
    Checks if latitude is a float between -90 and 90 degrees inclusive and
    Checks if longitude is a float between -180 and 180 degrees inclusive

    :param latitude: north-south position
    :param longitude: east-west position
    :return: returns true if both coordinates are valid, otherwise false.
    """
    return -90 <= latitude <= 90 and -180 <= longitude <= 180
