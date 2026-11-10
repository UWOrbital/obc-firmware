#!/usr/bin/env python3

"""
Firmware Version Generator

Version format:

YYMMMMRR

YY = year
MM = month
RR = monthly release number

Example

January 2023 first release

230100

Revision numbering will later be retrieved automatically from
Git tags or Nexus.
"""

from __future__ import annotations

from datetime import datetime, UTC


def generate_version(revision: int = 0) -> str:
    now = datetime.now(UTC)
    year = now.year % 100
    month = now.month

    return f"{year:02d}{month:02d}{revision:02d}"


if __name__ == "__main__":
    print(generate_version())
