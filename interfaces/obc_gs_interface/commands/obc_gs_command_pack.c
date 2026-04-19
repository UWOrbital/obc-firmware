#include "obc_gs_command_pack.h"
#include "obc_gs_command_data.h"
#include "obc_gs_command_id.h"
#include "data_pack_utils.h"
#include "obc_gs_errors.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/* Pack functions for each command */

// CMD_EXEC_OBC_RESET
static void packExecObcResetCmdData(uint8_t* buffer, uint32_t* offset, const cmd_msg_t* msg);

// CMD_RTC_SYNC
static void packRtcSyncCmdData(uint8_t* buffer, uint32_t* offset, const cmd_msg_t* msg);

// CMD_DOWNLINK_LOGS_NEXT_PASS
static void packDownlinkLogsNextPassCmdData(uint8_t* buffer, uint32_t* offset, const cmd_msg_t* msg);

// CMD_MICRO_SD_FORMAT
static void packMicroSdFormat(uint8_t* buffer, uint32_t* offset, const cmd_msg_t* cmdMsg);

// CMD_PING
static void packPingCmdData(uint8_t* buffer, uint32_t* offset, const cmd_msg_t* msg);

// CMD_DOWNLINK_TELEM
static void packDownlinkTelemCmdData(uint8_t* buffer, uint32_t* offset, const cmd_msg_t* msg);

// CMD_UPLINK_DISC
static void packUplinkDisconnectCmdData(uint8_t* buffer, uint32_t* offset, const cmd_msg_t* msg);

// CMD_SET_PROGRAMMING_SESSION
static void packSetProgrammingSessionCmdData(uint8_t* buffer, uint32_t* offset, const cmd_msg_t* msg);

// CMD_DOWNLOAD_DATA
static void packDownloadDataCmdData(uint8_t* buffer, uint32_t* offset, const cmd_msg_t* msg);

// CMD_ERASE_APP
static void packEraseAppCmdData(uint8_t* buffer, uint32_t* offset, const cmd_msg_t* msg);

// CMD_VERIFY_CRC
static void packVerifyCrcCmdData(uint8_t* buffer, uint32_t* offset, const cmd_msg_t* msg);

// CMD_I2C_PROBE
static void packI2CProbeCmdData(uint8_t* buffer, uint32_t* offset, const cmd_msg_t* msg);

typedef void (*pack_func_t)(uint8_t*, uint32_t*, const cmd_msg_t*);

static const pack_func_t packFns[] = {
    [CMD_EXEC_OBC_RESET] = packExecObcResetCmdData,
    [CMD_RTC_SYNC] = packRtcSyncCmdData,
    [CMD_DOWNLINK_LOGS_NEXT_PASS] = packDownlinkLogsNextPassCmdData,
    [CMD_MICRO_SD_FORMAT] = packMicroSdFormat,
    [CMD_PING] = packPingCmdData,
    [CMD_DOWNLINK_TELEM] = packDownlinkTelemCmdData,
    [CMD_UPLINK_DISC] = packUplinkDisconnectCmdData,
    [CMD_SET_PROGRAMMING_SESSION] = packSetProgrammingSessionCmdData,
    [CMD_DOWNLOAD_DATA] = packDownloadDataCmdData,
    [CMD_ERASE_APP] = packEraseAppCmdData,
    [CMD_VERIFY_CRC] = packVerifyCrcCmdData,
    [CMD_I2C_PROBE] = packI2CProbeCmdData,
    // Add more functions for other commands as needed
};

#define MAX_CMD_ID ((sizeof(packFns) / sizeof(pack_func_t)) - 1)

// Pack the command message
obc_gs_error_code_t packCmdMsg(uint8_t* buffer, uint32_t* offset, const cmd_msg_t* cmdMsg, uint8_t* numPacked) {
  if (buffer == NULL || offset == NULL || cmdMsg == NULL) {
    return OBC_GS_ERR_CODE_INVALID_ARG;
  }

  if (cmdMsg->id > MAX_CMD_ID) {
    return OBC_GS_ERR_CODE_UNSUPPORTED_CMD;
  }

  if (packFns[cmdMsg->id] == NULL) {
    return OBC_GS_ERR_CODE_UNSUPPORTED_CMD;
  }

  uint8_t oldOffset = *offset;
  uint8_t uplinkedId = cmdMsg->isTimeTagged ? (cmdMsg->id | 0x80) : cmdMsg->id;
  packUint8(uplinkedId, buffer, offset);

  packUint32(cmdMsg->timestamp, buffer, offset);

  packFns[cmdMsg->id](buffer, offset, cmdMsg);

  *numPacked = *offset - oldOffset;
  return OBC_GS_ERR_CODE_SUCCESS;
}

// CMD_EXEC_OBC_RESET
static void packExecObcResetCmdData(uint8_t* buffer, uint32_t* offset, const cmd_msg_t* cmdMsg) {
  // No data to pack
}

// CMD_RTC_SYNC
static void packRtcSyncCmdData(uint8_t* buffer, uint32_t* offset, const cmd_msg_t* cmdMsg) {
  packUint32(cmdMsg->rtcSync.unixTime, buffer, offset);
}

// CMD_DOWNLINK_LOGS_NEXT_PASS
static void packDownlinkLogsNextPassCmdData(uint8_t* buffer, uint32_t* offset, const cmd_msg_t* cmdMsg) {
  packUint8((uint8_t)cmdMsg->downlinkLogsNextPass.logLevel, buffer, offset);
}

// CMD_MICRO_SD_FORMAT
static void packMicroSdFormat(uint8_t* buffer, uint32_t* offset, const cmd_msg_t* cmdMsg) {
  // No data to pack
}

// CMD_PING
static void packPingCmdData(uint8_t* buffer, uint32_t* offset, const cmd_msg_t* cmdMsg) {
  // No data to pack
}

// CMD_DOWNLINK_TELEM
static void packDownlinkTelemCmdData(uint8_t* buffer, uint32_t* offset, const cmd_msg_t* msg) {
  // No data to pack
}

// CMD_UPLINK
static void packUplinkDisconnectCmdData(uint8_t* buffer, uint32_t* offset, const cmd_msg_t* msg) {
  // No data to pack
}

// CMD_SET_PROGRAMMING_SESSION
static void packSetProgrammingSessionCmdData(uint8_t* buffer, uint32_t* offset, const cmd_msg_t* cmdMsg) {
  packUint8((uint8_t)cmdMsg->setProgrammingSession.programmingSession, buffer, offset);
}

// CMD_DOWNLOAD_DATA
static void packDownloadDataCmdData(uint8_t* buffer, uint32_t* offset, const cmd_msg_t* cmdMsg) {
  packUint8((uint8_t)cmdMsg->downloadData.programmingSession, buffer, offset);
  packUint16(cmdMsg->downloadData.length, buffer, offset);
  packUint32(cmdMsg->downloadData.address, buffer, offset);
}

// CMD_ERASE_APP
static void packEraseAppCmdData(uint8_t* buffer, uint32_t* offset, const cmd_msg_t* cmdMsg) {
  // No data to pack
}

// CMD_VERIFY_CRC
static void packVerifyCrcCmdData(uint8_t* buffer, uint32_t* offset, const cmd_msg_t* cmdMsg) {
  // No data to pack
}

// CMD_I2C_PROBE
static void packI2CProbeCmdData(uint8_t* buffer, uint32_t* offset, const cmd_msg_t* msg) {
  // No data to pack
}
