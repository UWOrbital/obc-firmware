#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "flash_errors.h"

#define FLASH_ATTR_RAMFUNC_SECTION __attribute__((section(".ramFuncs")))

/**
 * @brief Initialize a flash bank
 *
 * @param bankNum The bank number to initialize
 * @return flash_error_code_t Error code
 */
flash_error_code_t flashFapiInitBank(uint32_t bankNum) FLASH_ATTR_RAMFUNC_SECTION;

/**
 * @brief Erase flash sections
 *
 * @param startAddr Start address of the flash to erase
 * @param size Size of the flash to erase
 * @return error_code_t Error code
 * @note Must initialize the bank first
 * @note All sectors with addresses in [startAddr, startAddr + size) will be erased
 */
flash_error_code_t flashFapiBlockErase(uint32_t startAddr, uint32_t size) FLASH_ATTR_RAMFUNC_SECTION;

/**
 * @brief Write data to flash
 *
 * @param flashAddress The address to write to
 * @param dataAddress The address of the data to write
 * @param numBytes The number of bytes to write
 * @return flash_error_code_t Error code
 */
flash_error_code_t flashFapiBlockWrite(uint32_t flashAddress, uint32_t dataAddress,
                                       uint32_t numBytes) FLASH_ATTR_RAMFUNC_SECTION;

/**
 * @brief Checks if a given section has any bytes written
 *
 * @note This will also pick up an ecc bytes written
 *
 * @param startAddr Which address to start the blank check
 * @param size32 Number of 32-bit words to check
 * @return bool True if the area is blank, else false
 */
bool flashFapiBlankCheck(uint32_t startAddr, uint32_t size32) FLASH_ATTR_RAMFUNC_SECTION;

/**
 * @brief Check if an address is a valid start address for the binary
 *
 * @param addr The address to check
 * @param binSize The size of the binary
 * @return true if the address is valid, false otherwise
 */
bool flashIsStartAddrValid(uint32_t addr, uint32_t binSize) FLASH_ATTR_RAMFUNC_SECTION;

/**
 * @brief Wait for the flash state machine to be ready
 *
 */
void flashWaitFsmReady(void) FLASH_ATTR_RAMFUNC_SECTION;

/**
 * @brief Wait for the flash state machine status to be success
 *
 */
void flashWaitFsmStatusSuccess(void) FLASH_ATTR_RAMFUNC_SECTION;

/**
 * @brief Get the sector number of an address
 *
 * @param addr The address to get the sector number of
 * @return uint8_t The sector number
 */
uint8_t flashSectorOfAddr(uint32_t addr) FLASH_ATTR_RAMFUNC_SECTION;

/**
 * @brief Get the start address of a sector
 *
 * @param sector The sector number
 * @return uint32_t The start address of the sector
 */
uint32_t flashSectorStartAddr(uint8_t sector) FLASH_ATTR_RAMFUNC_SECTION;

/**
 * @brief Get the end address of a sector
 *
 * @param sector The sector number
 * @return uint32_t The end address of the sector
 */
uint32_t flashSectorEndAddr(uint8_t sector) FLASH_ATTR_RAMFUNC_SECTION;

/**
 * @brief Get the number of sectors
 *
 * @return uint8_t The number of sectors
 */
uint8_t flashGetNumSectors(void) FLASH_ATTR_RAMFUNC_SECTION;
