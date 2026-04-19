import sys

from gs.backend.data.database.engine import get_db_session
from gs.backend.data.resources.utils import add_callsigns, add_main_commands, add_telemetry

"""
To migrate pre-determined datainto your local database,
you can run `python3 gs/backend/migrate.py` from the top
level directory.

Alternatively, you can include `callsigns`, `commands`, or 'telemetries'
as command arguments to migrate those respective datasets
individually.
"""

if __name__ == "__main__":
    if len(sys.argv) > 2:
        raise ValueError(f"Invalid input. Expected at most 1 argument, received {len(sys.argv)}")
    elif len(sys.argv[1:]) == 0:
        print("Migrating callsign data...")
        add_callsigns(get_db_session())
        print("Migrating main command data...")
        add_main_commands(get_db_session())
        print("Migrating telemetry data...")
        add_telemetry(get_db_session())
    else:
        match sys.argv[1]:
            case "callsigns":
                print("Migrating callsign data...")
                add_callsigns(get_db_session())
            case "commands":
                print("Migrating main command data...")
                add_main_commands(get_db_session())
            case "telemetries":
                print("Migrating telemetry data...")
                add_telemetry(get_db_session())
            case _:
                raise ValueError("Invalid input. Optional arguments include 'callsigns', 'commands', or 'telemetries'.")
