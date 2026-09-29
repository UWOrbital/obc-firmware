#include "bl_utils.h"
#include "bl_verification.h"
#include "bl_uart.h"
#include "bl_flash.h"
#include "obc_gs_commands_response.h"
#include "obc_gs_commands_response_pack.h"
#include "obc_gs_errors.h"
#include "obc_gs_command_data.h"
#include "obc_gs_command_unpack.h"
#include "command.h"
#include "obc_logging.h"
#include "bl_config.h"
#include "bl_time.h"

/* DEFINES */
#define MAX_PACKET_SIZE 223

/* TYPEDEFS */
typedef void (*appStartFunc_t)(void);

static uint8_t sendBuffer[MAX_PACKET_SIZE] = {0};
static uint8_t responseBuffer[CMD_RESPONSE_DATA_MAX_SIZE] = {0};

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
