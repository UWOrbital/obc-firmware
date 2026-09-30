#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "obc_errors.h"

// RTC year is 0-99, so we need to add this offset to get the actual year
#define RTC_YEAR_OFFSET 2000

/**
 * @struct rtc_time_t
 * @brief DS3232 time structure definition
 * @var hours Specifies the RTC hour value. (0-23)
 * @var minutes Specifies the RTC minutes value. (0-59)
 * @var seconds Specifies the RTC seconds value. (0-59)
 */
typedef struct {
  uint8_t hours;
  uint8_t minutes;
  uint8_t seconds;
} rtc_time_t;

/**
 * @struct rtc_date_t
 * @brief DS3232 date structure definition
 * @var date Specifies the RTC date value. (01-31)
 * @var month Specifies the RTC month value. (01-12)
 * @var year Specifies the RTC year value. (00-99)
 */
typedef struct {
  uint8_t date;
  uint8_t month;
  uint8_t year;
} rtc_date_t;

/**
 * @struct rtc_date_time_t
 * @brief DS3232 date plus time structure definition
 * @var date Specifies the RTC date
 * @var time Specifies the RTC time
 */
typedef struct {
  rtc_date_t date;
  rtc_time_t time;
} rtc_date_time_t;

/**
 * @struct rtc_control_t
 * @brief DS3232 control register structure definition
 * @var EOSC    Enable oscillator
 * @var BBSQW   Battery-backed square-wave enable
 * @var CONV    Convert temperature
 * @var INTCN   Interrupt control
 * @var A2IE    Alarm 2 interrupt enable
 * @var A1IE    Alarm 1 interrupt enable
 */
typedef struct {
  uint8_t EOSC : 1;
  uint8_t BBSQW : 1;
  uint8_t CONV : 1;
  uint8_t INTCN : 1;
  uint8_t A2IE : 1;
  uint8_t A1IE : 1;
} rtc_control_t;

/**
 * @struct rtc_status_t
 * @brief DS3232 status register structure definition
 * @var OSF     Oscillator stop flag
 * @var BB32KHZ Battery-backed 32kHz output
 * @var EN32KHZ Enabled 32.768kHz output
 * @var BSY     Busy. This bit indicates the device is busy executing temperature conversion function
 * @var A2F     Alarm 2 flag
 * @var A1F     Alarm 1 flag
 */
typedef struct {
  uint8_t OSF : 1;
  uint8_t BB32KHZ : 1;
  uint8_t EN32KHZ : 1;
  uint8_t BSY : 1;
  uint8_t A2F : 1;
  uint8_t A1F : 1;
} rtc_status_t;

/**
 * @struct rtc_alarm_time_t
 * @brief DS3232 alarm time structure definition
 * @var date    Specifies the alarm date value. (1-31)
 * @var hours   Specifies the alarm hours value. (0-23)
 * @var minutes Specifies the alarm minutes value. (0-59)
 * @var seconds Specifies the alarm seconds value. (0-59)
 */
typedef struct {
  uint8_t date;
  rtc_time_t time;
} rtc_alarm_time_t;

typedef enum {
  RTC_ALARM1_ONCE_PER_SECOND = 0x0F,
  RTC_ALARM1_MATCH_SECONDS = 0x0E,
  RTC_ALARM1_MATCH_MINUTES_SECONDS = 0x0C,
  RTC_ALARM1_MATCH_HOURS_MINUTES_SECONDS = 0x08,
  RTC_ALARM1_MATCH_DATE_HOURS_MINUTES_SECONDS = 0x00,
  // ALARM_MATCH_DAY_HOURS_MINUTES_SECONDS = 0x10 // Not supported
} rtc_alarm1_mode_t;

typedef enum {
  RTC_ALARM2_ONCE_PER_MINUTE = 0x07,
  RTC_ALARM2_MATCH_MINUTES = 0x06,
  RTC_ALARM2_MATCH_HOURS_MINUTES = 0x04,
  RTC_ALARM2_MATCH_DATE_HOURS_MINUTES = 0x00,
  // ALARM_MATCH_DAY_HOURS_MINUTES = 0x08 // Not supported
} rtc_alarm2_mode_t;

/**
 * @brief Initializes the RTC.
 * @param dateTime struct used to set date and time variables in RTC.
 * @return OBC_ERR_CODE_SUCCESS if the RTC was initialized, appropriate error code otherwise.
 */
obc_error_code_t rtcInit(rtc_date_time_t* dateTime);

/**
 * @brief Turns on the RTC.
 *
 */
void turnOnRTC(void);

/**
 * @brief Resets the RTC.
 *
 */
void resetRTC(void);

/*-------GET FUNCTIONS---------*/

/**
 * @brief Gets seconds data from RTC.
 *
 * @param seconds uint8_t pointer to store seconds.
 * @return OBC_ERR_CODE_SUCCESS if the seconds was accessed, appropriate error code otherwise.
 */
obc_error_code_t getSecondsRTC(uint8_t* seconds);

/**
 * @brief Gets minutes data from RTC.
 *
 * @param minutes uint8_t pointer to store minutes.
 * @return OBC_ERR_CODE_SUCCESS if the minutes was accessed, appropriate error code otherwise.
 */
obc_error_code_t getMinutesRTC(uint8_t* minutes);

/**
 * @brief Gets hour data from RTC.
 *
 * @param hour uint8_t pointer to store hour.
 * @return OBC_ERR_CODE_SUCCESS if the hour was accessed, appropriate error code otherwise.
 */
obc_error_code_t getHoursRTC(uint8_t* hours);

/**
 * @brief Gets days data from RTC.
 *
 * @param days uint8_t pointer to store days.
 * @return OBC_ERR_CODE_SUCCESS if the days was accessed, appropriate error code otherwise.
 */
obc_error_code_t getDayRTC(uint8_t* day);

/**
 * @brief Gets date data from RTC.
 *
 * @param date uint8_t pointer to store date.
 * @return OBC_ERR_CODE_SUCCESS if the date was accessed, appropriate error code otherwise.
 */
obc_error_code_t getDateRTC(uint8_t* date);

/**
 * @brief Gets month data from RTC.
 *
 * @param month uint8_t pointer to store month.
 * @return OBC_ERR_CODE_SUCCESS if the month was accessed, appropriate error code otherwise.
 */
obc_error_code_t getMonthRTC(uint8_t* month);

/**
 * @brief Gets year data from RTC.
 *
 * @param year uint8_t pointer to store year.
 * @return OBC_ERR_CODE_SUCCESS if the year was accessed, appropriate error code otherwise.
 */
obc_error_code_t getYearRTC(uint8_t* year);

/**
 * @brief Gets date and time data from RTC by populating rtc_date_time_t struct members.
 *
 * @param dateTime struct pointer to store date and time.
 * @return OBC_ERR_CODE_SUCCESS if the date and time were accessed, appropriate error code otherwise.
 */
obc_error_code_t getCurrentDateTimeRTC(rtc_date_time_t* dateTime);

/**
 * @brief Gets current time data from RTC by populating rtc_date_time_t struct members.
 *
 * @param time struct pointer to store current time.
 * @return OBC_ERR_CODE_SUCCESS if the current time was accessed, appropriate error code otherwise.
 */
obc_error_code_t getCurrentTimeRTC(rtc_time_t* time);

/**
 * @brief Gets current alarm configuration from RTC by populating rtc_alarm_time_t struct members.
 *
 * @param alarmTime struct pointer to store alarm time.
 * @return OBC_ERR_CODE_SUCCESS if the alarm time was accessed, appropriate error code otherwise.
 */
obc_error_code_t getAlarmTimeRTC(rtc_alarm_time_t* alarmTime);

/**
 * @brief Gets current control register configuration from RTC by populating rtc_control_t struct members.
 *
 * @param control struct pointer to store control configuration.
 * @return OBC_ERR_CODE_SUCCESS if control register was accessed, appropriate error code otherwise.
 */
obc_error_code_t getControlRTC(rtc_control_t* control);

/**
 * @brief Gets current status register configuration from RTC by populating rtc_status_t struct members.
 *
 * @param status struct pointer to store status configuration.
 * @return OBC_ERR_CODE_SUCCESS if status register was accessed, appropriate error code otherwise.
 */
obc_error_code_t getStatusRTC(rtc_status_t* status);

/**
 * @brief Gets aging offset from RTC.
 *
 * @param agingOffset int8_t pointer to store aging offset.
 * @return OBC_ERR_CODE_SUCCESS if aging register was accessed, appropriate error code otherwise.
 */
obc_error_code_t getAgingOffsetRTC(int8_t* agingOffset);

/**
 * @brief Gets temperature from RTC.
 *
 * @param temperature float pointer to store temperature.
 * @return OBC_ERR_CODE_SUCCESS if temperature register was accessed, appropriate error code otherwise.
 */
obc_error_code_t getTemperatureRTC(float* temperature);

/*-------SET FUNCTIONS---------*/

/**
 * @brief set seconds in RTC.
 *
 * @param writeSeconds uint8_t variable for seconds value to be set in RTC.
 * @return OBC_ERR_CODE_SUCCESS if seconds was set, appropriate error code otherwise.
 */
obc_error_code_t setSecondsRTC(uint8_t writeSeconds);

/**
 * @brief set minutes in RTC.
 *
 * @param writeMinutes uint8_t variable for minutes value to be set in RTC.
 * @return OBC_ERR_CODE_SUCCESS if minutes was set, appropriate error code otherwise.
 */
obc_error_code_t setMinutesRTC(uint8_t writeMinutes);

/**
 * @brief set hours in RTC.
 *
 * @param writeHouruint8_t variable for hours value to be set in RTC.
 * @return OBC_ERR_CODE_SUCCESS if hours was set, appropriate error code otherwise.
 * @warning 24 hour format is used.
 */
obc_error_code_t setHourRTC(uint8_t writeHour);

/**
 * @brief set day in RTC.
 *
 * @param writeDays uint8_t variable for day value to be set in RTC.
 * @return OBC_ERR_CODE_SUCCESS if day was set, appropriate error code otherwise.
 */
obc_error_code_t setDayRTC(uint8_t writeDays);

/**
 * @brief set date in RTC.
 *
 * @param writeDates uint8_t variable for date value to be set in RTC.
 * @return OBC_ERR_CODE_SUCCESS if date was set, appropriate error code otherwise.
 */
obc_error_code_t setDateRTC(uint8_t writeDates);

/**
 * @brief set month in RTC.
 *
 * @param writeMonths uint8_t variable for month value to be set in RTC.
 * @return OBC_ERR_CODE_SUCCESS if month was set, appropriate error code otherwise.
 */
obc_error_code_t setMonthRTC(uint8_t writeMonths);

/**
 * @brief set year in RTC.
 *
 * @param writeYears uint8_t variable for year value to be set in RTC.
 * @return OBC_ERR_CODE_SUCCESS if year was set, appropriate error code otherwise.
 */
obc_error_code_t setYearRTC(uint8_t writeYears);

/**
 * @brief set date and time in RTC.
 *
 * @param writeDateTime struct used to set date and time variables in RTC.
 * @return OBC_ERR_CODE_SUCCESS if date and time was set, appropriate error code otherwise.
 */
obc_error_code_t setCurrentDateTimeRTC(rtc_date_time_t* writeDateTime);

/**
 * @brief set control mode in RTC.
 *
 * @param writeControl struct used to set control variables in RTC.
 * @return OBC_ERR_CODE_SUCCESS if control was set, appropriate error code otherwise.
 */
obc_error_code_t setControlRTC(rtc_control_t* writeControl);

/**
 * @brief set status mode in RTC.
 *
 * @param writeStatus struct used to set status variables in RTC.
 * @return OBC_ERR_CODE_SUCCESS if status was set, appropriate error code otherwise.
 */
obc_error_code_t setStatusRTC(rtc_status_t* writeStatus);

/**
 * @brief set aging offset in RTC.
 *
 * @param writeAgingOffset int8_t variable used to set aging offset in RTC.
 * @return OBC_ERR_CODE_SUCCESS if year was set, appropriate error code otherwise.
 */
obc_error_code_t setAgingOffsetRTC(int8_t writeAgingOffset);

/**
 * @brief Set alarm 1
 *
 * @param mode Alarm mode.
 * @param dt Alarm time.
 * @return OBC_ERR_CODE_SUCCESS if successful, error code otherwise.
 * @note Alarm 1 interrupt must be enabled in the control register.
 */
obc_error_code_t setAlarm1RTC(rtc_alarm1_mode_t mode, rtc_alarm_time_t dt);

/**
 * @brief Set alarm 2
 *
 * @param mode Alarm mode.
 * @param dt Alarm time.
 * @return OBC_ERR_CODE_SUCCESS if successful, error code otherwise.
 * @note Alarm 2 interrupt must be enabled in the control register.
 */
obc_error_code_t setAlarm2RTC(rtc_alarm2_mode_t mode, rtc_alarm_time_t dt);

/**
 * @brief Clear alarm 1 interrupt flag.
 *
 * @return obc_error_code_t OBC_ERR_CODE_SUCCESS if successful, error code otherwise.
 * @warning This should only be called from the alarm handler.
 */
obc_error_code_t clearAlarm1RTC(void);
