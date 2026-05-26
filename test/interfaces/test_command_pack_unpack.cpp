#include "obc_gs_command_pack.h"
#include "obc_gs_command_unpack.h"
#include "obc_gs_command_data.h"
#include "obc_gs_command_id.h"
#include "obc_gs_errors.h"

#include <gtest/gtest.h>
#include <stdio.h>

// CMD_EXEC_OBC_RESET
TEST(TestCommandPackUnpack, ValidCmdExecObcResetPackUnpack) {
  obc_gs_error_code_t errCode;
  cmd_msg_t cmdMsg = {0};
  cmdMsg.id = CMD_EXEC_OBC_RESET;

  uint8_t buff[MAX_CMD_MSG_SIZE] = {0};
  uint32_t packOffset = 0;
  uint8_t numPacked = 0;
  errCode = packCmdMsg(buff, &packOffset, &cmdMsg, &numPacked);
  ASSERT_EQ(errCode, OBC_GS_ERR_CODE_SUCCESS);

  cmd_msg_t unpackedCmdMsg = {0};
  uint32_t unpackOffset = 0;
  errCode = unpackCmdMsg(buff, &unpackOffset, &unpackedCmdMsg);
  ASSERT_EQ(errCode, OBC_GS_ERR_CODE_SUCCESS);

  EXPECT_EQ(packOffset, unpackOffset);
  EXPECT_EQ(cmdMsg.id, unpackedCmdMsg.id);
}

// CMD_RTC_SYNC
TEST(TestCommandPackUnpack, ValidCmdRtcSyncPackUnpack) {
  obc_gs_error_code_t errCode;
  cmd_msg_t cmdMsg = {0};
  cmdMsg.id = CMD_RTC_SYNC;
  cmdMsg.rtcSync.unixTime = 0x12345678;

  uint8_t buff[MAX_CMD_MSG_SIZE] = {0};
  uint32_t packOffset = 0;
  uint8_t numPacked = 0;
  errCode = packCmdMsg(buff, &packOffset, &cmdMsg, &numPacked);
  ASSERT_EQ(errCode, OBC_GS_ERR_CODE_SUCCESS);

  cmd_msg_t unpackedCmdMsg = {0};
  uint32_t unpackOffset = 0;
  errCode = unpackCmdMsg(buff, &unpackOffset, &unpackedCmdMsg);
  ASSERT_EQ(errCode, OBC_GS_ERR_CODE_SUCCESS);

  EXPECT_EQ(packOffset, unpackOffset);
  EXPECT_EQ(cmdMsg.id, unpackedCmdMsg.id);
  EXPECT_EQ(cmdMsg.rtcSync.unixTime, unpackedCmdMsg.rtcSync.unixTime);
}

// CMD_DOWNLINK_LOGS_NEXT_PASS
TEST(TestCommandPackUnpack, ValidCmdDownlinkLogsNextPassPackUnpack) {
  obc_gs_error_code_t errCode;
  cmd_msg_t cmdMsg = {0};
  cmdMsg.id = CMD_DOWNLINK_LOGS_NEXT_PASS;
  cmdMsg.downlinkLogsNextPass.logLevel = 0;

  uint8_t buff[MAX_CMD_MSG_SIZE] = {0};
  uint32_t packOffset = 0;
  uint8_t numPacked = 0;
  errCode = packCmdMsg(buff, &packOffset, &cmdMsg, &numPacked);
  ASSERT_EQ(errCode, OBC_GS_ERR_CODE_SUCCESS);

  cmd_msg_t unpackedCmdMsg = {0};
  uint32_t unpackOffset = 0;
  errCode = unpackCmdMsg(buff, &unpackOffset, &unpackedCmdMsg);
  ASSERT_EQ(errCode, OBC_GS_ERR_CODE_SUCCESS);

  EXPECT_EQ(packOffset, unpackOffset);
  EXPECT_EQ(cmdMsg.id, unpackedCmdMsg.id);
  EXPECT_EQ(cmdMsg.downlinkLogsNextPass.logLevel, unpackedCmdMsg.downlinkLogsNextPass.logLevel);
}

// CMD_MICRO_SD_FORMAT
TEST(TestCommandPackUnpack, ValidCmdMicroSdFormatPackUnpack) {
  obc_gs_error_code_t errCode;
  cmd_msg_t cmdMsg = {0};
  cmdMsg.id = CMD_MICRO_SD_FORMAT;

  uint8_t buff[MAX_CMD_MSG_SIZE] = {0};
  uint32_t packOffset = 0;
  uint8_t numPacked = 0;
  errCode = packCmdMsg(buff, &packOffset, &cmdMsg, &numPacked);
  ASSERT_EQ(errCode, OBC_GS_ERR_CODE_SUCCESS);

  cmd_msg_t unpackedCmdMsg = {0};
  uint32_t unpackOffset = 0;
  errCode = unpackCmdMsg(buff, &unpackOffset, &unpackedCmdMsg);
  ASSERT_EQ(errCode, OBC_GS_ERR_CODE_SUCCESS);

  EXPECT_EQ(packOffset, unpackOffset);
  EXPECT_EQ(cmdMsg.id, unpackedCmdMsg.id);
}

// CMD_PING
TEST(TestCommandPackUnpack, ValidCmdPingPackUnpack) {
  obc_gs_error_code_t errCode;
  cmd_msg_t cmdMsg = {0};
  cmdMsg.id = CMD_PING;

  uint8_t buff[MAX_CMD_MSG_SIZE] = {0};
  uint32_t packOffset = 0;
  uint8_t numPacked = 0;
  errCode = packCmdMsg(buff, &packOffset, &cmdMsg, &numPacked);
  ASSERT_EQ(errCode, OBC_GS_ERR_CODE_SUCCESS);

  cmd_msg_t unpackedCmdMsg = {0};
  uint32_t unpackOffset = 0;
  errCode = unpackCmdMsg(buff, &unpackOffset, &unpackedCmdMsg);
  ASSERT_EQ(errCode, OBC_GS_ERR_CODE_SUCCESS);

  EXPECT_EQ(packOffset, unpackOffset);
  EXPECT_EQ(cmdMsg.id, unpackedCmdMsg.id);
}

// CMD_DOWNLINK_TELEM
TEST(TestCommandPackUnpack, ValidCmdDownlinkTelemPackUnpack) {
  obc_gs_error_code_t errCode;
  cmd_msg_t cmdMsg = {0};
  cmdMsg.id = CMD_DOWNLINK_TELEM;

  uint8_t buff[MAX_CMD_MSG_SIZE] = {0};
  uint32_t packOffset = 0;
  uint8_t numPacked = 0;
  errCode = packCmdMsg(buff, &packOffset, &cmdMsg, &numPacked);
  ASSERT_EQ(errCode, OBC_GS_ERR_CODE_SUCCESS);

  cmd_msg_t unpackedCmdMsg = {0};
  uint32_t unpackOffset = 0;
  errCode = unpackCmdMsg(buff, &unpackOffset, &unpackedCmdMsg);
  ASSERT_EQ(errCode, OBC_GS_ERR_CODE_SUCCESS);

  EXPECT_EQ(packOffset, unpackOffset);
  EXPECT_EQ(cmdMsg.id, unpackedCmdMsg.id);
}

// CMD_UPLINK_DISC
TEST(TestCommandPackUnpack, ValidCmdUplinkDisconnectPackUnpack) {
  obc_gs_error_code_t errCode;
  cmd_msg_t cmdMsg = {0};
  cmdMsg.id = CMD_UPLINK_DISC;

  uint8_t buff[MAX_CMD_MSG_SIZE] = {0};
  uint32_t packOffset = 0;
  uint8_t numPacked = 0;
  errCode = packCmdMsg(buff, &packOffset, &cmdMsg, &numPacked);
  ASSERT_EQ(errCode, OBC_GS_ERR_CODE_SUCCESS);

  cmd_msg_t unpackedCmdMsg = {0};
  uint32_t unpackOffset = 0;
  errCode = unpackCmdMsg(buff, &unpackOffset, &unpackedCmdMsg);
  ASSERT_EQ(errCode, OBC_GS_ERR_CODE_SUCCESS);

  EXPECT_EQ(packOffset, unpackOffset);
  EXPECT_EQ(cmdMsg.id, unpackedCmdMsg.id);
}

// CMD_SET_PROGRAMMING_SESSION
TEST(TestCommandPackUnpack, ValidCmdSetProgrammingSessionPackUnpack) {
  obc_gs_error_code_t errCode;
  cmd_msg_t cmdMsg = {0};
  cmdMsg.id = CMD_SET_PROGRAMMING_SESSION;
  cmdMsg.setProgrammingSession.programmingSession = APPLICATION;

  uint8_t buff[MAX_CMD_MSG_SIZE] = {0};
  uint32_t packOffset = 0;
  uint8_t numPacked = 0;
  errCode = packCmdMsg(buff, &packOffset, &cmdMsg, &numPacked);
  ASSERT_EQ(errCode, OBC_GS_ERR_CODE_SUCCESS);

  for (int i = 0; i < MAX_CMD_MSG_SIZE; i++) {
    printf(" 0x%x", buff[i]);
  }

  cmd_msg_t unpackedCmdMsg = {0};
  uint32_t unpackOffset = 0;
  errCode = unpackCmdMsg(buff, &unpackOffset, &unpackedCmdMsg);
  ASSERT_EQ(errCode, OBC_GS_ERR_CODE_SUCCESS);

  EXPECT_EQ(packOffset, unpackOffset);
  EXPECT_EQ(cmdMsg.id, unpackedCmdMsg.id);
  EXPECT_EQ(cmdMsg.setProgrammingSession.programmingSession, unpackedCmdMsg.setProgrammingSession.programmingSession);
}

// CMD_DOWNLOAD_DATA
TEST(TestCommandPackUnpack, ValidCmdDownloadDataPackUnpack) {
  obc_gs_error_code_t errCode;
  cmd_msg_t cmdMsg = {0};
  cmdMsg.id = CMD_DOWNLOAD_DATA;
  cmdMsg.downloadData.programmingSession = APPLICATION;
  cmdMsg.downloadData.length = 208;
  cmdMsg.downloadData.address = 0x00040000;

  uint8_t buff[MAX_CMD_MSG_SIZE] = {0};
  uint32_t packOffset = 0;
  uint8_t numPacked = 0;
  errCode = packCmdMsg(buff, &packOffset, &cmdMsg, &numPacked);
  ASSERT_EQ(errCode, OBC_GS_ERR_CODE_SUCCESS);

  for (int i = 0; i < MAX_CMD_MSG_SIZE; i++) {
    printf(" 0x%x", buff[i]);
  }
  printf(" %d", packOffset);

  cmd_msg_t unpackedCmdMsg = {0};
  uint32_t unpackOffset = 0;
  errCode = unpackCmdMsg(buff, &unpackOffset, &unpackedCmdMsg);
  ASSERT_EQ(errCode, OBC_GS_ERR_CODE_SUCCESS);

  EXPECT_EQ(packOffset, unpackOffset);
  EXPECT_EQ(cmdMsg.id, unpackedCmdMsg.id);
  EXPECT_EQ(cmdMsg.downloadData.programmingSession, unpackedCmdMsg.downloadData.programmingSession);
  EXPECT_EQ(cmdMsg.downloadData.length, unpackedCmdMsg.downloadData.length);
  EXPECT_EQ(cmdMsg.downloadData.address, unpackedCmdMsg.downloadData.address);
}

// CMD_ERASE_APP
TEST(TestCommandPackUnpack, ValidCmdEraseAppPackUnpack) {
  obc_gs_error_code_t errCode;
  cmd_msg_t cmdMsg = {0};
  cmdMsg.id = CMD_ERASE_APP;

  uint8_t buff[MAX_CMD_MSG_SIZE] = {0};
  uint32_t packOffset = 0;
  uint8_t numPacked = 0;
  errCode = packCmdMsg(buff, &packOffset, &cmdMsg, &numPacked);
  ASSERT_EQ(errCode, OBC_GS_ERR_CODE_SUCCESS);

  for (int i = 0; i < MAX_CMD_MSG_SIZE; i++) {
    printf(" 0x%x", buff[i]);
  }
  printf(" %d", packOffset);

  cmd_msg_t unpackedCmdMsg = {0};
  uint32_t unpackOffset = 0;
  errCode = unpackCmdMsg(buff, &unpackOffset, &unpackedCmdMsg);
  ASSERT_EQ(errCode, OBC_GS_ERR_CODE_SUCCESS);

  EXPECT_EQ(packOffset, unpackOffset);
  EXPECT_EQ(cmdMsg.id, unpackedCmdMsg.id);
}
