#pragma once

#include <stdint.h>
#include <stdlib.h>

#include "obc_errors.h"

// FM25V20A LIMITS
#define FRAM_MAX_ADDRESS 0x3FFFFU
#define FRAM_ID_LEN 9

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes FRAM sleep status
 */
void initFRAM(void);

/**
 * @brief Read FRAM status register.
 * @param status 1 byte buffer to store read data.
 * @return Error code. OBC_ERR_CODE_SUCCESS if successful.
 */
obc_error_code_t framReadStatusReg(uint8_t *status);

/**
 * @brief Write to FRAM status register.
 * @param status 1 byte value written to FRAM.
 * @return Error code. OBC_ERR_CODE_SUCCESS if successful.
 */
obc_error_code_t framWriteStatusReg(uint8_t status);

/**
 * @brief Read nBytes starting from given address. Note this is used for serial flash compatibility not to read data
 * fast!
 * @param addr Starting address of read.
 * @param buffer Buffer to hold read data.
 * @param nBytes Size of buffer.
 * @return Error code. OBC_ERR_CODE_SUCCESS if successful.
 */
obc_error_code_t framFastRead(uint32_t addr, uint8_t *buffer, size_t nBytes);

/**
 * @brief Read nBytes starting from given address.
 * @param addr Starting address of read.
 * @param buffer Buffer to hold read data.
 * @param nBytes Size of buffer.
 * @return Error code. OBC_ERR_CODE_SUCCESS if successful.
 */
obc_error_code_t framRead(uint32_t addr, uint8_t *buffer, size_t nBytes);

/**
 * @brief Write nBytes starting from given address.
 * @param addr Starting address of write.
 * @param buffer Buffer of data to write.
 * @param nBytes Size of buffer.
 * @return Error code. OBC_ERR_CODE_SUCCESS if successful.
 */
obc_error_code_t framWrite(uint32_t addr, const uint8_t *data, size_t nBytes);

/**
 * @brief Send sleep command to FRAM
 * @return Error code. OBC_ERR_CODE_SUCCESS if successful.
 */
obc_error_code_t framSleep(void);

/**
 * @brief Wake FRAM from sleep
 * @return Error code. OBC_ERR_CODE_SUCCESS if successful.
 */
obc_error_code_t framWakeUp(void);

/**
 * @brief Read FRAM manufacture ID.
 * @param id Buffer to hold read ID.
 * @param nBytes Size of Buffer. ID is max 9 bytes long
 * @return Error code. OBC_ERR_CODE_SUCCESS if successful.
 */
obc_error_code_t framReadID(uint8_t *id, size_t nBytes);

#ifdef __cplusplus
}
#endif
