#include "mcp4562.h"
#include "obc_i2c_io.h"
#include "obc_logging.h"

#include <stddef.h>

obc_error_code_t mcp4562WriteWiper(const mcp4562_t *device, uint16_t wiperValue) {
  if (device == NULL || wiperValue > MCP4562_MAX_WIPER_VALUE) return OBC_ERR_CODE_INVALID_ARG;

  // The command byte format is: [AD3 AD2 AD1 AD0] [C1 C0] [D9 D8]
  // We embed the 9th bit (D8) directly into the command byte.
  uint8_t cmdByte = MCP4562_CMD_WRITE_VOLATILE_WIPER | ((wiperValue >> 8) & 0x01U);
  uint8_t dataByte = wiperValue & 0xFFU;

  // We can use the standard register write function, treating the Command Byte as the "Register"
  obc_error_code_t errCode;
  RETURN_IF_ERROR_CODE(i2cWriteReg(device->i2cAddress, cmdByte, &dataByte, 1));

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t mcp4562ReadWiper(const mcp4562_t *device, uint16_t *wiperValue) {
  if (device == NULL || wiperValue == NULL) return OBC_ERR_CODE_INVALID_ARG;

  uint8_t rxData[2] = {0};
  uint8_t cmdByte = MCP4562_CMD_READ_VOLATILE_WIPER;

  // The chip returns 2 bytes. The 9th bit (D8) is in the first byte.
  obc_error_code_t errCode;
  RETURN_IF_ERROR_CODE(i2cReadReg(device->i2cAddress, cmdByte, rxData, 2, pdMS_TO_TICKS(100)));

  *wiperValue = ((uint16_t)(rxData[0] & 0x01U) << 8) | rxData[1];
  return OBC_ERR_CODE_SUCCESS;
}
