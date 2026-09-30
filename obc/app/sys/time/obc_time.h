#pragma once

#include <stdint.h>

#include "ds3232_mz.h"
#include "obc_errors.h"

/**
 * @brief Initialize the time module.
 */
obc_error_code_t initTime(void);

/**
 * @brief Get the current unix time.
 *
 * @return uint32_t The current unix time.
 * @warning Do not call in an ISR
 */
uint32_t getCurrentUnixTime(void);

/**
 * @brief Get the current unix time while in ISR.
 *
 * @return uint32_t The current unix time.
 */
uint32_t getCurrentUnixTimeInISR(void);

/**
 * @brief Get the current data time.
 *
 * @return rtc_date_time_t The current data time.
 * @warning Do not call in an ISR
 */
rtc_date_time_t getCurrentDateTime(void);

/**
 * @brief Get the current data time while in ISR.
 *
 * @return rtc_date_time_t The current data time.
 */
rtc_date_time_t getCurrentDateTimeinISR(void);

/**
 * @brief Increment the current unix time by 1 second.
 * @warning This function should not be called from an ISR.
 */
void incrementCurrentUnixTime(void);

/**
 * @brief Increment the current date time by 1 second.
 * @warning This function should not be called from an ISR.
 */
void incrementCurrentDateTime(void);

/**
 * @brief Sync the unix time with the RTC.
 *
 * @return obc_error_code_t OBC_ERR_CODE_SUCCESS if successful, otherwise an error code.
 */
obc_error_code_t syncUnixTime(void);
