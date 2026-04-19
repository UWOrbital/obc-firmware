#include "data_pack_utils.h"
#include "obc_gs_command_id.h"
#include "obc_gs_commands_response.h"
#include "obc_gs_errors.h"
#include "obc_gs_commands_response_pack.h"

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

obc_gs_error_code_t packCmdResponse(cmd_response_header_t* cmdResHeader, uint8_t* buffer, uint8_t* responseData) {
  if (buffer == NULL || cmdResHeader == NULL) return OBC_GS_ERR_CODE_INVALID_ARG;

  if (cmdResHeader->cmdId >= NUM_CMD_CALLBACKS || cmdResHeader->dataLen > CMD_RESPONSE_DATA_MAX_SIZE) {
    return OBC_GS_ERR_CODE_INVALID_ARG;
  }

  uint32_t offset = 0;
  packUint8((uint8_t)cmdResHeader->cmdId, buffer, &offset);
  packUint8((uint8_t)cmdResHeader->errCode, buffer, &offset);
  packUint8((uint8_t)cmdResHeader->dataLen, buffer, &offset);
  memcpy(&buffer[offset], responseData, CMD_RESPONSE_DATA_MAX_SIZE);

  return OBC_GS_ERR_CODE_SUCCESS;
}
