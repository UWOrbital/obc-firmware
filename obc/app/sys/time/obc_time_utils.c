#include "obc_time_utils.h"

#include <stddef.h>
#include <stdint.h>

#include "ds3232_mz.h"
#include "obc_errors.h"

obc_error_code_t datetimeToUnix(rtc_date_time_t *datetime, uint32_t *unixTime) {
  if (datetime == NULL || unixTime == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  uint32_t tmSec = datetime->time.seconds;
  uint32_t tmMin = datetime->time.minutes;
  uint32_t tmHour = datetime->time.hours;

  uint32_t tmYday =
      (uint32_t)calcDayOfYear(datetime->date.month, datetime->date.date, RTC_YEAR_OFFSET + datetime->date.year);

  // Get year since 1900
  uint32_t tmYear = (RTC_YEAR_OFFSET + datetime->date.year) - 1900;

  // See https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap04.html
  *unixTime = tmSec + tmMin * 60 + tmHour * 3600 + tmYday * 86400 + (tmYear - 70) * 31536000 +
              ((tmYear - 69) / 4) * 86400 - ((tmYear - 1) / 100) * 86400 + ((tmYear + 299) / 400) * 86400;

  return OBC_ERR_CODE_SUCCESS;
}

/*
 * 2000-03-01 (mod 400 year, immediately after Feb 29)
 * This custom epoch makes it easier to handle leap years
 */
#define LEAPOCH (946684800LL + 86400 * (31 + 29))

/* To be a leap year, the year number must be divisible by four
 * except for end-of-century years, which must be divisible by 400. */
#define DAYS_PER_400Y (365 * 400 + 97)  // 400 years have 97 leap days
#define DAYS_PER_100Y (365 * 100 + 24)  // 100 years have 24 leap days
#define DAYS_PER_4Y (365 * 4 + 1)       // 4 years have 1 leap day

#define SECS_PER_MIN 60
#define SECS_PER_HOUR (SECS_PER_MIN * 60)
#define SECS_PER_DAY (SECS_PER_HOUR * 24)
#define MINS_PER_HOUR 60

obc_error_code_t unixToDatetime(uint32_t ts, rtc_date_time_t *dt) {
  /*
      This function is based on the implementation of __secs_to_tm in the
      musl C library. The original implementation can be found here:
      https://git.musl-libc.org/cgit/musl/tree/src/time/__secs_to_tm.c
  */

  if (dt == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  // Since LEAPOCH starts in March, the first month is March
  static const uint8_t daysInMonth[] = {31, 30, 31, 30, 31, 31, 30, 31, 30, 31, 31, 29};

  // Track remainders
  uint32_t remDays, remSecs, remYears;

  uint32_t qcCycles;  // 400-year cycles
  uint32_t cCycles;   // 100-year cycles
  uint32_t qCycles;   // 4-year cycles

  // We won't be handling dates before 2000-03-01
  if (ts < LEAPOCH) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  uint32_t secs = ts - LEAPOCH;

  // Break the number of seconds since LEAPOCH into days and the remaining seconds
  // The remaining seconds determine the time of day (HH:MM:SS)

  uint32_t days = secs / SECS_PER_DAY;

  remSecs = secs % SECS_PER_DAY;

  // Break the number of days into 400-year cycles, 100-year cycles,
  // 4-year cycles, and years

  // Get the number of 400-year cycles since LEAPOCH
  qcCycles = days / DAYS_PER_400Y;
  remDays = days % DAYS_PER_400Y;

  // Get the number of 100-year cycles since end of last 400-year cycle
  cCycles = remDays / DAYS_PER_100Y;

  if (cCycles == 4) {
    cCycles--;
  }

  remDays -= cCycles * DAYS_PER_100Y;

  // Get the number of 4-year cycles since end of last 100-year cycle
  qCycles = remDays / DAYS_PER_4Y;

  if (qCycles == 25) {
    qCycles--;
  }

  remDays -= qCycles * DAYS_PER_4Y;

  // Get the number of years since end of last 4-year cycle
  remYears = remDays / 365;

  if (remYears == 4) {
    remYears--;
  }

  remDays -= remYears * 365;

  // Calculate the years since 2000
  uint8_t years = remYears + 4 * qCycles + 100 * cCycles + 400 * qcCycles;

  // Figure out which month we're in and how many days into the month we are
  uint8_t months = 0;
  while (remDays >= daysInMonth[months]) {
    remDays -= daysInMonth[months];
    months++;
  }

  // Checks before casting uint32_t to uint8_t
  if (remDays >= 31) {
    return OBC_ERR_CODE_UNKNOWN;
  }

  if (remSecs >= SECS_PER_DAY) {
    return OBC_ERR_CODE_UNKNOWN;
  }

  // RTC expects 0-99 so we don't need to offset
  dt->date.year = years;

  // Shift of 2 required since LEAPOCH starts in March
  // and convert to 1-indexed from 0-indexed
  dt->date.month = (months + 2) + 1;

  // Ex: If it's February, months = 11 and dt->date.month = 14
  // because LEAPOCH starts in March. So we need to subtract 12
  // to get the correct month (1-indexed)
  if (dt->date.month >= 12) {
    dt->date.month -= 12;
    dt->date.year++;
  }

  // Get day of month
  dt->date.date = remDays + 1;

  dt->time.hours = remSecs / SECS_PER_HOUR;
  dt->time.minutes = (remSecs / SECS_PER_MIN) % MINS_PER_HOUR;
  dt->time.seconds = remSecs % SECS_PER_MIN;

  return OBC_ERR_CODE_SUCCESS;
}

bool isLeapYear(uint16_t year) { return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0); }

uint16_t calcDayOfYear(uint8_t month, uint8_t day, uint16_t year) {
  // Cumulative days in a year up to the start of each month
  static const uint16_t days[2][12] = {{0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
                                       {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335}};

  // Row 0 is for non-leap years, row 1 is for leap years
  uint8_t leap = (uint8_t)isLeapYear(year);

  // Convert 1-indexed month to 0-indexed
  month--;
  day--;

  return days[leap][month] + day;
}
