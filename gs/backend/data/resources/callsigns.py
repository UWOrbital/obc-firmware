import csv

from gs.backend.data.tables.aro_user_tables import AROUserCallsigns


def callsigns() -> list[AROUserCallsigns]:
    """
    Returns the list of callsigns
    """
    with open("gs/backend/data/resources/callsigns.csv", newline="") as csvfile:
        file = csv.reader(csvfile)
        entries = []
        for row in file:
            entries.append(
                AROUserCallsigns(
                    call_sign=row[0],
                    first_name=row[1] if row[1] else None,
                    last_name=row[2] if row[2] else None,
                    personal_address=row[3] if row[3] else None,
                    personal_city=row[4] if row[4] else None,
                    personal_province=row[5] if row[5] else None,
                    personal_postal_code=row[6] if row[6] else None,
                    qual_level_a=bool(row[7]),
                    qual_level_b=bool(row[8]),
                    qual_level_c=bool(row[9]),
                    qual_level_d=bool(row[10]),
                    qual_level_e=bool(row[11]),
                    club_name=row[12] if row[12] else None,
                    second_club_name=row[13] if row[13] else None,
                    club_address=row[14] if row[14] else None,
                    club_city=row[15] if row[15] else None,
                    club_province=row[16] if row[16] else None,
                    club_postal_code=row[17] if row[17] else None,
                )
            )
        return entries
