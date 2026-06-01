#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "obc_errors.h"
#include <stdint.h>

#define MCP4562_MAX_WIPER_VALUE 256
#define MCP4562_CMD_WRITE_VOLATILE_WIPER 0x00  // Address 0000, Cmd 00
#define MCP4562_CMD_READ_VOLATILE_WIPER 0x0C   // Address 0000, Cmd 11

/**
 * @struct mcp4562_t
 * @brief MCP4562 Digital Potentiometer Device Handle.
 */
typedef struct {
  uint8_t i2cAddress;
} mcp4562_t;

/**
 * @brief Writes a new resistance value to the Volatile Wiper 0.
 * @param device Pointer to the MCP4562 device handle.
 * @param wiperValue The step value from 0 to 256 (0 = 0 Ohms, 256 = 10k Ohms).
 * @return OBC_ERR_CODE_SUCCESS if successful, error code otherwise.
 */
obc_error_code_t mcp4562WriteWiper(const mcp4562_t *device, uint16_t wiperValue);

/**
 * @brief Reads the current resistance value from the Volatile Wiper 0.
 * @param device Pointer to the MCP4562 device handle.
 * @param wiperValue Pointer to store the read step value (0 to 256).
 * @return OBC_ERR_CODE_SUCCESS if successful, error code otherwise.
 */
obc_error_code_t mcp4562ReadWiper(const mcp4562_t *device, uint16_t *wiperValue);

#ifdef __cplusplus
}
#endif
