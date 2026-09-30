#include "obc_time.h"

#include <FreeRTOS.h>
#include <os_atomic.h>
#include <os_task.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "ds3232_mz.h"
#include "obc_assert.h"
#include "obc_errors.h"
#include "obc_logging.h"
#include "obc_time_utils.h"

// Global Unix time
static volatile uint32_t currTime;
static volatile rtc_date_time_t currDataTime = {0};

/**
 * @brief Set the current unix time.
 *
 * @param unixTime The unix time to set.
 * @warning This function should not be called from an ISR.
 */
static void setCurrentUnixTime(uint32_t unixTime);

/**
 * @brief Set the current data time.
 *
 * @param datetime The data time to set.
 * @warning This function should not be called from an ISR.
 */
static void setCurrentDateTime(rtc_date_time_t datetime);

obc_error_code_t initTime(void) {
  obc_error_code_t errCode;

  memset((void *)&currTime, 0, sizeof(currTime));

  // Initialize the RTC

  // TODO: Replace hardcoded datetime with fetch from FRAM
  // or synch with ground station
  rtc_date_time_t rtcDateTime = {0};
  rtcDateTime.date.year = 23;  // 2023
  rtcDateTime.date.month = 1;
  rtcDateTime.date.date = 1;
  rtcDateTime.time.hours = 0;
  rtcDateTime.time.minutes = 0;
  rtcDateTime.time.seconds = 0;

  // Set current date and time
  RETURN_IF_ERROR_CODE(rtcInit(&rtcDateTime));

  // Enable alarm 1 interrupt
  rtc_control_t rtcControl = {0};
  RETURN_IF_ERROR_CODE(getControlRTC(&rtcControl));
  rtcControl.A1IE = 1;
  rtcControl.INTCN = 1;
  RETURN_IF_ERROR_CODE(setControlRTC(&rtcControl));

  // Synch the local Unix time with the RTC
  RETURN_IF_ERROR_CODE(syncUnixTime());

  return OBC_ERR_CODE_SUCCESS;
}

uint32_t getCurrentUnixTime(void) {
  uint32_t time;

  vPortEnterCritical();
  time = currTime;
  vPortExitCritical();

  return time;
}

uint32_t getCurrentUnixTimeInISR(void) {
  uint32_t time;

  UBaseType_t uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();
  time = currTime;
  taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);

  return time;
}

rtc_date_time_t getCurrentDateTime(void) {
  rtc_date_time_t dateTime = {0};
  vPortEnterCritical();
  dateTime = currDataTime;
  vPortExitCritical();

  return dateTime;
}

rtc_date_time_t getCurrentDateTimeinISR(void) {
  rtc_date_time_t dateTime = {0};
  UBaseType_t uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();
  dateTime = currDataTime;
  taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);

  return dateTime;
}

static void setCurrentUnixTime(uint32_t unixTime) {
  vPortEnterCritical();
  currTime = unixTime;
  vPortExitCritical();
}

static void setCurrentDateTime(rtc_date_time_t datetime) {
  vPortEnterCritical();
  currDataTime = datetime;
  vPortExitCritical();
}

void incrementCurrentUnixTime(void) { Atomic_Increment_u32(&currTime); }
void incrementCurrentDateTime(void) {
  vPortEnterCritical();
  currDataTime.time.seconds++;
  vPortExitCritical();
}

obc_error_code_t syncUnixTime(void) {
  obc_error_code_t errCode;

  rtc_date_time_t datetime = {0};

  RETURN_IF_ERROR_CODE(getCurrentDateTimeRTC(&datetime));

  uint32_t unixTime;
  RETURN_IF_ERROR_CODE(datetimeToUnix(&datetime, &unixTime));

  setCurrentUnixTime(unixTime);
  setCurrentDateTime(datetime);

  return OBC_ERR_CODE_SUCCESS;
}
