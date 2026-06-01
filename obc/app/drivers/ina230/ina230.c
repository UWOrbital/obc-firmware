#include "ina230.h"
#include "obc_i2c_io.h"
#include "obc_logging.h"

#include <stddef.h>

obc_error_code_t ina230Init(ina230_t *device, uint16_t configRegisterValue) {
  obc_error_code_t errCode;

  if (device == NULL || device->rShuntOhms <= 0.0f || device->maxExpectedAmps <= 0.0f) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  // 1. Calculate the Current and Power LSB for specific INA230
  // Based on datasheet, current_lsb = max expected current / 2^15
  // and power_lsb = 25 * current_lsb
  device->currentLsb = device->maxExpectedAmps / 32768.0f;
  device->powerLsb = INA230_CURRENT_TO_POWER_LSB * device->currentLsb;

  // 2. Calculate the Calibration Register Value
  float calFloat = 0.00512f / (device->currentLsb * device->rShuntOhms);
  uint16_t calValue = (uint16_t)calFloat;

  // 3. Write Calibration Register
  uint8_t writeBuf[2];
  writeBuf[0] = (uint8_t)(calValue >> 8);
  writeBuf[1] = (uint8_t)(calValue & 0xFF);
  RETURN_IF_ERROR_CODE(i2cWriteReg(device->i2cAddress, INA230_REG_CALIBRATION, writeBuf, 2));

  // 4. Write Configuration Register
  writeBuf[0] = (uint8_t)(configRegisterValue >> 8);
  writeBuf[1] = (uint8_t)(configRegisterValue & 0xFF);
  RETURN_IF_ERROR_CODE(i2cWriteReg(device->i2cAddress, INA230_REG_CONFIG, writeBuf, 2));

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t ina230GetBusVoltage(const ina230_t *device, float *voltageOut) {
  obc_error_code_t errCode;

  if (device == NULL || voltageOut == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  uint8_t raw[2] = {0};
  RETURN_IF_ERROR_CODE(i2cReadReg(device->i2cAddress, INA230_REG_BUS_VOLT, raw, 2, I2C_TRANSFER_TIMEOUT_TICKS));

  // Bus voltage is an unsigned 16-bit integer
  uint16_t rawVoltage = (uint16_t)((raw[0] << 8) | raw[1]);

  *voltageOut = rawVoltage * INA230_BUS_VOLTAGE_LSB;
  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t ina230GetShuntVoltage(const ina230_t *device, float *voltageOut) {
  obc_error_code_t errCode;

  if (device == NULL || voltageOut == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  uint8_t raw[2] = {0};
  RETURN_IF_ERROR_CODE(i2cReadReg(device->i2cAddress, INA230_REG_SHUNT_VOLT, raw, 2, I2C_TRANSFER_TIMEOUT_TICKS));

  // Shunt voltage is a signed 16-bit integer
  int16_t rawVoltage = (int16_t)((raw[0] << 8) | raw[1]);

  *voltageOut = rawVoltage * INA230_SHUNT_VOLTAGE_LSB;
  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t ina230GetCurrent(const ina230_t *device, float *currentOut) {
  obc_error_code_t errCode;

  if (device == NULL || currentOut == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  uint8_t raw[2] = {0};
  RETURN_IF_ERROR_CODE(i2cReadReg(device->i2cAddress, INA230_REG_CURRENT, raw, 2, I2C_TRANSFER_TIMEOUT_TICKS));

  // Current is a signed 16-bit integer
  int16_t rawCurrent = (int16_t)((raw[0] << 8) | raw[1]);

  *currentOut = rawCurrent * device->currentLsb;
  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t ina230GetPower(const ina230_t *device, float *powerOut) {
  obc_error_code_t errCode;

  if (device == NULL || powerOut == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  uint8_t raw[2] = {0};
  RETURN_IF_ERROR_CODE(i2cReadReg(device->i2cAddress, INA230_REG_POWER, raw, 2, I2C_TRANSFER_TIMEOUT_TICKS));

  // Power is an unsigned 16-bit integer
  uint16_t rawPower = (uint16_t)((raw[0] << 8) | raw[1]);

  *powerOut = rawPower * device->powerLsb;
  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t ina230SetOvercurrentAlert(const ina230_t *device, float maxCurrentAmps) {
  obc_error_code_t errCode;

  if (device == NULL || maxCurrentAmps <= 0.0f) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  // 1. Convert the current limit into a raw 16-bit register value.
  // The alert is armed in shunt-over-voltage mode (bit 15), so the Alert Limit register is
  // compared against the shunt voltage. Convert amps -> shunt volts -> register counts:
  //   raw = (I * Rshunt) / shunt_voltage_lsb
  uint16_t alertLimitRaw = (uint16_t)((maxCurrentAmps * device->rShuntOhms) / INA230_SHUNT_VOLTAGE_LSB);
  uint8_t limitBuf[2];
  limitBuf[0] = (uint8_t)(alertLimitRaw >> 8);
  limitBuf[1] = (uint8_t)(alertLimitRaw & 0xFF);

  // 2. Write the limit to the Alert Register
  RETURN_IF_ERROR_CODE(i2cWriteReg(device->i2cAddress, INA230_REG_ALERT_LIMIT, limitBuf, 2));

  // 3. Write to the Mask/Enable Register to activate the Shunt Over-Voltage flag
  uint8_t maskBuf[2];
  maskBuf[0] = (uint8_t)(INA230_ALERT_SHUNT_OVER_VOLTAGE >> 8);
  maskBuf[1] = (uint8_t)(INA230_ALERT_SHUNT_OVER_VOLTAGE & 0xFF);
  RETURN_IF_ERROR_CODE(i2cWriteReg(device->i2cAddress, INA230_REG_MASK_ENABLE, maskBuf, 2));

  return OBC_ERR_CODE_SUCCESS;
}

// This function will poll based on the over current alert that has been set
// if that over current alert is detected, then bit 4 of the mask/enable register
// will become 1 - reading from the register clears that 1 (read to clear)
obc_error_code_t ina230CheckAlert(const ina230_t *device, bool *hasAlerted) {
  obc_error_code_t errCode;
  if (device == NULL || hasAlerted == NULL) return OBC_ERR_CODE_INVALID_ARG;

  uint8_t raw[2] = {0};

  // Reading this register automatically clears the alert flag on the chip!
  RETURN_IF_ERROR_CODE(i2cReadReg(device->i2cAddress, INA230_REG_MASK_ENABLE, raw, 2, I2C_TRANSFER_TIMEOUT_TICKS));

  uint16_t maskRegister = (uint16_t)((raw[0] << 8) | raw[1]);

  // Check if Bit 4 is a 1
  *hasAlerted = (maskRegister & INA230_FLAG_ALERT_FUNCTION) ? true : false;

  return OBC_ERR_CODE_SUCCESS;
}
