#include "bl_uart.h"
#include "bl_flash.h"
#include "obc_gs_commands_response.h"
#include "obc_gs_commands_response_pack.h"
#include "obc_gs_errors.h"
#include "obc_gs_crc.h"
#include "obc_errors.h"
#include "obc_gs_command_data.h"
#include "obc_gs_command_unpack.h"
#include <metadata_struct.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include "command.h"
#include "obc_logging.h"
#include "bl_config.h"
#include "bl_errors.h"
#include "bl_time.h"
#include "obc_metadata.h"
#if defined(DEBUG) && !defined(OBC_REVISION_2)
#include <gio.h>
#endif

/* LINKER EXPORTED SYMBOLS */
extern uint32_t __ramFuncsLoadStart__;
extern uint32_t __ramFuncsSize__;

extern uint32_t __ramFuncsRunStart__;
extern uint32_t __ramFuncsRunEnd__;

/* DEFINES */
// These values were chosen so that the UART transfers and flash writes are quick, but don't
// use too much RAM
#define BL_ECC_FIX_CHUNK_SIZE 128U  // Bytes
#define LAST_SECTOR_START_ADDR blFlashSectorStartAddr(15U)
#define WAIT_FOREVER UINT32_MAX
#define MAX_PACKET_SIZE 223
#define EXTENDED_APP_JUMP_TIMEOUT 2000
#define DEFAULT_APP_JUMP_TIMEOUT 2000
#define LED_DELAY_MS 500
#define MEMORY_BLANK_CHECK_SIZE APP_WRITE_PACKET_SIZE

/* TYPEDEFS */
typedef void (*appStartFunc_t)(void);

// Get this from the bl_command_callbacks for simplicity
extern programming_session_t programmingSession;

static uint8_t sendBuffer[MAX_PACKET_SIZE] = {0};
static uint8_t responseBuffer[CMD_RESPONSE_DATA_MAX_SIZE] = {0};
static uint8_t recvBuffer[MAX_PACKET_SIZE] = {0};

obc_error_code_t blRunCommand(uint8_t recvBuffer[]) {
  if (recvBuffer == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  obc_error_code_t errCode = OBC_ERR_CODE_SUCCESS;
  cmd_info_t currCmdInfo;
  cmd_msg_t unpackedCmdMsg = {0};
  uint32_t unpackOffset = 0;

  obc_gs_error_code_t interfaceErr = unpackCmdMsg(recvBuffer, &unpackOffset, &unpackedCmdMsg);
  if (interfaceErr != OBC_GS_ERR_CODE_SUCCESS) {
    blUartWriteBytes(strlen("ERROR: Message Corrupted\r\n"), (uint8_t *)"ERROR: Message Corrupted\r\n");
    return OBC_ERR_CODE_CORRUPTED_MSG;
  }

  uint8_t responseDataLen = 0;
  memset(responseBuffer, 0, CMD_RESPONSE_DATA_MAX_SIZE);
  memset(sendBuffer, 0, MAX_PACKET_SIZE);
  cmd_response_header_t cmdResponse = {0};

  if (verifyCommand(&unpackedCmdMsg, &currCmdInfo) == OBC_ERR_CODE_SUCCESS &&
      processNonTimeTaggedCommand(&unpackedCmdMsg, &currCmdInfo, responseBuffer, &responseDataLen) ==
          OBC_ERR_CODE_SUCCESS) {
    cmdResponse.errCode = CMD_RESPONSE_SUCCESS;
  } else {
    cmdResponse.errCode = CMD_RESPONSE_ERROR;
  }

  cmdResponse.cmdId = unpackedCmdMsg.id;
  cmdResponse.dataLen = responseDataLen;

  if (packCmdResponse(&cmdResponse, sendBuffer, responseBuffer) != OBC_GS_ERR_CODE_SUCCESS) {
    LOG_ERROR_CODE(OBC_ERR_CODE_FAILED_PACK);
  } else {
    blUartWriteBytes(MAX_PACKET_SIZE, sendBuffer);
  }

  return errCode;
}

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

obc_error_code_t blJumpToApp() {
  obc_error_code_t errCode;

  // If a success error code is sent, it means that the memory is occupied
  if (blFlashFapiBlankCheck(APP_START_ADDRESS, 2)) {
    blUartWriteBytes(strlen("ERROR: Metadata blank check failed\r\n"),
                     (uint8_t *)"ERROR: Metadata blank check failed\r\n");
    return OBC_ERR_CODE_CORRUPTED_APP;
  }

  // Cast the metadata of the flash into a usable pointer
  metadata_t *app_metadata = (metadata_t *)(APP_START_ADDRESS + APP_METADATA_OFFSET);

  RETURN_IF_ERROR_CODE(blAppBlankCheck(app_metadata));

  // Check magic number, board id and verify the crc
  RETURN_IF_ERROR_CODE(verifyMetadata(app_metadata));

  blUartWriteBytes(strlen("ATTEMPTING: Running application...\r\n"),
                   (uint8_t *)"ATTEMPTING: Running application..\r\n");

  // We wait for about 100ms so that the remaining uart info can be sent before the buffer is cleared
  // by the app being initialized
  uint32_t initTime = blGetCurrentTick();
  while ((blGetCurrentTick() - initTime) < 10 || blGetCurrentTick() < initTime) {
  };

  // Go to the application's entry point
  uint32_t appStartAddress = (uint32_t)app_metadata->app_entry_func_addr;
  ((appStartFunc_t)appStartAddress)();

  // If it was not possible to jump to the app, we log that error here
  blUartWriteBytes(strlen("ERROR: Failed to run application\r\n"), (uint8_t *)"ERROR: Failed to run application\r\n");
  return OBC_ERR_CODE_FAILED_TO_LOAD_APP;
}

/* PUBLIC FUNCTIONS */
int main(void) {
  obc_error_code_t errCode = OBC_ERR_CODE_SUCCESS;

  blUartInit();
  blInitTick();
  gioInit();

  // F021 API and the functions that use it must be executed from RAM since they
  // can't execute from the same flash bank being modified
  memcpy(&__ramFuncsRunStart__, &__ramFuncsLoadStart__, (uint32_t)&__ramFuncsSize__);

  bl_error_code_t interfaceErr = blFlashFapiInitBank(RM46_FLASH_BANK);

  if (interfaceErr != BL_ERR_CODE_SUCCESS) {
    char blUartWriteBuffer[BL_MAX_MSG_SIZE] = {0};
    int32_t blUartWriteBufferLen =
        snprintf(blUartWriteBuffer, BL_MAX_MSG_SIZE, "Failed to init flash, BL error code: %d\r\n", errCode);
    if (blUartWriteBufferLen < 0) {
      blUartWriteBytes(strlen("Error with processing message buffer length\r\n"),
                       (uint8_t *)"Error with processing message buffer length\r\n");
    } else {
      blUartWriteBytes(blUartWriteBufferLen, (uint8_t *)blUartWriteBuffer);
    }
  }

  uint32_t jumpToAppTimeout = blGetCurrentTick() + DEFAULT_APP_JUMP_TIMEOUT;
  uint32_t ledTimeout = blGetCurrentTick() + LED_DELAY_MS;
  while (1) {
    if (blGetCurrentTick() > ledTimeout) {
#if defined(DEBUG) && !defined(OBC_REVISION_2)
      gioToggleBit(STATE_MGR_DEBUG_LED_GIO_PORT, STATE_MGR_DEBUG_LED_GIO_BIT);
#endif
      ledTimeout = blGetCurrentTick() + LED_DELAY_MS;
    }

    if (blUartReadBytes(recvBuffer, MAX_PACKET_SIZE, 100) == OBC_ERR_CODE_SUCCESS) {
      LOG_IF_ERROR_CODE(blRunCommand(recvBuffer));
      jumpToAppTimeout = blGetCurrentTick() + EXTENDED_APP_JUMP_TIMEOUT;
    }

    if (blGetCurrentTick() > jumpToAppTimeout) {
      LOG_IF_ERROR_CODE(blJumpToApp());
    }
  }
  memset(recvBuffer, 0, MAX_PACKET_SIZE);
}
