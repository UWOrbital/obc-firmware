#include "flash.h"
#include "flash_config.h"
#include "flash_errors.h"

#include "F021.h"
#include "reg_flash.h"
#include "sys_core.h"

#include <FapiFunctions.h>
#include <Types.h>
#include <stdint.h>
#include <stdbool.h>

/* DEFINES */
#define BL_FLASH_APP_SECTORS_MASK 0xFF00U  // Sectors 0-7 are reserved for the bootloader
#define FLASH_BANK_WIDTH_BYTES 16U         // Programming at an address is limited to the bank width number of bytes

#define SYS_CLK_FREQ 220UL  // MHz

#define METADATA_START_ADDRESS (uint32_t)0x0013ffe0
#define METADATA_SIZE_BYTES ((uint32_t)0x00140000 - METADATA_START_ADDRESS)

/* PUBLIC FUNCTION DEFINITIONS */
flash_error_code_t flashFapiInitBank(uint32_t bankNum) {
  if ((Fapi_initializeFlashBanks(SYS_CLK_FREQ)) != Fapi_Status_Success) {
    return FLASH_ERR_CODE_UNKNOWN;
  }

  if (Fapi_setActiveFlashBank((Fapi_FlashBankType)bankNum) != Fapi_Status_Success) {
    return FLASH_ERR_CODE_INVALID_ARG;
  }

  if (Fapi_enableMainBankSectors(BL_FLASH_APP_SECTORS_MASK) != Fapi_Status_Success) {
    return FLASH_ERR_CODE_UNKNOWN;
  }

  // Possible infinite loop, but watchdog should reset the device if it gets stuck
  flashWaitFsmReady();
  flashWaitFsmStatusSuccess();

  return FLASH_ERR_CODE_SUCCESS;
}

uint8_t flashSectorOfAddr(uint32_t addr) {
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

uint32_t flashSectorStartAddr(uint8_t sector) { return (uint32_t)(flashSectors[sector].start); }

uint32_t flashSectorEndAddr(uint8_t sector) {
  return (uint32_t)(flashSectors[sector].start) + flashSectors[sector].length;
}

uint8_t flashGetNumSectors(void) { return NUM_FLASH_SECTORS; }

flash_error_code_t flashFapiBlockErase(uint32_t startAddr, uint32_t size) {
  flash_error_code_t errCode = FLASH_ERR_CODE_SUCCESS;

  const uint32_t endAddr = startAddr + size;

  // Find the start and end of the sectors to erase. Assume flashSectors is sorted
  // by start address, and that the first sector starts at address 0

  const uint8_t startSector = flashSectorOfAddr(startAddr);
  const uint8_t endSector = flashSectorOfAddr(endAddr);

  for (uint8_t i = startSector; i < endSector + 1U; i++) {
    if (Fapi_issueAsyncCommandWithAddress(Fapi_EraseSector, flashSectors[i].start) != Fapi_Status_Success) {
      errCode = FLASH_ERR_CODE_UNKNOWN;
      break;
    }

    flashWaitFsmReady();
    flashWaitFsmStatusSuccess();
  }

  return errCode;
}

flash_error_code_t flashFapiBlockWrite(uint32_t dstAddr, uint32_t srcAddr, uint32_t numBytes) {
  flash_error_code_t errCode = FLASH_ERR_CODE_SUCCESS;

  register uint32_t src = srcAddr;
  register uint32_t dst = dstAddr;

  uint32_t bytesToFlashNext = numBytes < FLASH_BANK_WIDTH_BYTES ? numBytes : FLASH_BANK_WIDTH_BYTES;

  while (numBytes > 0) {
    if (Fapi_issueProgrammingCommand((uint32_t *)dst, (uint8_t *)src, (uint32_t)bytesToFlashNext, NULL, 0,
                                     Fapi_AutoEccGeneration) != Fapi_Status_Success) {
      errCode = FLASH_ERR_CODE_UNKNOWN;
      break;
    }

    flashWaitFsmReady();
    flashWaitFsmStatusSuccess();

    src += bytesToFlashNext;
    dst += bytesToFlashNext;

    numBytes -= bytesToFlashNext;

    if (numBytes < FLASH_BANK_WIDTH_BYTES) {
      bytesToFlashNext = numBytes;
    }
  }

  return errCode;
}

bool flashFapiBlankCheck(uint32_t startAddr, uint32_t size32) {
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

bool flashIsStartAddrValid(uint32_t addr, uint32_t binSize) {
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

void flashWaitFsmReady(void) {
  while (FAPI_CHECK_FSM_READY_BUSY != Fapi_Status_FsmReady) {
    asm(" NOP");
  }
}

void flashWaitFsmStatusSuccess(void) {
  while (FAPI_GET_FSM_STATUS != Fapi_Status_Success) {
    asm(" NOP");
  }
}
