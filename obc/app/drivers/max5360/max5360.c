#include "max5360.h"

#include <stdint.h>

#include "obc_errors.h"
#include "obc_i2c_io.h"
#include "obc_logging.h"

#define DAC_ADDRESS 0x60U
#define DAC_VREF_VALUE 2U
#define DAC_STEP_VALUE 64U
#define DAC_SHUT_DOWN_BIT 0x01U
#define DAC_CODE_TRANSFER_BYTES 1U
#define DAC_MAX_VOLTAGE_OUTPUT          \
  ((float)DAC_VREF_VALUE * 0b00111111 / \
   DAC_STEP_VALUE)  // the dac takes a 6 bit number for what to output so the max for that number is 0b00111111

#define I2C_MUTEX_TIMEOUT portMAX_DELAY
#define I2C_TRANSFER_TIMEOUT pdMS_TO_TICKS(100)

/**
 * @brief powers on the max5460 DAC and sets it to output a certain voltage
 *
 * @param analogVoltsOutput the value in volts that will be outputted by the DAC
 *
 * @return obc_error_code_t - whether it was successful or not
 */
obc_error_code_t max5360WriteVoltage(float analogVoltsOutput) {
  if (analogVoltsOutput < 0 || analogVoltsOutput > DAC_MAX_VOLTAGE_OUTPUT) {
    return OBC_ERR_CODE_INVALID_ARG;
  }
  obc_error_code_t errCode;
  // the dac will calculate the output voltage by doing dacCode * DAC_VREF_VALUE / DAC_STEP_VALUE so we need to account
  // for this the 2 LSB should be 0 according to datasheet
  // round the value to the nearest integer before casting
  uint8_t dacCode = ((uint8_t)(analogVoltsOutput * DAC_STEP_VALUE / DAC_VREF_VALUE + 0.5)) << 2;

  RETURN_IF_ERROR_CODE(
      i2cSendTo(DAC_ADDRESS, DAC_CODE_TRANSFER_BYTES, &dacCode, I2C_MUTEX_TIMEOUT, I2C_TRANSFER_TIMEOUT));
  return OBC_ERR_CODE_SUCCESS;
}

/**
 * @brief powers off the max5360 and stops it from outputting anything
 *
 * @return obc_error_code_t whether the power off was successful or not
 */
obc_error_code_t max5360PowerOff(void) {
  obc_error_code_t errCode;
  uint8_t dacRecvBuf;
  // Reading from the DAC will turn it off
  // See datasheet page 10
  RETURN_IF_ERROR_CODE(
      i2cReceiveFrom(DAC_ADDRESS, DAC_CODE_TRANSFER_BYTES, &dacRecvBuf, I2C_MUTEX_TIMEOUT, I2C_TRANSFER_TIMEOUT));
  // DAC should output all ones
  if (dacRecvBuf != UINT8_MAX) {
    return OBC_ERR_CODE_MAX5360_SHUTDOWN_FAILURE;
  }
  return OBC_ERR_CODE_SUCCESS;
}
