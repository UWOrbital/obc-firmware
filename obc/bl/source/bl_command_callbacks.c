#include "reg_system.h"
#include "bl_errors.h"
#include "bl_config.h"
#include "bl_time.h"
#include "obc_errors.h"
#include "obc_general_util.h"
#include "obc_gs_command_data.h"
#include "obc_gs_command_id.h"
#include "command.h"
#include <stddef.h>
#include "obc_gs_crc.h"
#include <stdint.h>
#include "bl_uart.h"
#include "bl_flash.h"
#include "obc_metadata.h"
#include <stdio.h>

#define BL_BIN_RX_CHUNK_SIZE 208U  // Bytes
#define MAX_PACKET_SIZE 223
#define RESET_SYSTEM_MASK (1 << 15)

programming_session_t programmingSession = APPLICATION;
extern uint32_t __APP_IMAGE_TOTAL_SECTION_SIZE;

static obc_error_code_t pingCmdCallback(cmd_msg_t *cmd, uint8_t *responseData, uint8_t *responseDataLen) {
  if (cmd == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }
  return OBC_ERR_CODE_SUCCESS;
}

static obc_error_code_t setProgrammingSessionCmdCallback(cmd_msg_t *cmd, uint8_t *responseData,
                                                         uint8_t *responseDataLen) {
  if (cmd == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  if (cmd->setProgrammingSession.programmingSession == APPLICATION) {
    programmingSession = APPLICATION;
  } else {
    return OBC_ERR_CODE_CORRUPTED_MSG;
  }
  return OBC_ERR_CODE_SUCCESS;
}

static obc_error_code_t eraseAppCmdCallback(cmd_msg_t *cmd, uint8_t *responseData, uint8_t *responseDataLen) {
  if (cmd == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }
  bl_error_code_t errCode =
      blFlashFapiBlockErase((uint32_t)APP_START_ADDRESS, (uint32_t)&__APP_IMAGE_TOTAL_SECTION_SIZE - 1);

  if (errCode != BL_ERR_CODE_SUCCESS) {
    char blUartWriteBuffer[BL_MAX_MSG_SIZE] = {0};
    int32_t blUartWriteBufferLen =
        snprintf(blUartWriteBuffer, BL_MAX_MSG_SIZE, "Failed to erase, BL error code: %d\r\n", errCode);
    if (blUartWriteBufferLen < 0) {
      uint8_t msgSize = sizeof("Error with processing message buffer length\r\n");
      memcpy(responseData, "Error with processing message buffer length\r\n", msgSize);
      *responseDataLen = msgSize;
    } else {
      memcpy(responseData, blUartWriteBuffer, blUartWriteBufferLen);
      *responseDataLen = blUartWriteBufferLen;
    }
    return OBC_ERR_CODE_FAILED_FILE_WRITE;
  }

  return OBC_ERR_CODE_SUCCESS;
}

static obc_error_code_t downloadDataCmdCallback(cmd_msg_t *cmd, uint8_t *responseData, uint8_t *responseDataLen) {
  if (cmd == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }
  // TODO: Replace magic number
  if (!blFlashIsStartAddrValid(cmd->downloadData.address, APP_WRITE_PACKET_SIZE)) {
    uint8_t msgSize = sizeof("Invalid start address\r\n");
    memcpy(responseData, "Invalid start address\r\n", msgSize);
    *responseDataLen = msgSize;
    return OBC_ERR_CODE_INVALID_ARG;
  }

  if ((cmd->downloadData.address - APP_START_ADDRESS) % APP_WRITE_PACKET_SIZE != 0) {
    uint8_t msgSize = sizeof("Start address not 208 byte aligned\r\n");
    memcpy(responseData, "Start address not 208 byte aligned\r\n", msgSize);
    *responseDataLen = msgSize;

    return OBC_ERR_CODE_INVALID_ARG;
  }

  // TODO: Figure out why you need to write a byte here before writing
  blUartWriteBytes(1, (uint8_t *)"W");

  bl_error_code_t errCode =
      blFlashFapiBlockWrite(cmd->downloadData.address, (uint32_t)cmd->downloadData.data, cmd->downloadData.length);

  if (errCode != BL_ERR_CODE_SUCCESS) {
    char blUartWriteBuffer[BL_MAX_MSG_SIZE] = {0};
    int32_t blUartWriteBufferLen =
        snprintf(blUartWriteBuffer, BL_MAX_MSG_SIZE, "Failed to write, BL error code: %d\r\n", errCode);
    if (blUartWriteBufferLen < 0) {
      uint8_t msgSize = sizeof("Error with processing message buffer length\r\n");
      memcpy(responseData, "Error with processing message buffer length\r\n", msgSize);
      *responseDataLen = msgSize;
    } else {
      memcpy(responseData, blUartWriteBuffer, blUartWriteBufferLen);
      *responseDataLen = blUartWriteBufferLen;
    }
    return OBC_ERR_CODE_FAILED_FILE_WRITE;
  }

  return OBC_ERR_CODE_SUCCESS;
}

static obc_error_code_t verifyCrcCmdCallback(cmd_msg_t *cmd, uint8_t *responseData, uint8_t *responseDataLen) {
  if (cmd == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  // If a success error code is sent, it means that the memory is occupied
  if (blFlashFapiBlankCheck(APP_START_ADDRESS, 2)) {
    blUartWriteBytes(strlen("ERROR: Metadata blank check failed\r\n"),
                     (uint8_t *)"ERROR: Metadata blank check failed\r\n");
    return OBC_ERR_CODE_CORRUPTED_APP;
  }

  // Cast the metadata of the flash into a usable pointer
  metadata_t *app_metadata = (metadata_t *)(APP_START_ADDRESS + APP_METADATA_OFFSET);

  // TODO: Refactor blank check functions and check if the app is blank here

  uint32_t calculatedCrc = crc32(0, (uint8_t *)APP_START_ADDRESS, app_metadata->crc_addr - APP_START_ADDRESS);

  memcpy(responseData, &calculatedCrc, sizeof(calculatedCrc));
  *responseDataLen = sizeof(calculatedCrc);

  return OBC_ERR_CODE_SUCCESS;
}

static obc_error_code_t execObcResetCmdCallback(cmd_msg_t *cmd, uint8_t *responseData, uint8_t *responseDataLen) {
  if (cmd == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  // TODO: Implement OBC reset functionality
  blUartWriteBytes(strlen("Resetting System \r\n"), (uint8_t *)"Resetting System \r\n");
  systemREG1->SYSECR |= RESET_SYSTEM_MASK;
  while (1) {
  }

  return OBC_ERR_CODE_SUCCESS;
}

const cmd_info_t cmdsConfig[] = {
    [CMD_EXEC_OBC_RESET] = {execObcResetCmdCallback, CMD_POLICY_PROD, CMD_TYPE_NORMAL},
    [CMD_PING] = {pingCmdCallback, CMD_POLICY_PROD, CMD_TYPE_NORMAL},
    [CMD_SET_PROGRAMMING_SESSION] = {setProgrammingSessionCmdCallback, CMD_POLICY_PROD, CMD_TYPE_NORMAL},
    [CMD_ERASE_APP] = {eraseAppCmdCallback, CMD_POLICY_PROD, CMD_TYPE_NORMAL},
    [CMD_DOWNLOAD_DATA] = {downloadDataCmdCallback, CMD_POLICY_PROD, CMD_TYPE_NORMAL},
    [CMD_VERIFY_CRC] = {verifyCrcCmdCallback, CMD_POLICY_PROD, CMD_TYPE_NORMAL},
};

// This function is purely to trick the compiler into thinking we are using the cmdsConfig variable so we avoid the
// unused variable error
void unusedFunc() { UNUSED(cmdsConfig); }
