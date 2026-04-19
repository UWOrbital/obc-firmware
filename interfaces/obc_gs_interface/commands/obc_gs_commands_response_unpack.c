#include "data_unpack_utils.h"
#include "obc_gs_commands_response_unpack.h"
#include "obc_gs_command_id.h"
#include "obc_gs_commands_response.h"
#include "obc_gs_errors.h"

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

obc_gs_error_code_t unpackCmdResponse(uint8_t* buffer, cmd_response_header_t* response, uint8_t* responseDataBuffer) {
  if (response == NULL || buffer == NULL || responseDataBuffer == NULL) return OBC_GS_ERR_CODE_INVALID_ARG;

  uint32_t offset = 0;
  response->cmdId = unpackUint8(buffer, &offset);
  if (response->cmdId >= NUM_CMD_CALLBACKS) return OBC_GS_ERR_CODE_UNSUPPORTED_CMD;
  if (response->dataLen >= CMD_RESPONSE_DATA_MAX_SIZE) return OBC_GS_ERR_CODE_INVALID_ARG;

  obc_gs_error_code_t errCode = 0;
  response->errCode = (cmd_response_error_code_t)unpackUint8(buffer, &offset);
  response->dataLen = unpackUint8(buffer, &offset);

  memcpy(responseDataBuffer, &buffer[offset], CMD_RESPONSE_DATA_MAX_SIZE);
  return errCode;
}
