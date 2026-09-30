#pragma once

#include <stdbool.h>

#include "cc1120.h"
#include "obc_logging.h"

/**
 * @brief E2E test for SPI read function.
 * Reads through all registers up to the extended register space,
 * and compares values to default values.
 * Burst reads all the values and compares them to the default values.
 * Burst reads FREQ registers in extended address space and compares them to defaults.
 *
 * @return OBC_ERROR_CODE_SUCCESS - If all registers are read correctly and have the right value.
 * @return An error code - If any register does not have the expected value,
 *                 or status byte is invalid.
 */
obc_error_code_t cc1120TestSpiRead(void);
