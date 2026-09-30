#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "bl_errors.h"

#define BL_FLASH_ATTR_RAMFUNC_SECTION __attribute__((section(".ramFuncs")))

/**
 * @brief Initialize a flash bank
 *
 * @param bankNum The bank number to initialize
 * @return bl_error_code_t Error code
 */
bl_error_code_t blFlashFapiInitBank(uint32_t bankNum) BL_FLASH_ATTR_RAMFUNC_SECTION;

/**
 * @brief Erase flash sections
 *
 * @param startAddr Start address of the flash to erase
 * @param size Size of the flash to erase
 * @return bl_error_code_t Error code
 * @note Must initialize the bank first
 * @note All sectors with addresses in [startAddr, startAddr + size) will be erased
 */
bl_error_code_t blFlashFapiBlockErase(uint32_t startAddr, uint32_t size) BL_FLASH_ATTR_RAMFUNC_SECTION;

/**
 * @brief Write data to flash
 *
 * @param flashAddress The address to write to
 * @param dataAddress The address of the data to write
 * @param numBytes The number of bytes to write
 * @return bl_error_code_t Error code
 */
bl_error_code_t blFlashFapiBlockWrite(uint32_t flashAddress, uint32_t dataAddress,
                                      uint32_t numBytes) BL_FLASH_ATTR_RAMFUNC_SECTION;

/**
 * @brief Checks if a given section has any bytes written
 *
 * @note This will also pick up an ecc bytes written
 *
 * @param startAddr Which address to start the blank check
 * @param size32 Number of 32-bit words to check
 * @return bool True if the area is blank, else false
 */
bool blFlashFapiBlankCheck(uint32_t startAddr, uint32_t size32) BL_FLASH_ATTR_RAMFUNC_SECTION;

/**
 * @brief Check if an address is a valid start address for the binary
 *
 * @param addr The address to check
 * @param binSize The size of the binary
 * @return true if the address is valid, false otherwise
 */
bool blFlashIsStartAddrValid(uint32_t addr, uint32_t binSize) BL_FLASH_ATTR_RAMFUNC_SECTION;

/**
 * @brief Wait for the flash state machine to be ready
 *
 */
void blFlashWaitFsmReady(void) BL_FLASH_ATTR_RAMFUNC_SECTION;

/**
 * @brief Wait for the flash state machine status to be success
 *
 */
void blFlashWaitFsmStatusSuccess(void) BL_FLASH_ATTR_RAMFUNC_SECTION;

/**
 * @brief Get the sector number of an address
 *
 * @param addr The address to get the sector number of
 * @return uint8_t The sector number
 */
uint8_t blFlashSectorOfAddr(uint32_t addr) BL_FLASH_ATTR_RAMFUNC_SECTION;

/**
 * @brief Get the start address of a sector
 *
 * @param sector The sector number
 * @return uint32_t The start address of the sector
 */
uint32_t blFlashSectorStartAddr(uint8_t sector) BL_FLASH_ATTR_RAMFUNC_SECTION;

/**
 * @brief Get the end address of a sector
 *
 * @param sector The sector number
 * @return uint32_t The end address of the sector
 */
uint32_t blFlashSectorEndAddr(uint8_t sector) BL_FLASH_ATTR_RAMFUNC_SECTION;

/**
 * @brief Get the number of sectors
 *
 * @return uint8_t The number of sectors
 */
uint8_t blFlashGetNumSectors(void) BL_FLASH_ATTR_RAMFUNC_SECTION;
