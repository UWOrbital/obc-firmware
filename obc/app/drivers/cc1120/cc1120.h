#pragma once

#include <stdint.h>
#include "obc_errors.h"

// See chapter 8.5 in the datasheet
#define TXRX_INTERRUPT_THRESHOLD 100U

typedef struct {
  uint8_t addr;
  uint8_t val;
} register_setting_t;

typedef enum {
  CC1120_STATE_SLEEP = 0,
  CC1120_STATE_IDLE,
  CC1120_STATE_XOFF,
  CC1120_STATE_BIAS_SETTLE_MC,
  CC1120_STATE_REG_SETTLE_MC,
  CC1120_STATE_MANCAL,
  CC1120_STATE_BIAS_SETTLE,
  CC1120_STATE_REG_SETTLE,
  CC1120_STATE_STARTCAL,
  CC1120_STATE_BWBOOST,
  CC1120_STATE_FS_LOCK,
  CC1120_STATE_IFADCON,
  CC1120_STATE_RX,
  CC1120_STATE_RX_END,
  CC1120_STATE_RESERVED,
  CC1120_STATE_TXRX_SWITCH,
  CC1120_STATE_RX_FIFO_ERR,
  CC1120_STATE_FSTXON,
  CC1120_STATE_TX,
  CC1120_STATE_TX_END,
  CC1120_STATE_RXTX_SWITCH,
  CC1120_STATE_TX_FIFO_ERR,
  CC1120_STATE_IFADCON_TXRX
} cc1120_state_t;

/**
 * @brief - Reads from consecutive registers from the CC1120.
 *
 * @param addr - The address of the first register to read.
 * @param data - The array to store the read data, or a pointer to a single uint8_t if len=1.
 * @param len - The number of registers to read.
 * @return OBC_ERR_CODE_SUCCESS - If the read was successful.
 * @return An error code - If the register is not valid, or the status byte is invalid.
 */
obc_error_code_t cc1120ReadSpi(uint8_t addr, uint8_t data[], uint8_t len);

/**
 * @brief - Reads from consecutive extended address space registers on the CC1120
 *
 * @param addr - The address of the first register to read.
 * @param data - The array to store the read data, or a pointer to a single uint8_t if len=1.
 * @param len - The number of registers to read.
 * @return OBC_ERR_CODE_SUCCESS - If the read was successful.
 * @return An error code - If the register is not valid, or the status byte is invalid.
 */
obc_error_code_t cc1120ReadExtAddrSpi(uint8_t addr, uint8_t data[], uint8_t len);

/**
 * @brief - Writes to consecutive registers on the CC1120.
 *
 * @param addr - The address of the first register to write to.
 * @param data - The array of data to write to the registers, or a pointer to a single uint8_t if len=1.
 * @param len - The number of registers to write.
 * @return OBC_ERR_CODE_SUCCESS - If the write was successful.
 * @return An error code - If the register is not valid, or the status byte is invalid.
 */
obc_error_code_t cc1120WriteSpi(uint8_t addr, const uint8_t data[], uint8_t len);

/**
 * @brief - Writes to consecutive extended address space registers on the CC1120.
 *
 * @param addr - The address of the first register to write to.
 * @param data - The array of data to write to the registers, or a pointer to a single uint8_t if len=1.
 * @param len - The number of registers to write.
 * @return OBC_ERR_CODE_SUCCESS - If the write was successful.
 * @return An error code - If the register is not valid, or the status byte is invalid.
 */
obc_error_code_t cc1120WriteExtAddrSpi(uint8_t addr, const uint8_t data[], uint8_t len);

/**
 * @brief Calls a strobe command on the CC1120.
 *
 * @param addr - The address of the strobe command.
 * @return OBC_ERR_CODE_SUCCESS - If the strobe command was successful.
 * @return An error code - If the register is not valid, or the status byte is invalid.
 */
obc_error_code_t cc1120StrobeSpi(uint8_t addr);

/**
 * @brief - Reads consecutive registers from the FIFO memory.
 *
 * @param data - The array to store the read data, or a pointer to a single uint8_t if len=1.
 * @param len - The number of registers to read.
 * @return OBC_ERR_CODE_SUCCESS - If the read was successful.
 * @return An error code - If the status byte is invalid.
 */
obc_error_code_t cc1120ReadFifo(uint8_t data[], uint8_t len);

/**
 * @brief - Writes consecutive registers to the FIFO memory.
 *
 * @param data - The array of data to write to the registers, or a pointer to a single uint8_t if len=1.
 * @param len - The number of registers to write.
 * @return OBC_ERR_CODE_SUCCESS - If the write was successful.
 * @return An error code - If the status byte is invalid.
 */
obc_error_code_t cc1120WriteFifo(uint8_t data[], uint8_t len);

/**
 * @brief - Reads consecutive registers directly from the FIFO on the CC1120.
 *
 * @param addr - The address of the first register to read. Range 0x00 - 0xFF.
 * @param data - The array to store the read data, or a pointer to a single uint8_t if len=1.
 * @param len - The number of registers to read.
 * @return OBC_ERR_CODE_SUCCESS - If the read was successful.
 * @return An error code - If the register is not valid, or the status byte is invalid.
 */
obc_error_code_t cc1120ReadFifoDirect(uint8_t addr, uint8_t data[], uint8_t len);

/**
 * @brief - Writes consecutive registers directly to the FIFO on the CC1120.
 *
 * @param addr - The address of the first register to write to. Range 0x00 - 0xFF.
 * @param data - The array of data to write to the registers, or a pointer to a single uint8_t if len=1.
 * @param len - The number of registers to write.
 * @return OBC_ERR_CODE_SUCCESS - If the write was successful.
 * @return An error code - If the register is not valid, or the status byte is invalid.
 */
obc_error_code_t cc1120WriteFifoDirect(uint8_t addr, uint8_t data[], uint8_t len);

/**
 * @brief - Reads the status register on the CC1120 and consecutively sends a byte over SPI.
 *
 * @param data - The data to send to the status register.
 * @return OBC_ERR_CODE_SUCCESS - If the status byte is valid.
 * @return OBC_ERR_CODE_CC1120_INVALID_STATUS_BYTE - If the status byte is invalid.(Base case)
 * @return OBC_ERR_CODE_CC1120_CHIP_CALIBRATING - Chip not ready, calibrating
 * @return OBC_ERR_CODE_CC1120_CHIP_SETTLING - Chip not ready, settling
 * @return OBC_ERR_CODE_CC1120_RX_FIFO_ERROR - Chip not ready, flush the SFRX
 * @return OBC_ERR_CODE_CC1120_TX_FIFO_ERROR - Chip not ready, flush the SFTX
 */
obc_error_code_t cc1120SendByteReceiveStatus(uint8_t data);

/**
 * @brief Gets the number of bytes queued in the TX FIFO
 *
 * @param numBytes - A pointer to an 8-bit integer to store the number of bytes in
 * @return obc_error_code_t - Whether or not the registe read was successful
 */
obc_error_code_t cc1120GetBytesInTxFifo(uint8_t *numBytes);

/**
 * @brief Gets the number of bytes queued in the RX FIFO
 *
 * @param numBytes - A pointer to an 8-bit integer to store the number of bytes in
 * @return obc_error_code_t - Whether or not the register read was successful
 */
obc_error_code_t cc1120GetBytesInRxFifo(uint8_t *numBytes);

/**
 * @brief Gets the state of the CC1120 from the MARCSTATE register
 *
 * @param stateNum - A pointer to an 8-bit integer to store the state in
 * @return obc_error_code_t - Whether or not the register read was successful
 */
obc_error_code_t cc1120GetState(cc1120_state_t *stateNum);

/**
 * @brief Resets CC1120 & initializes transmit mode
 *
 * @return obc_error_code_t - Whether or not the setup was a success
 */
obc_error_code_t cc1120Init(void);

/**
 * @brief Returns a random number generated by CC1120
 * #Check section 3.2 RNDGEN for more information
 *
 * @param randomValue- A pointer to an 8-bit integer to store the random value in
 * @return obc_error_code_t - Whether or not the register read was successful
 */
obc_error_code_t cc1120Rng(uint8_t *randomValue);
