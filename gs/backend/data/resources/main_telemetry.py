import csv

from gs.backend.data.tables.main_tables import MainTelemetry


def main_telemetry() -> list[MainTelemetry]:
    """
    Returns the list of main telemetry
    """
    with open("gs/backend/data/resources/telemetry.csv", newline="") as csvfile:
        file = csv.reader(csvfile)
        return [
            MainTelemetry(
                id=int(row[0]),
                name=row[1],
                format=row[2],
                data_size=int(row[3]),
                total_size=int(row[4]),
            )
            for row in file
        ]
