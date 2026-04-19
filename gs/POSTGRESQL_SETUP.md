# PostgreSQL Setup

## Table of Contents

- [Installation](#installation)
- [Database Setup](#database-setup)
- [Environment Variable Setup](#environment-variable-setup)
- [Sources](#sources)

**[Back to top](#table-of-contents)**

## Installation

Run the following command dependant on your setup.
If you have Windows, install WSL. See the instructions in the top level [README.md](../../README.md)

### Linux/WSL

```sh
sudo apt update
sudo apt install postgresql-16
```

### MacOS

```sh
brew install postgresql@16
brew services start postgresql
```

**[Back to top](#table-of-contents)**

## Database Setup

Run `sudo -u postgres psql --version` to check that it was installed properly. It should print that major version is 16.

Below are the commands to setup PostgreSQL for the ground station
```sh
sudo -u postgres psql # Connect to the PostgreSQL CLI
CREATE USER username WITH PASSWORD 'password' SUPERUSER;  # Change username to your user on the machine and password to a strong password
# Log out of this session by pressing Ctrl + D
```
Log back in by running `psql` if username you created above matches the current user, you don't need to log in.
Otherwise, you will need to run `psql -U username` and then enter the password created above to proceed.
```sh
CREATE DATABASE gs; # gs database
\c gs; # Connect to the gs database
```

**[Back to top](#table-of-contents)**

## Environment Variable Setup

Find the `gs/backend/config/template.env` and create a copy of it in the same directory with the name `.env`
Inside your newly created `.env` file, put your username and password created above inside the quotes for `GS_DATABASE_USER` and `GS_DATABASE_PASSWORD` respectively.

In the end your `.env` file, should look like this:
```sh
GS_DATABASE_USER="username"      # replace username with your created username
GS_DATABASE_PASSWORD="password"  # replace password with your created password above
GS_DATABASE_LOCATION="localhost" # Change this if using a remote server
GS_DATABASE_PORT="5432"          # Default PostgreSQL port. Change if needed
GS_DATABASE_NAME="gs"            # Name of the database. NOTE: Make sure to create this manually before running the backend of the first time
```

Note: You can choose to remove the first to lines of the `.env` that start with \# as those lines are comments.

Now, you can start the backend as by running `fastapi dev gs/backend/main.py` from the top level directory to run in development mode.

To migrate pre-determined datainto your local database, you can run `python3 gs/backend/migrate.py` from the top level directory.

Alternatively, you can include `callsigns`, `commands`, or `telemetries` as command arguments to migrate those respective datasets individually.

**[Back to top](#table-of-contents)**

## Sources

- [How to Install and Setup PostgreSQL on Ubuntu 20.04 | Step-by-Step](https://www.cherryservers.com/blog/how-to-install-and-setup-postgresql-server-on-ubuntu-20-04)
- [Install and configure PostgreSQL](https://documentation.ubuntu.com/server/how-to/databases/install-postgresql/index.html)
- [How To Install PostgreSQL on Ubuntu 20.04 \[Quickstart\]](https://www.digitalocean.com/community/tutorials/how-to-install-postgresql-on-ubuntu-20-04-quickstart)

**[Back to top](#table-of-contents)**
