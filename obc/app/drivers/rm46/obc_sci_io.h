#pragma once

#include <sci.h>
#include <stdint.h>

#include "obc_errors.h"

#ifndef OBC_UART_BAUD_RATE
#define OBC_UART_BAUD_RATE 115200
#endif

/**
 * @brief Initialize mutexes protecting SCI and SCI2.
 *
 */
void initSciMutex(void);

/**
 * @brief Read raw bytes from UART_READ_REG (blocking).
 *
 * @param numBytes Number of bytes to read
 * @param buf Buffer to store the bytes read
 * @param uartMutexTimeoutTicks Number of ticks to wait for the mutex to become available
 * @param blockTimeTicks Number of ticks to wait for async transfer to complete
 * @param sciReg Pointer to SCI register to read from
 * @return obc_error_code_t OBC_ERR_CODE_SUCCESS on success, else an error code
 */
obc_error_code_t sciReadBytes(uint8_t *buf, size_t numBytes, TickType_t uartMutexTimeoutTicks, size_t blockTimeTicks,
                              sciBASE_t *sciReg);

/**
 * @brief Send raw bytes to UART_PRINT_REG (blocking).
 *
 * @param buf Buffer to send
 * @param numBytes Number of bytes to send
 * @param uartMutexTimeoutTicks Number of ticks to wait for the mutex to become available
 * @param sciReg Pointer to SCI register to transmit bytes to
 * @return OBC_ERR_CODE_SUCCESS on success, else an error code
 */
obc_error_code_t sciSendBytes(uint8_t *buf, size_t numBytes, TickType_t uartMutexTimeoutTicks, sciBASE_t *sciReg);
