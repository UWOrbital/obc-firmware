#include "ds3232_mz.h"
#include "obc_i2c_io.h"
#include "obc_errors.h"
#include "obc_logging.h"
#include "obc_bit_ops.h"

#include <stdint.h>
#include <gio.h>

#define DS3232_I2C_ADDRESS 0x68U

/* DS3232 registers */
#define DS3232_REG_SECONDS 0x00U
#define DS3232_REG_MINUTES 0x01U
#define DS3232_REG_HOURS 0x02U
#define DS3232_REG_DAY 0x03U
#define DS3232_REG_DATE 0X04U
#define DS3232_REG_MONTH 0x05U
#define DS3232_REG_YEAR 0x06U
#define DS3232_REG_ALARM_1_SECONDS 0x07U
#define DS3232_REG_ALARM_1_MINUTES 0x08U
#define DS3232_REG_ALARM_1_HOURS 0x09U
#define DS3232_REG_ALARM_1_DAY_DATE 0x0AU
#define DS3232_REG_ALARM_2_MINUTES 0x0BU
#define DS3232_REG_ALARM_2_HOURS 0x0CU
#define DS3232_REG_ALARM_2_DAY_DATE 0x0DU
#define DS3232_REG_CONTROL 0X0EU
#define DS3232_REG_STATUS 0X0FU
#define DS3232_REG_AGING 0X10U
#define DS3232_REG_TEMP_MSB 0x11U
#define DS3232_REG_TEMP_LSB 0x12U

#define DS3232_TEMP_RESOLUTION 0.25f

#define RTC_RST_GIO_PIN 1UL
#define RTC_RST_GIO_PORT gioPORTA

// GPIO connected to RST configured as open drain
#define RTC_OFF 1UL
#define RTC_ON 0UL

#define MAX_SECONDS 59U
#define MAX_MINUTES 59U
#define MAX_HOURS 23U
#define MIN_DAY 1U
#define MAX_DAY 7U
#define MIN_DATE 1U
#define MAX_DATE 31U
#define MIN_MONTH 1U
#define MAX_MONTH 12U
#define MAX_YEAR 99U

#define I2C_TRANSFER_TIMEOUT pdMS_TO_TICKS(100)

typedef enum {
  ENABLE_MATCH = 0U,
  DISABLE_MATCH = 1U,
} alarm_match_t;

/* STATIC FUNCTIONS */

/**
 * @brief Set the value of the alarm 1 seconds register.
 *
 * @param enable Choose whether seconds should match or not.
 * @param seconds uint8_t Number of seconds to match. (0-59)
 * @return OBC_ERR_CODE_SUCCESS if successful, error code otherwise.
 */
static obc_error_code_t setAlarm1SecondsRTC(alarm_match_t en, uint8_t seconds);

/**
 * @brief Set the value of the alarm 1 minutes register.
 *
 * @param enable Choose whether minutes should match or not.
 * @param minutes uint8_t Number of minutes to match. (0-59)
 * @return OBC_ERR_CODE_SUCCESS if successful, error code otherwise.
 */
static obc_error_code_t setAlarm1MinutesRTC(alarm_match_t en, uint8_t minutes);

/**
 * @brief Set the value of the alarm 1 hours register.
 *
 * @param enable Choose whether hours should match or not.
 * @param hour uint8_t Number of hours to match. (0-23)
 * @return OBC_ERR_CODE_SUCCESS if successful, error code otherwise.
 */
static obc_error_code_t setAlarm1HoursRTC(alarm_match_t en, uint8_t hour);

/**
 * @brief Set the value of the alarm 1 date register.
 *
 * @param enable Choose whether date should match or not.
 * @param date uint8_t Number of date to match. (1-31)
 * @return OBC_ERR_CODE_SUCCESS if successful, error code otherwise.
 */
static obc_error_code_t setAlarm1DateRTC(alarm_match_t en, uint8_t date);

/**
 * @brief Set the value of the alarm 2 minutes register.
 *
 * @param enable Choose whether minutes should match or not.
 * @param minutes uint8_t Number of minutes to match. (0-59)
 * @return OBC_ERR_CODE_SUCCESS if successful, error code otherwise.
 */
static obc_error_code_t setAlarm2MinutesRTC(alarm_match_t en, uint8_t minutes);

/**
 * @brief Set the value of the alarm 2 hours register.
 *
 * @param enable Choose whether hours should match or not.
 * @param hour uint8_t Number of hours to match. (0-23)
 * @return OBC_ERR_CODE_SUCCESS if successful, error code otherwise.
 */
static obc_error_code_t setAlarm2HoursRTC(alarm_match_t en, uint8_t hour);

/**
 * @brief Set the value of the alarm 2 date register.
 *
 * @param enable Choose whether date should match or not.
 * @param date uint8_t Number of date to match. (1-31)
 * @return OBC_ERR_CODE_SUCCESS if successful, error code otherwise.
 */
static obc_error_code_t setAlarm2DateRTC(alarm_match_t en, uint8_t date);

obc_error_code_t rtcInit(rtc_date_time_t *dt) {
  obc_error_code_t errCode;

  if (dt == NULL) return OBC_ERR_CODE_INVALID_ARG;

  turnOnRTC();
  RETURN_IF_ERROR_CODE(setCurrentDateTimeRTC(dt));

  return OBC_ERR_CODE_SUCCESS;
}

void turnOnRTC(void) { gioSetBit(RTC_RST_GIO_PORT, RTC_RST_GIO_PIN, RTC_ON); }

void resetRTC(void) {
  gioSetBit(RTC_RST_GIO_PORT, RTC_RST_GIO_PIN, RTC_OFF);

  while (gioGetBit(RTC_RST_GIO_PORT, RTC_RST_GIO_PIN) != RTC_OFF);

  gioSetBit(RTC_RST_GIO_PORT, RTC_RST_GIO_PIN, RTC_ON);
}

obc_error_code_t getSecondsRTC(uint8_t *seconds) {
  obc_error_code_t errCode;

  if (seconds == NULL) return OBC_ERR_CODE_INVALID_ARG;

  uint8_t data;
  RETURN_IF_ERROR_CODE(i2cReadReg(DS3232_I2C_ADDRESS, DS3232_REG_SECONDS, &data, 1, I2C_TRANSFER_TIMEOUT));

  *seconds = twoDigitDecimalFromBCD(data);

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t getMinutesRTC(uint8_t *minutes) {
  obc_error_code_t errCode;
  if (minutes == NULL) return OBC_ERR_CODE_INVALID_ARG;

  uint8_t data;
  RETURN_IF_ERROR_CODE(i2cReadReg(DS3232_I2C_ADDRESS, DS3232_REG_MINUTES, &data, 1, I2C_TRANSFER_TIMEOUT));

  *minutes = twoDigitDecimalFromBCD(data);

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t getHourRTC(uint8_t *hours) {
  obc_error_code_t errCode;

  if (hours == NULL) return OBC_ERR_CODE_INVALID_ARG;

  uint8_t data;
  RETURN_IF_ERROR_CODE(i2cReadReg(DS3232_I2C_ADDRESS, DS3232_REG_HOURS, &data, 1, I2C_TRANSFER_TIMEOUT));

  *hours = twoDigitDecimalFromBCD(data);

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t getDayRTC(uint8_t *days) {
  obc_error_code_t errCode;

  if (days == NULL) return OBC_ERR_CODE_INVALID_ARG;

  uint8_t data;
  RETURN_IF_ERROR_CODE(i2cReadReg(DS3232_I2C_ADDRESS, DS3232_REG_DAY, &data, 1, I2C_TRANSFER_TIMEOUT));

  *days = data;

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t getDateRTC(uint8_t *date) {
  obc_error_code_t errCode;

  if (date == NULL) return OBC_ERR_CODE_INVALID_ARG;

  uint8_t data;
  RETURN_IF_ERROR_CODE(i2cReadReg(DS3232_I2C_ADDRESS, DS3232_REG_DATE, &data, 1, I2C_TRANSFER_TIMEOUT));

  *date = twoDigitDecimalFromBCD(data);

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t getMonthRTC(uint8_t *month) {
  obc_error_code_t errCode;

  if (month == NULL) return OBC_ERR_CODE_INVALID_ARG;

  uint8_t data;
  RETURN_IF_ERROR_CODE(i2cReadReg(DS3232_I2C_ADDRESS, DS3232_REG_MONTH, &data, 1, I2C_TRANSFER_TIMEOUT));

  *month = twoDigitDecimalFromBCD(data & 0x1F);

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t getYearRTC(uint8_t *year) {
  obc_error_code_t errCode;

  if (year == NULL) return OBC_ERR_CODE_INVALID_ARG;

  uint8_t data;
  RETURN_IF_ERROR_CODE(i2cReadReg(DS3232_I2C_ADDRESS, DS3232_REG_YEAR, &data, 1, I2C_TRANSFER_TIMEOUT));

  *year = twoDigitDecimalFromBCD(data);

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t getCurrentTimeRTC(rtc_time_t *time) {
  obc_error_code_t errCode;

  rtc_time_t tmp = {0};

  RETURN_IF_ERROR_CODE(getMinutesRTC(&tmp.minutes));
  RETURN_IF_ERROR_CODE(getSecondsRTC(&tmp.seconds));
  RETURN_IF_ERROR_CODE(getHourRTC(&tmp.hours));

  *time = tmp;

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t getCurrentDateTimeRTC(rtc_date_time_t *dateTime) {
  obc_error_code_t errCode;

  rtc_date_time_t tmp = {0};

  RETURN_IF_ERROR_CODE(getDateRTC(&tmp.date.date));
  RETURN_IF_ERROR_CODE(getMonthRTC(&tmp.date.month));
  RETURN_IF_ERROR_CODE(getYearRTC(&tmp.date.year));

  RETURN_IF_ERROR_CODE(getHourRTC(&tmp.time.hours));
  RETURN_IF_ERROR_CODE(getMinutesRTC(&tmp.time.minutes));
  RETURN_IF_ERROR_CODE(getSecondsRTC(&tmp.time.seconds));

  *dateTime = tmp;

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t getControlRTC(rtc_control_t *control) {
  obc_error_code_t errCode;

  if (control == NULL) return OBC_ERR_CODE_INVALID_ARG;

  uint8_t data;
  RETURN_IF_ERROR_CODE(i2cReadReg(DS3232_I2C_ADDRESS, DS3232_REG_CONTROL, &data, 1, I2C_TRANSFER_TIMEOUT));

  control->EOSC = (data >> 7) & 1;
  control->BBSQW = (data >> 6) & 1;
  control->CONV = (data >> 5) & 1;
  control->INTCN = (data >> 2) & 1;
  control->A2IE = (data >> 1) & 1;
  control->A1IE = (data & 1);

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t getStatusRTC(rtc_status_t *status) {
  obc_error_code_t errCode;

  if (status == NULL) return OBC_ERR_CODE_INVALID_ARG;

  uint8_t data;
  RETURN_IF_ERROR_CODE(i2cReadReg(DS3232_I2C_ADDRESS, DS3232_REG_STATUS, &data, 1, I2C_TRANSFER_TIMEOUT));

  status->OSF = (data >> 7) & 1;
  status->BB32KHZ = (data >> 6) & 1;
  status->EN32KHZ = (data >> 3) & 1;
  status->BSY = (data >> 2) & 1;
  status->A2F = (data >> 1) & 1;
  status->A1F = (data & 1);

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t getAgingOffsetRTC(int8_t *agingOffset) {
  obc_error_code_t errCode;

  if (agingOffset == NULL) return OBC_ERR_CODE_INVALID_ARG;

  uint8_t data;
  RETURN_IF_ERROR_CODE(i2cReadReg(DS3232_I2C_ADDRESS, DS3232_REG_AGING, &data, 1, I2C_TRANSFER_TIMEOUT));

  *agingOffset = (int8_t)data;

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t getTemperatureRTC(float *temperature) {
  obc_error_code_t errCode;

  if (temperature == NULL) return OBC_ERR_CODE_INVALID_ARG;

  uint8_t dataBuff[2];
  RETURN_IF_ERROR_CODE(i2cReadReg(DS3232_I2C_ADDRESS, DS3232_REG_TEMP_MSB, dataBuff, 2, I2C_TRANSFER_TIMEOUT));

  int16_t val = ((int8_t)dataBuff[0] << 2) | (dataBuff[1] >> 6);

  *temperature = (float)val * DS3232_TEMP_RESOLUTION;

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t setSecondsRTC(uint8_t writeSeconds) {
  obc_error_code_t errCode;

  if (writeSeconds > MAX_SECONDS) return OBC_ERR_CODE_INVALID_ARG;

  uint8_t writeVal = twoDigitDecimalToBCD(writeSeconds);

  RETURN_IF_ERROR_CODE(i2cWriteReg(DS3232_I2C_ADDRESS, DS3232_REG_SECONDS, &writeVal, 1));

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t setMinutesRTC(uint8_t writeMinutes) {
  obc_error_code_t errCode;

  if (writeMinutes > MAX_MINUTES) return OBC_ERR_CODE_INVALID_ARG;

  uint8_t writeVal = twoDigitDecimalToBCD(writeMinutes);

  RETURN_IF_ERROR_CODE(i2cWriteReg(DS3232_I2C_ADDRESS, DS3232_REG_MINUTES, &writeVal, 1));

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t setHourRTC(uint8_t writeHours) {
  obc_error_code_t errCode;

  if (writeHours > MAX_HOURS) return OBC_ERR_CODE_INVALID_ARG;

  uint8_t writeVal = twoDigitDecimalToBCD(writeHours);
  RETURN_IF_ERROR_CODE(i2cWriteReg(DS3232_I2C_ADDRESS, DS3232_REG_HOURS, &writeVal, 1));

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t setDayRTC(uint8_t writeDays) {
  obc_error_code_t errCode;

  if (writeDays < MIN_DAY || writeDays > MAX_DAY) return OBC_ERR_CODE_INVALID_ARG;

  RETURN_IF_ERROR_CODE(i2cWriteReg(DS3232_I2C_ADDRESS, DS3232_REG_DAY, &writeDays, 1));

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t setDateRTC(uint8_t writeDates) {
  obc_error_code_t errCode;

  if (writeDates < MIN_DATE || writeDates > MAX_DATE) return OBC_ERR_CODE_INVALID_ARG;

  uint8_t writeVal = twoDigitDecimalToBCD(writeDates);
  RETURN_IF_ERROR_CODE(i2cWriteReg(DS3232_I2C_ADDRESS, DS3232_REG_DATE, &writeVal, 1));

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t setMonthRTC(uint8_t writeMonths) {
  obc_error_code_t errCode;

  uint8_t monthNum = writeMonths & 0x1F;
  if (monthNum > MAX_MONTH || monthNum < MIN_MONTH) return OBC_ERR_CODE_INVALID_ARG;

  uint8_t writeVal = twoDigitDecimalToBCD(writeMonths);
  RETURN_IF_ERROR_CODE(i2cWriteReg(DS3232_I2C_ADDRESS, DS3232_REG_MONTH, &writeVal, 1));

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t setYearRTC(uint8_t writeYears) {
  obc_error_code_t errCode;

  if (writeYears > MAX_YEAR) return OBC_ERR_CODE_INVALID_ARG;

  uint8_t writeVal = twoDigitDecimalToBCD(writeYears);
  RETURN_IF_ERROR_CODE(i2cWriteReg(DS3232_I2C_ADDRESS, DS3232_REG_YEAR, &writeVal, 1));

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t setCurrentDateTimeRTC(rtc_date_time_t *writeDateTime) {
  obc_error_code_t errCode;

  if (writeDateTime == NULL) return OBC_ERR_CODE_INVALID_ARG;

  RETURN_IF_ERROR_CODE(setSecondsRTC(writeDateTime->time.seconds));
  RETURN_IF_ERROR_CODE(setMinutesRTC(writeDateTime->time.minutes));
  RETURN_IF_ERROR_CODE(setHourRTC(writeDateTime->time.hours));

  RETURN_IF_ERROR_CODE(setDateRTC(writeDateTime->date.date));
  RETURN_IF_ERROR_CODE(setMonthRTC(writeDateTime->date.month));
  RETURN_IF_ERROR_CODE(setYearRTC(writeDateTime->date.year));

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t setControlRTC(rtc_control_t *writeControl) {
  obc_error_code_t errCode;

  if (writeControl == NULL) return OBC_ERR_CODE_INVALID_ARG;

  uint8_t writeVal = (writeControl->EOSC << 7) | (writeControl->BBSQW << 6) | (writeControl->CONV << 5) |
                     (writeControl->INTCN << 2) | (writeControl->A2IE << 1) | (writeControl->A1IE);

  RETURN_IF_ERROR_CODE(i2cWriteReg(DS3232_I2C_ADDRESS, DS3232_REG_CONTROL, &writeVal, 1));

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t setStatusRTC(rtc_status_t *writeStatus) {
  obc_error_code_t errCode;

  if (writeStatus == NULL) return OBC_ERR_CODE_INVALID_ARG;

  uint8_t writeVal = (writeStatus->OSF << 7) | (writeStatus->BB32KHZ << 6) | (writeStatus->EN32KHZ << 3) |
                     (writeStatus->BSY << 2) | (writeStatus->A2F << 1) | (writeStatus->A1F);

  RETURN_IF_ERROR_CODE(i2cWriteReg(DS3232_I2C_ADDRESS, DS3232_REG_STATUS, &writeVal, 1));

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t setAgingOffsetRTC(int8_t writeAgingOffset) {
  obc_error_code_t errCode;

  uint8_t writeVal = (writeAgingOffset < 0) ? (1 << 7) : 0;
  writeVal |= (writeAgingOffset < 0) ? -writeAgingOffset : writeAgingOffset;

  RETURN_IF_ERROR_CODE(i2cWriteReg(DS3232_I2C_ADDRESS, DS3232_REG_AGING, &writeVal, 1));

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t setAlarm1SecondsRTC(alarm_match_t en, uint8_t seconds) {
  obc_error_code_t errCode;

  if (seconds > MAX_SECONDS) return OBC_ERR_CODE_INVALID_ARG;

  uint8_t writeVal = (en << 7) | twoDigitDecimalToBCD(seconds);
  RETURN_IF_ERROR_CODE(i2cWriteReg(DS3232_I2C_ADDRESS, DS3232_REG_ALARM_1_SECONDS, &writeVal, 1));

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t setAlarm1MinutesRTC(alarm_match_t en, uint8_t minutes) {
  obc_error_code_t errCode;

  if (minutes > MAX_MINUTES) return OBC_ERR_CODE_INVALID_ARG;

  uint8_t writeVal = (en << 7) | twoDigitDecimalToBCD(minutes);
  RETURN_IF_ERROR_CODE(i2cWriteReg(DS3232_I2C_ADDRESS, DS3232_REG_ALARM_1_MINUTES, &writeVal, 1));

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t setAlarm1HoursRTC(alarm_match_t en, uint8_t hours) {
  obc_error_code_t errCode;

  if (hours > MAX_HOURS) return OBC_ERR_CODE_INVALID_ARG;

  uint8_t writeVal = (en << 7) | twoDigitDecimalToBCD(hours);
  RETURN_IF_ERROR_CODE(i2cWriteReg(DS3232_I2C_ADDRESS, DS3232_REG_ALARM_1_HOURS, &writeVal, 1));

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t setAlarm1DateRTC(alarm_match_t en, uint8_t date) {
  obc_error_code_t errCode;

  if (date < MIN_DATE || date > MAX_DATE) return OBC_ERR_CODE_INVALID_ARG;

  uint8_t writeVal = (en << 7) | twoDigitDecimalToBCD(date);
  RETURN_IF_ERROR_CODE(i2cWriteReg(DS3232_I2C_ADDRESS, DS3232_REG_ALARM_1_DAY_DATE, &writeVal, 1));

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t setAlarm1RTC(rtc_alarm1_mode_t mode, rtc_alarm_time_t dt) {
  obc_error_code_t errCode;

  switch (mode) {
    case RTC_ALARM1_ONCE_PER_SECOND:
      RETURN_IF_ERROR_CODE(setAlarm1SecondsRTC(DISABLE_MATCH, 0));
      RETURN_IF_ERROR_CODE(setAlarm1MinutesRTC(DISABLE_MATCH, 0));
      RETURN_IF_ERROR_CODE(setAlarm1HoursRTC(DISABLE_MATCH, 0));
      RETURN_IF_ERROR_CODE(setAlarm1DateRTC(DISABLE_MATCH, 1));
      break;
    case RTC_ALARM1_MATCH_SECONDS:
      RETURN_IF_ERROR_CODE(setAlarm1SecondsRTC(ENABLE_MATCH, dt.time.seconds));
      RETURN_IF_ERROR_CODE(setAlarm1MinutesRTC(DISABLE_MATCH, 0));
      RETURN_IF_ERROR_CODE(setAlarm1HoursRTC(DISABLE_MATCH, 0));
      RETURN_IF_ERROR_CODE(setAlarm1DateRTC(DISABLE_MATCH, 1));
      break;
    case RTC_ALARM1_MATCH_MINUTES_SECONDS:
      RETURN_IF_ERROR_CODE(setAlarm1SecondsRTC(ENABLE_MATCH, dt.time.seconds));
      RETURN_IF_ERROR_CODE(setAlarm1MinutesRTC(ENABLE_MATCH, dt.time.minutes));
      RETURN_IF_ERROR_CODE(setAlarm1HoursRTC(DISABLE_MATCH, 0));
      RETURN_IF_ERROR_CODE(setAlarm1DateRTC(DISABLE_MATCH, 1));
      break;
    case RTC_ALARM1_MATCH_HOURS_MINUTES_SECONDS:
      RETURN_IF_ERROR_CODE(setAlarm1SecondsRTC(ENABLE_MATCH, dt.time.seconds));
      RETURN_IF_ERROR_CODE(setAlarm1MinutesRTC(ENABLE_MATCH, dt.time.minutes));
      RETURN_IF_ERROR_CODE(setAlarm1HoursRTC(ENABLE_MATCH, dt.time.hours));
      RETURN_IF_ERROR_CODE(setAlarm1DateRTC(DISABLE_MATCH, 1));
      break;
    case RTC_ALARM1_MATCH_DATE_HOURS_MINUTES_SECONDS:
      RETURN_IF_ERROR_CODE(setAlarm1SecondsRTC(ENABLE_MATCH, dt.time.seconds));
      RETURN_IF_ERROR_CODE(setAlarm1MinutesRTC(ENABLE_MATCH, dt.time.minutes));
      RETURN_IF_ERROR_CODE(setAlarm1HoursRTC(ENABLE_MATCH, dt.time.hours));
      RETURN_IF_ERROR_CODE(setAlarm1DateRTC(ENABLE_MATCH, dt.date));
      break;
    default:
      return OBC_ERR_CODE_INVALID_ARG;
  }

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t setAlarm2MinutesRTC(alarm_match_t en, uint8_t minutes) {
  obc_error_code_t errCode;

  if (minutes > MAX_MINUTES) return OBC_ERR_CODE_INVALID_ARG;

  uint8_t writeVal = (en << 7) | twoDigitDecimalToBCD(minutes);
  RETURN_IF_ERROR_CODE(i2cWriteReg(DS3232_I2C_ADDRESS, DS3232_REG_ALARM_2_MINUTES, &writeVal, 1));

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t setAlarm2HoursRTC(alarm_match_t en, uint8_t hours) {
  obc_error_code_t errCode;

  if (hours > MAX_HOURS) return OBC_ERR_CODE_INVALID_ARG;

  uint8_t writeVal = (en << 7) | twoDigitDecimalToBCD(hours);
  RETURN_IF_ERROR_CODE(i2cWriteReg(DS3232_I2C_ADDRESS, DS3232_REG_ALARM_2_HOURS, &writeVal, 1));

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t setAlarm2DateRTC(alarm_match_t en, uint8_t date) {
  obc_error_code_t errCode;

  if (date < MIN_DATE || date > MAX_DATE) return OBC_ERR_CODE_INVALID_ARG;

  uint8_t writeVal = (en << 7) | twoDigitDecimalToBCD(date);
  RETURN_IF_ERROR_CODE(i2cWriteReg(DS3232_I2C_ADDRESS, DS3232_REG_ALARM_2_DAY_DATE, &writeVal, 1));

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t setAlarm2RTC(rtc_alarm2_mode_t mode, rtc_alarm_time_t dt) {
  obc_error_code_t errCode;

  switch (mode) {
    case RTC_ALARM2_ONCE_PER_MINUTE:
      RETURN_IF_ERROR_CODE(setAlarm2MinutesRTC(DISABLE_MATCH, 0));
      RETURN_IF_ERROR_CODE(setAlarm2HoursRTC(DISABLE_MATCH, 0));
      RETURN_IF_ERROR_CODE(setAlarm2DateRTC(DISABLE_MATCH, 1));
      break;
    case RTC_ALARM2_MATCH_MINUTES:
      RETURN_IF_ERROR_CODE(setAlarm2MinutesRTC(ENABLE_MATCH, dt.time.minutes));
      RETURN_IF_ERROR_CODE(setAlarm2HoursRTC(DISABLE_MATCH, 0));
      RETURN_IF_ERROR_CODE(setAlarm2DateRTC(DISABLE_MATCH, 1));
      break;
    case RTC_ALARM2_MATCH_HOURS_MINUTES:
      RETURN_IF_ERROR_CODE(setAlarm2MinutesRTC(ENABLE_MATCH, dt.time.minutes));
      RETURN_IF_ERROR_CODE(setAlarm2HoursRTC(ENABLE_MATCH, dt.time.hours));
      RETURN_IF_ERROR_CODE(setAlarm2DateRTC(DISABLE_MATCH, 1));
      break;
    case RTC_ALARM2_MATCH_DATE_HOURS_MINUTES:
      RETURN_IF_ERROR_CODE(setAlarm2MinutesRTC(ENABLE_MATCH, dt.time.minutes));
      RETURN_IF_ERROR_CODE(setAlarm2HoursRTC(ENABLE_MATCH, dt.time.hours));
      RETURN_IF_ERROR_CODE(setAlarm2DateRTC(ENABLE_MATCH, dt.date));
      break;
    default:
      return OBC_ERR_CODE_INVALID_ARG;
  }

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t clearAlarm1RTC(void) {
  obc_error_code_t errCode;

  // This is a read-modify-write operation.
  // However, alarm flags are only cleared by the alarm handler
  // so this should not be a problem.

  rtc_status_t status = {0};
  RETURN_IF_ERROR_CODE(getStatusRTC(&status));

  status.A1F = 0;

  RETURN_IF_ERROR_CODE(setStatusRTC(&status));

  return OBC_ERR_CODE_SUCCESS;
}
