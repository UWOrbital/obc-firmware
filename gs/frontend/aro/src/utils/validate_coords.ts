/**
 * @brief Data Validation for Latitude and Longitude
 *
 * Checks if latitude is a float between -90 and 90 degrees inclusive and
 * Checks if longitude is a float between -180 and 180 degrees inclusive
 * @param latitude - north-south position
 * @param longitude - east-west position
 * @return Returns true if both coordinates are valid, otherwise false.
 */

export function ValidateCoords(latitude: number, longitude: number): boolean {
  return (
    latitude >= -90 && latitude <= 90 && longitude >= -180 && longitude <= 180
  );
}
