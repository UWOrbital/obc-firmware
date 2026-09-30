#pragma once

#include <stdint.h>

#include "ds3232_mz.h"
#include "obc_errors.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Checks if a given year is a leap year.
 *
 * @param year Year to check
 * @return true if the year is a leap year, false otherwise
 */
bool isLeapYear(uint16_t year);

/**
 * @brief Calculates the day of the year given a month, day, and year.
 *
 * @param month The month (1-12)
 * @param day The day (1-31)
 * @param year The year
 * @return uint16_t
 */
uint16_t calcDayOfYear(uint8_t month, uint8_t day, uint16_t year);

/**
 * @brief Convert a datetime to unix time.
 *
 * @param datetime The datetime to convert.
 * @param unixTime Buffer to store the unix time.
 * @return obc_error_code_t OBC_ERR_CODE_SUCCESS if successful, otherwise an error code.
 */
obc_error_code_t datetimeToUnix(rtc_date_time_t *datetime, uint32_t *unixTime);

/**
 * @brief Convert unix time to a datetime.
 *
 * @param unixTime The unix time to convert.
 * @param datetime Buffer to store the datetime.
 * @return obc_error_code_t OBC_ERR_CODE_SUCCESS if successful, otherwise an error code.
 */
obc_error_code_t unixToDatetime(uint32_t unixTime, rtc_date_time_t *datetime);

#ifdef __cplusplus
}
#endif
