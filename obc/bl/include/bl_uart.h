#pragma once

#include <stdint.h>

#include "obc_board_config.h"
#include "obc_errors.h"

/**
 * @brief Initialize the UART module
 *
 */
void blUartInit(void);

/**
 * @brief Read a stream of bytes from the UART.
 *
 * @param uartReg UART register to read from
 * @param buf Buffer to read into
 * @param timeout The timeout for the function to wait to receive bytes.
 * @param numBytes Number of bytes to read
 */
obc_error_code_t blUartReadBytes(uint8_t *buf, uint32_t numBytes, uint32_t timeout_ms);

/**
 * @brief Write a stream of bytes to the UART
 *
 * @param uartReg UART register to write to
 * @param buf Buffer to write from
 * @param numBytes Number of bytes to write
 */
void blUartWriteBytes(uint32_t numBytes, uint8_t *buf);
