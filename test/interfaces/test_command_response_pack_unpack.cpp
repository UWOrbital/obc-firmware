
#include "obc_gs_command_id.h"
#include "obc_gs_commands_response_pack.h"
#include "obc_gs_commands_response_unpack.h"
#include "obc_gs_fec.h"
#include "obc_gs_commands_response.h"
#include "data_unpack_utils.h"
#include "obc_gs_errors.h"

#include <iostream>
#include <gtest/gtest.h>
#include <stdbool.h>

TEST(pack_unpack_command_responses, packResponse) {
  cmd_response_header_t cmdResponse = {.cmdId = CMD_EXEC_OBC_RESET, .errCode = CMD_RESPONSE_SUCCESS, .dataLen = 2};

  uint8_t buffer[RS_DECODED_SIZE] = {0};
  uint8_t responseData[CMD_RESPONSE_DATA_MAX_SIZE] = {0};
  obc_gs_error_code_t errCode = packCmdResponse(&cmdResponse, buffer, responseData);
  ASSERT_EQ(errCode, OBC_GS_ERR_CODE_SUCCESS);

  uint32_t offset = 0;
  uint8_t cmdId = unpackUint8(buffer, &offset);
  cmd_response_error_code_t encoded = (cmd_response_error_code_t)unpackUint8(buffer, &offset);
  uint8_t dataLength = unpackUint8(buffer, &offset);

  EXPECT_EQ(cmdId, (uint8_t)CMD_EXEC_OBC_RESET);
  EXPECT_EQ(encoded, CMD_RESPONSE_SUCCESS);
  EXPECT_EQ(dataLength, 2);
}

TEST(pack_unpack_command_responses, unpackCommandResponse) {
  cmd_response_header_t cmdResponse = {.cmdId = CMD_EXEC_OBC_RESET, .errCode = CMD_RESPONSE_ERROR, .dataLen = 2};

  uint8_t buffer[RS_DECODED_SIZE] = {0};
  uint8_t responseData[CMD_RESPONSE_DATA_MAX_SIZE] = {0};
  obc_gs_error_code_t errCode = packCmdResponse(&cmdResponse, buffer, responseData);
  ASSERT_EQ(errCode, OBC_GS_ERR_CODE_SUCCESS);

  cmd_response_header_t cmdResponseUnpacked;
  errCode = unpackCmdResponse(buffer, &cmdResponseUnpacked, responseData);
  ASSERT_EQ(errCode, OBC_GS_ERR_CODE_SUCCESS);

  EXPECT_EQ(cmdResponseUnpacked.cmdId, cmdResponse.cmdId);
  EXPECT_EQ(cmdResponseUnpacked.errCode, cmdResponse.errCode);
  EXPECT_EQ(cmdResponseUnpacked.dataLen, cmdResponse.dataLen);
}

TEST(pack_unpack_command_responses, packInvalidCommand) {
  cmd_response_header_t cmdResponse = {.cmdId = NUM_CMD_CALLBACKS, .errCode = CMD_RESPONSE_ERROR, .dataLen = 223};

  uint8_t buffer[RS_DECODED_SIZE] = {0};
  uint8_t responseData[CMD_RESPONSE_DATA_MAX_SIZE] = {0};
  obc_gs_error_code_t errCode = packCmdResponse(&cmdResponse, buffer, responseData);
  ASSERT_EQ(errCode, OBC_GS_ERR_CODE_INVALID_ARG);
}

TEST(pack_unpack_command_responses, unpackInvalidCommand) {
  const uint8_t maxNumber = (uint8_t)NUM_CMD_CALLBACKS;
  uint8_t buffer[RS_DECODED_SIZE] = {[0] = maxNumber};
  uint8_t responseData[CMD_RESPONSE_DATA_MAX_SIZE] = {0};

  cmd_response_header_t deserializedResponse = {
      .cmdId = CMD_END_OF_FRAME, .errCode = CMD_RESPONSE_SUCCESS, .dataLen = 0};
  obc_gs_error_code_t errCode = unpackCmdResponse(buffer, &deserializedResponse, responseData);
  EXPECT_EQ(errCode, OBC_GS_ERR_CODE_UNSUPPORTED_CMD);
}
