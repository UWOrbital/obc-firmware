#pragma once

#include <FreeRTOS.h>
#include <os_semphr.h>
#include <stdint.h>

#include "obc_errors.h"

/**
 * @brief Initializes all of the semaphores that will be used by cc1120Send and cc1120Receive
 *
 */
void initAllCc1120TxRxSemaphores(void);

/**
 * @brief Adds the given data to the CC1120 FIFO buffer and transmits
 *
 * @param data - An array of 8-bit data to transmit
 * @param len - The size of the provided array
 * @param txFifoEmptyTimeoutTicks - The amount of time to wait for the txFifoEmptySemaphore to become available
 * @return obc_error_code_t
 */
obc_error_code_t cc1120Send(uint8_t *data, uint32_t len, TickType_t txFifoEmptyTimeoutTicks);

/**
 * @brief Switches the cc1120 to RX mode to continuously receive bytes and send them to the decode task
 * @param syncWordTimeoutTicks - The amount of time to wait for the syncReceivedSemaphore to become available
 * @return obc_error_code_t
 */
obc_error_code_t cc1120ReceiveToDecodeTask(TickType_t syncWordTimeoutTicks);

/**
 * @brief Switches the cc1120 to RX mode to continuously receive bytes and send them to the decode task
 * @param syncWordTimeoutTicks - The amount of time to wait for the syncReceivedSemaphore to become available
 * @param recvBuf: the buffer to store the received bytes
 * @param recvBufLen: length of recvBuf
 * @return obc_error_code_t
 */
obc_error_code_t cc1120Receive(uint8_t *recvBuf, uint16_t recvBufLen, TickType_t syncWordTimeoutTicks);

/**
 * @brief block until the tx fifo is empty without decrementing the semaphore
 *
 * @return obc_error_code_t - whether the tx fifo empty semaphore became available without timing out or not
 */
obc_error_code_t txFifoEmptyCheckBlocking(void);

/**
 * @brief callback function to be used in an ISR when the TX FIFO drops below (CC1120_TX_FIFO_SIZE -
 * TXRX_INTERRUPT_THRESHOLD)
 */
void txFifoReadyCallback(void);

/**
 * @brief callback function to be used in an ISR when the RX FIFO fills above TXRX_INTERRUPT_THRESHOLD
 */
void rxFifoReadyCallback(void);

/**
 * @brief callback function to be used in an ISR when the TX FIFO has become empty
 */
void txFifoEmptyCallback(void);

/**
 * @brief callback function to be used in an ISR when the sync word has been received
 */
void syncEventCallback(void);
