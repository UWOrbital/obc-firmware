#include "cc1120_txrx.h"
#include "obc_logging.h"
#include "cc1120_mcu.h"
#include "cc1120.h"
#include "cc1120_defs.h"
#include "obc_math.h"
#include "obc_board_config.h"

#include "uplink_decoder.h"

#include <FreeRTOS.h>
#include <os_semphr.h>
#include <sys_common.h>
#include <FreeRTOSConfig.h>

#include <stdbool.h>

#define COMMS_MAX_UPLINK_BYTES \
  1000U  // Maximum amount of bytes we will currently be uplinking at a time (should be updated in the future)

#define TIME_FOR_SINGLE_CHUNK_MS \
  (((TXRX_INTERRUPT_THRESHOLD - 1) * 8) / 9600)  // number of bits divided by cc1120 baudrate
#define TX_SEMAPHORE_TIMEOUT pdMS_TO_TICKS(5000)
#define RX_SEMAPHORE_TIMEOUT pdMS_TO_TICKS(100)
#define TX_FIFO_EMPTY_SEMAPHORE_TIMEOUT pdMS_TO_TICKS(5000)
#define SYNC_EVENT_SEMAPHORE_TIMEOUT pdMS_TO_TICKS(30000)

static SemaphoreHandle_t rxSemaphore = NULL;
static StaticSemaphore_t rxSemaphoreBuffer;
static SemaphoreHandle_t txSemaphore = NULL;
static StaticSemaphore_t txSemaphoreBuffer;
static SemaphoreHandle_t txFifoEmptySemaphore = NULL;
static StaticSemaphore_t txFifoEmptySemaphoreBuffer;
static SemaphoreHandle_t syncReceivedSemaphore = NULL;
static StaticSemaphore_t syncReceivedSemaphoreBuffer;

static obc_error_code_t cc1120SendVariablePktMode(uint8_t *data, uint32_t len);

static obc_error_code_t cc1120SendInifinitePktMode(uint8_t *data, uint32_t len);

static obc_error_code_t writeFifoBlocking(uint8_t *data, uint32_t len);

void initAllCc1120TxRxSemaphores(void) {
  if (txSemaphore == NULL) {
    txSemaphore = xSemaphoreCreateBinaryStatic(&txSemaphoreBuffer);
    // Initialize semaphore with count of 1
    xSemaphoreGive(txSemaphore);
  }
  if (rxSemaphore == NULL) {
    rxSemaphore = xSemaphoreCreateBinaryStatic(&rxSemaphoreBuffer);
  }
  if (txFifoEmptySemaphore == NULL) {
    txFifoEmptySemaphore = xSemaphoreCreateBinaryStatic(&txFifoEmptySemaphoreBuffer);
    // Initialize semaphore with count of 1
    xSemaphoreGive(txFifoEmptySemaphore);
  }
  if (syncReceivedSemaphore == NULL) {
    syncReceivedSemaphore = xSemaphoreCreateBinaryStatic(&syncReceivedSemaphoreBuffer);
  }
}

/**
 * @brief Adds the given data to the CC1120 FIFO buffer and transmits
 *
 * @param data - The packet to transmit
 * @param len - The size of the provided packet in bytes
 * @param txFifoEmptyTimeoutTicks - The amount of time to wait for the txFifoEmptySemaphore to become available
 * @return obc_error_code_t
 */
obc_error_code_t cc1120Send(uint8_t *data, uint32_t len, TickType_t txFifoEmptyTimeoutTicks) {
  obc_error_code_t errCode;

  if (txSemaphore == NULL) {
    return OBC_ERR_CODE_INVALID_STATE;
  }

  if (len < 1) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  if (data == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  // wait on the semaphore to make sure tx fifo is empty
  if (xSemaphoreTake(txFifoEmptySemaphore, txFifoEmptyTimeoutTicks) != pdPASS) {
    LOG_ERROR_CODE(OBC_ERR_CODE_SEMAPHORE_TIMEOUT);
    cc1120StrobeSpi(CC1120_STROBE_SFTX);
    xSemaphoreGive(txFifoEmptySemaphore);
    return OBC_ERR_CODE_SEMAPHORE_TIMEOUT;
  }

  // See section 8.1.5
  if (len > CC1120_MAX_PACKET_LEN) {
    RETURN_IF_ERROR_CODE(cc1120SendInifinitePktMode(data, len));
  } else {  // If packet size <= 255, use variable packet length mode
    RETURN_IF_ERROR_CODE(cc1120SendVariablePktMode(data, len));
  }

  return OBC_ERR_CODE_SUCCESS;
}

/**
 * @brief Transmits data with length less than 256
 *
 * @param data - The packet to transmit
 * @param len - The size of the provided packet in bytes
 * @return obc_error_code_t
 */
static obc_error_code_t cc1120SendVariablePktMode(uint8_t *data, uint32_t len) {
  obc_error_code_t errCode;

  // Set to variable packet length mode
  uint8_t spiTransferData = VARIABLE_PACKET_LENGTH_MODE;
  RETURN_IF_ERROR_CODE(cc1120WriteSpi(CC1120_REGS_PKT_CFG0, &spiTransferData, 1));

  // Set max packet size
  spiTransferData = CC1120_MAX_PACKET_LEN;
  RETURN_IF_ERROR_CODE(cc1120WriteSpi(CC1120_REGS_PKT_LEN, &spiTransferData, 1));

  // Write current packet size
  uint8_t variableDataLen = (uint8_t)len;
  RETURN_IF_ERROR_CODE(cc1120WriteFifo(&variableDataLen, 1));  // Write packet size

  // Write TXRX_INTERRUPT_THRESHOLD bytes to TX fifo and activate TX mode
  RETURN_IF_ERROR_CODE(writeFifoBlocking(data, uint32Min(len, (uint32_t)TXRX_INTERRUPT_THRESHOLD)));
  RETURN_IF_ERROR_CODE(cc1120StrobeSpi(CC1120_STROBE_STX));

  // Continously wait for the tx fifo to drop below (128 - TXRX_INTERRUPT_THRESHOLD) bytes before writing
  // TXRX_INTERRUPT_THRESHOLD more bytes
  uint32_t groupsOfBytesWritten;
  for (groupsOfBytesWritten = 1; groupsOfBytesWritten < len / TXRX_INTERRUPT_THRESHOLD; groupsOfBytesWritten++) {
    RETURN_IF_ERROR_CODE(
        writeFifoBlocking(data + groupsOfBytesWritten * TXRX_INTERRUPT_THRESHOLD, TXRX_INTERRUPT_THRESHOLD));
  }

  // If not all bytes have been sent, write the remaining bytes to TX FIFO
  uint32_t bytesSent = (groupsOfBytesWritten - 1) * TXRX_INTERRUPT_THRESHOLD + uint32Min(len, TXRX_INTERRUPT_THRESHOLD);
  if (bytesSent < len) {
    RETURN_IF_ERROR_CODE(writeFifoBlocking(data + groupsOfBytesWritten * TXRX_INTERRUPT_THRESHOLD,
                                           len - groupsOfBytesWritten * TXRX_INTERRUPT_THRESHOLD));
  }
  return OBC_ERR_CODE_SUCCESS;
}

/**
 * @brief Transmits data with length greater than 255
 *
 * @param data - The packet to transmit
 * @param len - The size of the provided packet in bytes
 * @return obc_error_code_t
 */
static obc_error_code_t cc1120SendInifinitePktMode(uint8_t *data, uint32_t len) {
  obc_error_code_t errCode;

  // temporarily set packet size to infinite
  uint8_t spiTransferData = INFINITE_PACKET_LENGTH_MODE;
  RETURN_IF_ERROR_CODE(cc1120WriteSpi(CC1120_REGS_PKT_CFG0, &spiTransferData, 1));

  // Set packet length to mod(len, 256) so that the correct number of bits
  // are sent when fixed packet mode gets reactivated
  spiTransferData = len % (CC1120_MAX_PACKET_LEN + 1);
  RETURN_IF_ERROR_CODE(cc1120WriteSpi(CC1120_REGS_PKT_LEN, &spiTransferData, 1));

  // Write TXRX_INTERRUPT_THRESHOLD bytes to TX fifo and activate TX mode
  RETURN_IF_ERROR_CODE(writeFifoBlocking(data, TXRX_INTERRUPT_THRESHOLD));
  RETURN_IF_ERROR_CODE(cc1120StrobeSpi(CC1120_STROBE_STX));

  // Continously wait for the tx fifo to drop below (128 - TXRX_INTERRUPT_THRESHOLD) bytes before writing
  // TXRX_INTERRUPT_THRESHOLD more bytes need to also make sure that we do not send all of the remaining bytes if len is
  // a multiple of TXRX_INTERRUPT_THRESHOLD to ensure this, we subtract 1 from len in the loop bounds so there is always
  // 1 to TXRX_INTERRUPT_THRESHOLD Bytes left after the loop
  uint32_t groupsOfBytesWritten;
  for (groupsOfBytesWritten = 1; groupsOfBytesWritten < (len - 1) / TXRX_INTERRUPT_THRESHOLD; groupsOfBytesWritten++) {
    RETURN_IF_ERROR_CODE(
        writeFifoBlocking(data + groupsOfBytesWritten * TXRX_INTERRUPT_THRESHOLD, TXRX_INTERRUPT_THRESHOLD));
  }

  // switch back to fixed packet length mode so that transmission is able to properly end once the remaining bytes are
  // sent
  spiTransferData = FIXED_PACKET_LENGTH_MODE;
  RETURN_IF_ERROR_CODE(cc1120WriteSpi(CC1120_REGS_PKT_CFG0, &spiTransferData, 1));

  // write the remaining bytes to TX FIFO
  RETURN_IF_ERROR_CODE(writeFifoBlocking(data + groupsOfBytesWritten * TXRX_INTERRUPT_THRESHOLD,
                                         len - groupsOfBytesWritten * TXRX_INTERRUPT_THRESHOLD));

  return OBC_ERR_CODE_SUCCESS;
}

/**
 * @brief helper function for the cc1120Send functions to take the txSemaphore and then write data to TX FIFO
 *
 * @param data - The packet to transmit
 * @param len - The size of the provided packet in bytes
 * @return obc_error_code_t
 */
static obc_error_code_t writeFifoBlocking(uint8_t *data, uint32_t len) {
  obc_error_code_t errCode;
  if (xSemaphoreTake(txSemaphore, TX_SEMAPHORE_TIMEOUT) != pdPASS) {
    LOG_ERROR_CODE(OBC_ERR_CODE_SEMAPHORE_TIMEOUT);
    return OBC_ERR_CODE_SEMAPHORE_TIMEOUT;
  }
  errCode = cc1120WriteFifo(data, len);
  if (errCode != OBC_ERR_CODE_SUCCESS) {
    LOG_ERROR_CODE(errCode);
    xSemaphoreGive(txSemaphore);
    return errCode;
  }
  return OBC_ERR_CODE_SUCCESS;
}

/**
 * @brief Switches the cc1120 to RX mode to continuously receive bytes and send them to the decode task
 * @param syncWordTimeoutTicks - The amount of time to wait for the syncReceivedSemaphore to become available
 * @return obc_error_code_t
 */
obc_error_code_t cc1120ReceiveToDecodeTask(TickType_t syncWordTimeoutTicks) {
  obc_error_code_t errCode = OBC_ERR_CODE_SUCCESS;
  if (rxSemaphore == NULL) {
    return OBC_ERR_CODE_INVALID_STATE;
  }

  // poll the semaphore to clear whatever value it has (do not block and wait on it)
  xSemaphoreTake(syncReceivedSemaphore, (TickType_t)0);

  // When changing which signals are sent by each gpio, the output will be unstable so interrupts should be disabled
  // see chapter 3.4 in the datasheet for more info
  gioDisableNotification(gioPORTA, CC1120_PKT_SYNC_RXTX_gioPORTA_PIN);

  // switch gpio 2 to be a SYNC_EVENT signal instead of CC1120_PKT_SYNC_RXTX_PIN
  uint8_t spiTransferData = SYNC_EVENT_SIGNAL_NUM;
  RETURN_IF_ERROR_CODE(cc1120WriteSpi(CC1120_REGS_IOCFG2, &spiTransferData, 1));

  // enable interrupts again now that the gpio signals are set
  gioEnableNotification(gioPORTA, (uint32_t)CC1120_SYNC_EVENT_PIN);

  // Temporarily set packet size to infinite
  spiTransferData = INFINITE_PACKET_LENGTH_MODE;
  RETURN_IF_ERROR_CODE(cc1120WriteSpi(CC1120_REGS_PKT_CFG0, &spiTransferData, 1));

  // Switch cc1120 to receive mode
  RETURN_IF_ERROR_CODE(cc1120StrobeSpi(CC1120_STROBE_SRX));

  uint8_t dataBuffer[TXRX_INTERRUPT_THRESHOLD];

  // wait to receive sync word before continuing
  if (xSemaphoreTake(syncReceivedSemaphore, syncWordTimeoutTicks) != pdPASS) {
    LOG_ERROR_CODE(OBC_ERR_CODE_SEMAPHORE_TIMEOUT);
    return OBC_ERR_CODE_SEMAPHORE_TIMEOUT;
  }
  // See chapters 8.1, 8.4, 8.5
  // If we do not stop receiving data, continue looping until COMMS_MAX_UPLINK_BYTES rounded up to the nearest multiple
  // of TXRX_INTERRUPT_THRESHOLD bytes are received
  uint8_t rxFifoReadCycles;  // number of times we receive TXRX_INTERRUPT_THRESHOLD bytes and read them out
  for (rxFifoReadCycles = 0;
       rxFifoReadCycles < (COMMS_MAX_UPLINK_BYTES + TXRX_INTERRUPT_THRESHOLD - 1) / TXRX_INTERRUPT_THRESHOLD;
       ++rxFifoReadCycles) {
    // wait until we have not received more than TXRX_INTERRUPT_THRESHOLD bytes for more than rxTimeout
    // before exiting this loop since that means we are no longer transmitting
    if (xSemaphoreTake(rxSemaphore, RX_SEMAPHORE_TIMEOUT) != pdPASS) {
      break;
    }
    RETURN_IF_ERROR_CODE(cc1120ReadFifo(dataBuffer, TXRX_INTERRUPT_THRESHOLD));
    for (uint8_t i = 0; i < TXRX_INTERRUPT_THRESHOLD; ++i) {
      sendToDecodeDataQueue(&dataBuffer[i]);
    }
  }

  uint8_t numBytesInRxFifo;

  // check the number of bytes remaining in the RX FIFO
  RETURN_IF_ERROR_CODE(cc1120GetBytesInRxFifo(&numBytesInRxFifo));

  if (numBytesInRxFifo != 0) {
    // if there are still bytes in the RX FIFO, read them out
    RETURN_IF_ERROR_CODE(cc1120ReadFifo(dataBuffer, numBytesInRxFifo));
  }

  // send the bytes read (if any) to decode data queue
  for (uint8_t i = 0; i < numBytesInRxFifo; ++i) {
    sendToDecodeDataQueue(&dataBuffer[i]);
  }

  if (rxFifoReadCycles == (COMMS_MAX_UPLINK_BYTES + TXRX_INTERRUPT_THRESHOLD - 1) / TXRX_INTERRUPT_THRESHOLD) {
    // if recv was terminated by the cubesat due to us receiving the max number of bytes return an error
    return OBC_ERR_CODE_CC1120_RECEIVE_TERMINATED;
  }

  return OBC_ERR_CODE_SUCCESS;
}

/**
 * @brief Switches the cc1120 to RX mode to continuously receive bytes and send them to the decode task
 * @param syncWordTimeoutTicks - The amount of time to wait for the syncReceivedSemaphore to become available
 * @param recvBuf: the buffer to store the received bytes
 * @param recvBufLen: length of recvBuf
 * @return obc_error_code_t
 */
obc_error_code_t cc1120Receive(uint8_t *recvBuf, uint16_t recvBufLen, TickType_t syncWordTimeoutTicks) {
  if (rxSemaphore == NULL) {
    return OBC_ERR_CODE_INVALID_STATE;
  }
  if (recvBuf == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  obc_error_code_t errCode = OBC_ERR_CODE_SUCCESS;

  // poll the semaphore to clear whatever value it has (do not block and wait on it)
  xSemaphoreTake(syncReceivedSemaphore, (TickType_t)0);

  // When changing which signals are sent by each gpio, the output will be unstable so interrupts should be disabled
  // see chapter 3.4 in the datasheet for more info
  gioDisableNotification(gioPORTA, CC1120_PKT_SYNC_RXTX_gioPORTA_PIN);

  // switch gpio 2 to be a SYNC_EVENT signal instead of CC1120_PKT_SYNC_RXTX_PIN
  uint8_t spiTransferData = SYNC_EVENT_SIGNAL_NUM;
  RETURN_IF_ERROR_CODE(cc1120WriteSpi(CC1120_REGS_IOCFG2, &spiTransferData, 1));

  // enable interrupts again now that the gpio signals are set
  gioEnableNotification(gioPORTA, (uint32_t)CC1120_SYNC_EVENT_PIN);

  // Temporarily set packet size to infinite
  spiTransferData = INFINITE_PACKET_LENGTH_MODE;
  RETURN_IF_ERROR_CODE(cc1120WriteSpi(CC1120_REGS_PKT_CFG0, &spiTransferData, 1));

  // Switch cc1120 to receive mode
  RETURN_IF_ERROR_CODE(cc1120StrobeSpi(CC1120_STROBE_SRX));

  // wait to receive sync word before continuing
  if (xSemaphoreTake(syncReceivedSemaphore, syncWordTimeoutTicks) != pdPASS) {
    LOG_ERROR_CODE(OBC_ERR_CODE_SEMAPHORE_TIMEOUT);
    return OBC_ERR_CODE_SEMAPHORE_TIMEOUT;
  }
  // See chapters 8.1, 8.4, 8.5
  // If we do not stop receiving data, continue looping until COMMS_MAX_UPLINK_BYTES rounded up to the nearest multiple
  // of TXRX_INTERRUPT_THRESHOLD bytes are received
  uint8_t rxFifoReadCycles;  // number of times we receive TXRX_INTERRUPT_THRESHOLD bytes and read them out
  for (rxFifoReadCycles = 0; rxFifoReadCycles < recvBufLen / TXRX_INTERRUPT_THRESHOLD; ++rxFifoReadCycles) {
    // wait until we have not received more than TXRX_INTERRUPT_THRESHOLD bytes for more than rxTimeout
    // before exiting this loop since that means we are no longer transmitting
    if (xSemaphoreTake(rxSemaphore, RX_SEMAPHORE_TIMEOUT) != pdPASS) {
      break;
    }
    RETURN_IF_ERROR_CODE(
        cc1120ReadFifo(recvBuf + rxFifoReadCycles * TXRX_INTERRUPT_THRESHOLD, TXRX_INTERRUPT_THRESHOLD));
  }

  if (rxFifoReadCycles == 0) {
    // if we never entered the for loop due to recvLen < TXRX_INTERRUPT_THRESHOLD, then block to allow cc1120 to receive
    // up to 99 bytes before we check the number of bytes
    vTaskDelay(pdMS_TO_TICKS(TIME_FOR_SINGLE_CHUNK_MS));
  }

  uint8_t numBytesInRxFifo = 0;

  // check the number of bytes remaining in the RX FIFO
  RETURN_IF_ERROR_CODE(cc1120GetBytesInRxFifo(&numBytesInRxFifo));

  if (numBytesInRxFifo > recvBufLen - (rxFifoReadCycles * TXRX_INTERRUPT_THRESHOLD)) {
    return OBC_ERR_CODE_BUFF_TOO_SMALL;
  }

  if (numBytesInRxFifo != 0) {
    // if there are still bytes in the RX FIFO, read them out
    RETURN_IF_ERROR_CODE(cc1120ReadFifo(recvBuf + rxFifoReadCycles * TXRX_INTERRUPT_THRESHOLD, numBytesInRxFifo));
  }

  return OBC_ERR_CODE_SUCCESS;
}

/**
 * @brief block until the tx fifo is empty without decrementing the semaphore
 *
 * @return obc_error_code_t - whether the tx fifo empty semaphore became available without timing out or not
 */
obc_error_code_t txFifoEmptyCheckBlocking(void) {
  if (xSemaphoreTake(txFifoEmptySemaphore, TX_FIFO_EMPTY_SEMAPHORE_TIMEOUT) != pdPASS) {
    return OBC_ERR_CODE_SEMAPHORE_TIMEOUT;
  }
  xSemaphoreGive(txFifoEmptySemaphore);
  return OBC_ERR_CODE_SUCCESS;
}

void txFifoReadyCallback(void) {
  BaseType_t xHigherPriorityTaskAwoken = pdFALSE;
  // give semaphore and set xHigherPriorityTaskAwoken to pdTRUE if this unblocks a higher priority task than the current
  // one
  if (xSemaphoreGiveFromISR(txSemaphore, &xHigherPriorityTaskAwoken) != pdPASS) {
    LOG_ERROR_FROM_ISR("TX FIFO below spec.");
  }
  // if xHigherPriorityTaskAwoken == pdTRUE then request a context switch since this means a higher priority task has
  // been unblocked
  portYIELD_FROM_ISR(xHigherPriorityTaskAwoken);
}

void rxFifoReadyCallback(void) {
  BaseType_t xHigherPriorityTaskAwoken = pdFALSE;
  // give semaphore and set xHigherPriorityTaskAwoken to pdTRUE if this unblocks a higher priority task than the current
  // one
  if (xSemaphoreGiveFromISR(rxSemaphore, &xHigherPriorityTaskAwoken) != pdPASS) {
    LOG_ERROR_CODE(OBC_ERR_CODE_QUEUE_FULL);
  }
  // if xHigherPriorityTaskAwoken == pdTRUE then request a context switch since this means a higher priority task has
  // been unblocked
  portYIELD_FROM_ISR(xHigherPriorityTaskAwoken);
}

void txFifoEmptyCallback(void) {
  BaseType_t xHigherPriorityTaskAwoken = pdFALSE;
  // give semaphore and set xHigherPriorityTaskAwoken to pdTRUE if this unblocks a higher priority task than the current
  // one
  if (xSemaphoreGiveFromISR(txFifoEmptySemaphore, &xHigherPriorityTaskAwoken) != pdPASS) {
    LOG_ERROR_CODE(OBC_ERR_CODE_QUEUE_EMPTY);
  }
  // if xHigherPriorityTaskAwoken == pdTRUE then request a context switch since this means a higher priority task has
  // been unblocked
  portYIELD_FROM_ISR(xHigherPriorityTaskAwoken);
}

void syncEventCallback(void) {
  BaseType_t xHigherPriorityTaskAwoken = pdFALSE;
  // give semaphore and set xHigherPriorityTaskAwoken to pdTRUE if this unblocks a higher priority task than the current
  // one
  if (xSemaphoreGiveFromISR(syncReceivedSemaphore, &xHigherPriorityTaskAwoken) != pdPASS) {
    LOG_ERROR_FROM_ISR("Sync word receieved.");
  }
  // if xHigherPriorityTaskAwoken == pdTRUE then request a context switch since this means a higher priority task has
  // been unblocked
  portYIELD_FROM_ISR(xHigherPriorityTaskAwoken);
}
