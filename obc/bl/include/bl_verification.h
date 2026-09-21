#pragma once

#include "obc_errors.h"
#include "obc_metadata.h"
#include <stdint.h>

/**
 * @brief Checks that the app's board ID matches the bootloader's board ID
 *
 * @param boardType Board type of the app
 */
obc_error_code_t verifyBoardType(uint8_t boardType);

/**
 * @brief Calculates the app's CRC and compares to the app metadata
 *
 * @param crcAddr Address of the app's CRC in the metadata
 */
obc_error_code_t verifyCrc(uint32_t crcAddr, uint32_t appStartAddress);

/**
 * @brief Verifies the magic num with the app's metadata
 *
 * @param magicNum Address of the app's magicNum in the metadata
 */
obc_error_code_t verifyMagicNum(uint32_t magicNum);

/**
 * @brief Verify's the app metadata using verifyBoardType, verifyCrc, and verifyMagicNum
 *
 * @param app_metadata Address of the app's metadata
 */
obc_error_code_t verifyMetadata(metadata_t *app_metadata, uint32_t appStartAddress);

/**
 * @brief Verify there are no blank sectors in the app.
 *
 * @param app_metadata Address of the app's metadata
 */
obc_error_code_t blAppBlankCheck(metadata_t *app_metadata, uint32_t appStartAddress);
