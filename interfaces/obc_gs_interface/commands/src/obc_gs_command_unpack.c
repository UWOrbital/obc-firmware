#include "obc_gs_command_unpack.h"
#include "obc_gs_command_data.h"
#include "obc_gs_command_id.h"
#include "data_unpack_utils.h"
#include "obc_gs_errors.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/* Unpack functions for each command */

// CMD_EXEC_OBC_RESET
static void unpackExecObcResetCmdData(const uint8_t* buffer, uint32_t* offset, cmd_msg_t* msg);

// CMD_RTC_SYNC
static void unpackRtcSyncCmdData(const uint8_t* buffer, uint32_t* offset, cmd_msg_t* msg);

// CMD_DOWNLINK_LOGS_NEXT_PASS
static void unpackDownlinkLogsNextPassCmdData(const uint8_t* buffer, uint32_t* offset, cmd_msg_t* msg);

// CMD_MICRO_SD_FORMAT
static void unpackMicroSdFormat(const uint8_t* buffer, uint32_t* offset, cmd_msg_t* msg);

// CMD_PING
static void unpackPingCmdData(const uint8_t* buffer, uint32_t* offset, cmd_msg_t* msg);

// CMD_DOWNLINK_TELEM
static void unpackDownlinkTelemCmdData(const uint8_t* buffer, uint32_t* offset, cmd_msg_t* msg);

// CMD_UPLINK_DISC
static void unpackUplinkDisconnectCmdData(const uint8_t* buffer, uint32_t* offset, cmd_msg_t* msg);

// CMD_SET_PROGRAMMING_SESSION
static void unpackSetProgrammingSessionCmdData(const uint8_t* buffer, uint32_t* offset, cmd_msg_t* msg);

// CMD_SET_PROGRAMMING_SESSION
static void unpackSetProgrammingSessionCmdData(const uint8_t* buffer, uint32_t* offset, cmd_msg_t* msg);

// CMD_DOWNLOAD_DATA
static void unpackDownloadDataCmdData(const uint8_t* buffer, uint32_t* offset, cmd_msg_t* msg);

// CMD_ERASE_APP
static void unpackEraseAppCmdData(const uint8_t* buffer, uint32_t* offset, cmd_msg_t* msg);

// CMD_VERIFY_CRC
static void unpackVerifyCrcCmdData(const uint8_t* buffer, uint32_t* offset, cmd_msg_t* msg);

// CMD_I2C_PROBE
static void unpackI2CProbeCmdData(const uint8_t* buffer, uint32_t* offset, cmd_msg_t* msg);

typedef void (*unpack_func_t)(const uint8_t*, uint32_t*, cmd_msg_t*);

static const unpack_func_t unpackFns[] = {
    [CMD_EXEC_OBC_RESET] = unpackExecObcResetCmdData,
    [CMD_RTC_SYNC] = unpackRtcSyncCmdData,
    [CMD_DOWNLINK_LOGS_NEXT_PASS] = unpackDownlinkLogsNextPassCmdData,
    [CMD_MICRO_SD_FORMAT] = unpackMicroSdFormat,
    [CMD_PING] = unpackPingCmdData,
    [CMD_DOWNLINK_TELEM] = unpackDownlinkTelemCmdData,
    [CMD_UPLINK_DISC] = unpackUplinkDisconnectCmdData,
    [CMD_SET_PROGRAMMING_SESSION] = unpackSetProgrammingSessionCmdData,
    [CMD_DOWNLOAD_DATA] = unpackDownloadDataCmdData,
    [CMD_ERASE_APP] = unpackEraseAppCmdData,
    [CMD_VERIFY_CRC] = unpackVerifyCrcCmdData,
    [CMD_I2C_PROBE] = unpackI2CProbeCmdData,
    // Add more functions for other commands as needed
};

#define MAX_CMD_ID ((sizeof(unpackFns) / sizeof(unpack_func_t)) - 1)

// Unpack the command message
obc_gs_error_code_t unpackCmdMsg(const uint8_t* buffer, uint32_t* offset, cmd_msg_t* cmdMsg) {
  if (buffer == NULL || offset == NULL || cmdMsg == NULL) {
    return OBC_GS_ERR_CODE_INVALID_ARG;
  }

  uint8_t id = unpackUint8(buffer, offset);

  // MSB is 0 if the command is time tagged
  bool isTimeTagged = (id & 0x80);

  // Mask out the MSB
  id = id & 0x7F;

  if (id > MAX_CMD_ID) {
    return OBC_GS_ERR_CODE_UNSUPPORTED_CMD;
  }

  if (unpackFns[id] == NULL) {
    return OBC_GS_ERR_CODE_UNSUPPORTED_CMD;
  }

  uint32_t timestamp = unpackUint32(buffer, offset);

  cmdMsg->id = id;
  cmdMsg->timestamp = timestamp;
  cmdMsg->isTimeTagged = isTimeTagged;

  unpackFns[id](buffer, offset, cmdMsg);

  return OBC_GS_ERR_CODE_SUCCESS;
}

// CMD_EXEC_OBC_RESET
static void unpackExecObcResetCmdData(const uint8_t* buffer, uint32_t* offset, cmd_msg_t* cmdMsg) {
  // No data to unpack
}

// CMD_RTC_SYNC
static void unpackRtcSyncCmdData(const uint8_t* buffer, uint32_t* offset, cmd_msg_t* cmdMsg) {
  cmdMsg->rtcSync.unixTime = unpackUint32(buffer, offset);
}

// CMD_DOWNLINK_LOGS_NEXT_PASS
static void unpackDownlinkLogsNextPassCmdData(const uint8_t* buffer, uint32_t* offset, cmd_msg_t* cmdMsg) {
  cmdMsg->downlinkLogsNextPass.logLevel = unpackUint8(buffer, offset);
}

// CMD_MICRO_SD_FORMAT
static void unpackMicroSdFormat(const uint8_t* buffer, uint32_t* offset, cmd_msg_t* cmdMsg) {
  // No data to unpack
}

// CMD_PING
static void unpackPingCmdData(const uint8_t* buffer, uint32_t* offset, cmd_msg_t* cmdMsg) {
  // No data to unpack
}

// CMD_DOWNLINK_TELEM
static void unpackDownlinkTelemCmdData(const uint8_t* buffer, uint32_t* offset, cmd_msg_t* msg) {
  // No data to unpack
}

// CMD_UPLINK_DISC
static void unpackUplinkDisconnectCmdData(const uint8_t* buffer, uint32_t* offset, cmd_msg_t* msg) {
  // No data to unpack
}

// CMD_SET_PROGRAMMING_SESSION
static void unpackSetProgrammingSessionCmdData(const uint8_t* buffer, uint32_t* offset, cmd_msg_t* cmdMsg) {
  cmdMsg->setProgrammingSession.programmingSession = unpackUint8(buffer, offset);
}

// CMD_DOWNLOAD_DATA
static void unpackDownloadDataCmdData(const uint8_t* buffer, uint32_t* offset, cmd_msg_t* cmdMsg) {
  cmdMsg->downloadData.programmingSession = unpackUint8(buffer, offset);
  cmdMsg->downloadData.length = unpackUint16(buffer, offset);
  cmdMsg->downloadData.address = unpackUint32(buffer, offset);
  // Typecast necessary to avoid the const pointer to pointer conversion warning
  cmdMsg->downloadData.data = (uint8_t*)buffer + *offset;
}

// CMD_ERASE_APP
static void unpackEraseAppCmdData(const uint8_t* buffer, uint32_t* offset, cmd_msg_t* cmdMsg) {
  // No data to unpack
}

// CMD_VERIFY_CRC
static void unpackVerifyCrcCmdData(const uint8_t* buffer, uint32_t* offset, cmd_msg_t* cmdMsg) {
  // No data to unpack
}

// CMD_I2C_PROBE
static void unpackI2CProbeCmdData(const uint8_t* buffer, uint32_t* offset, cmd_msg_t* cmdMsg) {
  // No data to unpack
}
