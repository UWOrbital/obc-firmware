#include "bl_flash.h"

#include <stdbool.h>
#include <stdint.h>

#include "F021.h"
#include "bl_config.h"
#include "bl_errors.h"
#include "bl_flash_config.h"
#include "reg_flash.h"
#include "sys_core.h"

/* DEFINES */
#define BL_FLASH_APP_SECTORS_MASK 0xFF00U  // Sectors 0-7 are reserved for the bootloader
#define BL_FLASH_BANK_WIDTH_BYTES 16U      // Programming at an address is limited to the bank width number of bytes

/* PUBLIC FUNCTION DEFINITIONS */
bl_error_code_t blFlashFapiInitBank(uint32_t bankNum) {
  if ((Fapi_initializeFlashBanks(SYS_CLK_FREQ)) != Fapi_Status_Success) {
    return BL_ERR_CODE_UNKNOWN;
  }

  if (Fapi_setActiveFlashBank((Fapi_FlashBankType)bankNum) != Fapi_Status_Success) {
    return BL_ERR_CODE_INVALID_ARG;
  }

  if (Fapi_enableMainBankSectors(BL_FLASH_APP_SECTORS_MASK) != Fapi_Status_Success) {
    return BL_ERR_CODE_UNKNOWN;
  }

  // Possible infinite loop, but watchdog should reset the device if it gets stuck
  blFlashWaitFsmReady();
  blFlashWaitFsmStatusSuccess();

  return BL_ERR_CODE_SUCCESS;
}

uint8_t blFlashSectorOfAddr(uint32_t addr) {
  uint8_t sector = 0U;
  for (uint8_t i = 0U; i < NUM_FLASH_SECTORS; i++) {
    const uint32_t sectorStartAddr = (uint32_t)(flashSectors[i].start);
    const uint32_t sectorEndAddr = sectorStartAddr + flashSectors[i].length;

    if (addr >= sectorStartAddr && addr < sectorEndAddr) {
      sector = i;
      break;
    }
  }

  return sector;
}

uint32_t blFlashSectorStartAddr(uint8_t sector) { return (uint32_t)(flashSectors[sector].start); }

uint32_t blFlashSectorEndAddr(uint8_t sector) {
  return (uint32_t)(flashSectors[sector].start) + flashSectors[sector].length;
}

uint8_t blFlashGetNumSectors(void) { return NUM_FLASH_SECTORS; }

bl_error_code_t blFlashFapiBlockErase(uint32_t startAddr, uint32_t size) {
  bl_error_code_t errCode = BL_ERR_CODE_SUCCESS;

  const uint32_t endAddr = startAddr + size;

  // Find the start and end of the sectors to erase. Assume flashSectors is sorted
  // by start address, and that the first sector starts at address 0

  const uint8_t startSector = blFlashSectorOfAddr(startAddr);
  const uint8_t endSector = blFlashSectorOfAddr(endAddr);

  for (uint8_t i = startSector; i < endSector + 1U; i++) {
    if (Fapi_issueAsyncCommandWithAddress(Fapi_EraseSector, flashSectors[i].start) != Fapi_Status_Success) {
      errCode = BL_ERR_CODE_UNKNOWN;
      break;
    }

    blFlashWaitFsmReady();
    blFlashWaitFsmStatusSuccess();
  }

  return errCode;
}

bl_error_code_t blFlashFapiBlockWrite(uint32_t dstAddr, uint32_t srcAddr, uint32_t numBytes) {
  bl_error_code_t errCode = BL_ERR_CODE_SUCCESS;

  register uint32_t src = srcAddr;
  register uint32_t dst = dstAddr;

  uint32_t bytesToFlashNext = numBytes < BL_FLASH_BANK_WIDTH_BYTES ? numBytes : BL_FLASH_BANK_WIDTH_BYTES;

  while (numBytes > 0) {
    if (Fapi_issueProgrammingCommand((uint32_t *)dst, (uint8_t *)src, (uint32_t)bytesToFlashNext, NULL, 0,
                                     Fapi_AutoEccGeneration) != Fapi_Status_Success) {
      errCode = BL_ERR_CODE_UNKNOWN;
      break;
    }

    blFlashWaitFsmReady();
    blFlashWaitFsmStatusSuccess();

    src += bytesToFlashNext;
    dst += bytesToFlashNext;

    numBytes -= bytesToFlashNext;

    if (numBytes < BL_FLASH_BANK_WIDTH_BYTES) {
      bytesToFlashNext = numBytes;
    }
  }

  return errCode;
}

bool blFlashFapiBlankCheck(uint32_t startAddr, uint32_t size32) {
  Fapi_FlashStatusWordType wordType = {.au32StatusWord = {0}};
  _coreDisableFlashEcc_();
  flashWREG->FEDACCTRL1 = 0x00000005U;

  Fapi_StatusType status = Fapi_doBlankCheckByByte((uint8_t *)startAddr, size32, &wordType);
  flashWREG->FEDACCTRL1 = 0x000A060AU;
  _coreEnableFlashEcc_();

  if (status == Fapi_Status_Success) {
    return true;
  } else {
    return false;
  }
}

bool blFlashIsStartAddrValid(uint32_t addr, uint32_t binSize) {
  const uint32_t lastFlashAddr = (uint32_t)flashSectors[NUM_FLASH_SECTORS - 1].start +
                                 flashSectors[NUM_FLASH_SECTORS - 1].length - (METADATA_SIZE_BYTES);

  // Cannot write to the first sector (contains bootloader)
  if (addr <= (uint32_t)flashSectors[0].start) {
    return false;
  }

  if (addr + binSize > lastFlashAddr) {
    return false;
  }

  // Check for 16 byte alignment
  if (addr % 16 != 0) {
    return false;
  }

  return true;
}

void blFlashWaitFsmReady(void) {
  while (FAPI_CHECK_FSM_READY_BUSY != Fapi_Status_FsmReady) {
    asm(" NOP");
  }
}

void blFlashWaitFsmStatusSuccess(void) {
  while (FAPI_GET_FSM_STATUS != Fapi_Status_Success) {
    asm(" NOP");
  }
}
