#include "bl_verification.h"
#include "bl_uart.h"
#include "bl_flash.h"
#include "obc_gs_crc.h"
#include <string.h>
#include "obc_logging.h"
#include "bl_config.h"
#include "bl_time.h"

#define MEMORY_BLANK_CHECK_SIZE APP_WRITE_PACKET_SIZE

obc_error_code_t verifyBoardType(uint8_t boardType) {
  if (boardType == BOARD_ID) {
    return OBC_ERR_CODE_SUCCESS;
  } else {
    blUartWriteBytes(strlen("ERROR: Board ID of bootloader and app are different, aborting...\r\n"),
                     (uint8_t *)"ERROR: Board ID of bootloader and app are different, aborting...\r\n");
    return OBC_ERR_CODE_BOARD_MISMATCH;
  }
}

obc_error_code_t verifyCrc(uint32_t crcAddr) {
  if (blFlashFapiBlankCheck(crcAddr, 1)) {
    blUartWriteBytes(strlen("ERROR: CRC blank check failed\r\n"), (uint8_t *)"ERROR: CRC blank check failed\r\n");
    return OBC_ERR_CODE_CORRUPTED_APP;
  }
  // Calculate crc via the crc32 algorithm (same one used in python's binascii and zlib libraries)
  uint32_t calculatedCrc = crc32(0, (uint8_t *)APP_START_ADDRESS, crcAddr - APP_START_ADDRESS);

  if (calculatedCrc == *((uint32_t *)crcAddr)) {
    return OBC_ERR_CODE_SUCCESS;
  } else {
    blUartWriteBytes(strlen("ERROR: Failed to verify CRC\r\n"), (uint8_t *)"ERROR: Failed to verify CRC\r\n");
    return OBC_ERR_CODE_CORRUPTED_APP;
  }
}

obc_error_code_t verifyMagicNum(uint32_t magicNum) {
  if (magicNum == MAGIC_NUM) {
    return OBC_ERR_CODE_SUCCESS;
  } else {
    blUartWriteBytes(strlen("ERROR: Failed to verify Magic Number\r\n"),
                     (uint8_t *)"ERROR: Failed to verify Magic Number\r\n");
    return OBC_ERR_CODE_MAGIC_NUM_MISMATCH;
  }
}

obc_error_code_t verifyMetadata(metadata_t *app_metadata) {
  obc_error_code_t errCode;
  RETURN_IF_ERROR_CODE(verifyMagicNum(app_metadata->magic_num));
  RETURN_IF_ERROR_CODE(verifyBoardType(app_metadata->board_id));
  RETURN_IF_ERROR_CODE(verifyCrc(app_metadata->crc_addr));
  return OBC_ERR_CODE_SUCCESS;
}

// NOTE: This function does not check if the crc is written
obc_error_code_t blAppBlankCheck(metadata_t *app_metadata) {
  uint16_t writeSections = (app_metadata->crc_addr - APP_START_ADDRESS) / MEMORY_BLANK_CHECK_SIZE;

  for (uint16_t i = 0; i < writeSections; i++) {
    if (blFlashFapiBlankCheck(APP_START_ADDRESS + i * MEMORY_BLANK_CHECK_SIZE, MEMORY_BLANK_CHECK_SIZE / 4)) {
      blUartWriteBytes(strlen("ERROR: Blank check failed \r\n"), (uint8_t *)"ERROR: Blank check failed \r\n");
      return OBC_ERR_CODE_CORRUPTED_APP;
    }
  }

  // Any left over memory that needs to be checked
  if (blFlashFapiBlankCheck(APP_START_ADDRESS + writeSections * MEMORY_BLANK_CHECK_SIZE,
                            (app_metadata->crc_addr - APP_START_ADDRESS - writeSections * MEMORY_BLANK_CHECK_SIZE))) {
    blUartWriteBytes(strlen("ERROR: Blank check failed \r\n"), (uint8_t *)"ERROR: Blank check failed \r\n");
    return OBC_ERR_CODE_CORRUPTED_APP;
  }

  return OBC_ERR_CODE_SUCCESS;
}
