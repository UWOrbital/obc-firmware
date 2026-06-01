#include "tca6424.h"
#include "obc_i2c_io.h"
#include "obc_logging.h"

#include <stddef.h>

// Helper function for register write access
static obc_error_code_t tca6424aWriteRegister(const tca6424a_t *device, uint8_t addr, uint8_t *data, uint8_t size) {
  obc_error_code_t errCode;
  if (device == NULL || data == NULL) return OBC_ERR_CODE_INVALID_ARG;

  uint8_t slaveReg = (size > 1) ? (addr | (0x01U << 7)) : addr;
  RETURN_IF_ERROR_CODE(i2cWriteReg(device->i2cAddress, slaveReg, data, size));
  return OBC_ERR_CODE_SUCCESS;
}

// Helper function for register read access
static obc_error_code_t tca6424aReadRegister(const tca6424a_t *device, uint8_t addr, uint8_t *data, uint8_t size) {
  obc_error_code_t errCode;
  if (device == NULL || data == NULL) return OBC_ERR_CODE_INVALID_ARG;

  uint8_t slaveReg = (size > 1) ? (addr | (0x01U << 7)) : addr;
  RETURN_IF_ERROR_CODE(i2cReadReg(device->i2cAddress, slaveReg, data, size, TCA6424A_I2C_TRANSFER_TIMEOUT_TICKS));
  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t tca6424aSetIO(const tca6424a_t *device, uint8_t pinLocation, tca6424a_IO_t IO) {
  obc_error_code_t errCode;
  if (device == NULL) return OBC_ERR_CODE_INVALID_ARG;

  // 1. Decode the port and pin combo
  uint8_t pinPort = (pinLocation & 0xF0U) >> 4;
  uint8_t pinIndex = pinLocation & 0x0FU;
  if (pinIndex > MAX_PIN_INDEX || pinPort > MAX_PORT_INDEX) return OBC_ERR_CODE_INVALID_ARG;
  // Use the correct config register with its respective port.
  uint8_t confPortAddr = TCA6424A_CONFIGURATION_PORT_ZERO_ADDR + pinPort;

  // 2. Read current config
  uint8_t regVal = 0;
  RETURN_IF_ERROR_CODE(tca6424aReadRegister(device, confPortAddr, &regVal, 1));

  // 3. Set or clear only the bit we care about
  if (IO == TCA6424A_IO_INPUT) {
    regVal |= (1U << pinIndex);  // Set bit to 1
  } else {
    regVal &= ~(1U << pinIndex);  // Clear bit to 0
  }

  // 4. Write it back to register
  RETURN_IF_ERROR_CODE(tca6424aWriteRegister(device, confPortAddr, &regVal, 1));
  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t tca6424aWritePin(const tca6424a_t *device, uint8_t pinLocation, tca6424a_level_t level) {
  obc_error_code_t errCode;
  if (device == NULL) return OBC_ERR_CODE_INVALID_ARG;

  // 1. Decode the port and pin combo
  uint8_t pinPort = (pinLocation & 0xF0U) >> 4;
  uint8_t pinIndex = pinLocation & 0x0FU;
  if (pinIndex > MAX_PIN_INDEX || pinPort > MAX_PORT_INDEX) return OBC_ERR_CODE_INVALID_ARG;
  // We only care about the output port because we can't write to an input port
  uint8_t outputPortAddr = TCA6424A_OUTPUT_PORT_ZERO_ADDR + pinPort;

  // 2. Read current output state
  uint8_t regVal = 0;
  RETURN_IF_ERROR_CODE(tca6424aReadRegister(device, outputPortAddr, &regVal, 1));

  // 3. Set or clear only the bit we care about
  if (level == TCA6424A_LEVEL_HIGH) {
    regVal |= (1U << pinIndex);  // Set bit to 1
  } else {
    regVal &= ~(1U << pinIndex);  // Clear bit to 0
  }

  // 4. Write it back to register
  RETURN_IF_ERROR_CODE(tca6424aWriteRegister(device, outputPortAddr, &regVal, 1));
  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t tca6424aReadPin(const tca6424a_t *device, uint8_t pinLocation, tca6424a_level_t *level) {
  obc_error_code_t errCode;
  if (device == NULL || level == NULL) return OBC_ERR_CODE_INVALID_ARG;

  // 1. Decode the port and pin combo
  uint8_t pinPort = (pinLocation & 0xF0U) >> 4;
  uint8_t pinIndex = pinLocation & 0x0FU;
  if (pinIndex > MAX_PIN_INDEX || pinPort > MAX_PORT_INDEX) return OBC_ERR_CODE_INVALID_ARG;
  // We only care about the input port because we can't read from an output port.
  uint8_t inputPortAddr = TCA6424A_INPUT_PORT_ZERO_ADDR + pinPort;

  // 2. Read current output state
  uint8_t regVal = 0;
  RETURN_IF_ERROR_CODE(tca6424aReadRegister(device, inputPortAddr, &regVal, 1));

  // 3. Write the output value back to the out variable.
  *level = (regVal & (1U << pinIndex)) ? TCA6424A_LEVEL_HIGH : TCA6424A_LEVEL_LOW;
  return OBC_ERR_CODE_SUCCESS;
}
