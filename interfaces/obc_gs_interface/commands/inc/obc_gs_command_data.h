#pragma once

#include "obc_gs_command_id.h"

#include <stdint.h>
#include <stdbool.h>
/* -------------------------- */
/*   Command Data Structures  */
/* -------------------------- */
// CMD_RTC_SYNC
typedef struct {
  uint32_t unixTime;
} rtc_sync_cmd_data_t;

// CMD_DOWNLINK_LOGS_NEXT_PASS
typedef struct {
  uint8_t logLevel;
} downlink_logs_next_pass_cmd_data_t;

/* -------------------------- */
/* BL Command Data Structures */
/* -------------------------- */
// Programming Session Enum
typedef enum { APPLICATION = 0x00 } programming_session_t;

// CMD_DOWNLOAD_DATA
typedef struct {
  programming_session_t programmingSession;
  uint16_t length;
  uint32_t address;
  // Defined as a pointer to avoid packing
  uint8_t *data;
} download_data_cmd_data_t;

// CMD_SET_PROGRAMMING_SESSION
typedef struct {
  programming_session_t programmingSession;
} set_programming_session_cmd_data_t;

/* -------------------------- */
/*   Command Message Struct   */
/* -------------------------- */

typedef struct {
  union {
    rtc_sync_cmd_data_t rtcSync;
    downlink_logs_next_pass_cmd_data_t downlinkLogsNextPass;
    download_data_cmd_data_t downloadData;
    set_programming_session_cmd_data_t setProgrammingSession;
  };

  uint32_t timestamp;  // Unix timestamp in seconds
  bool isTimeTagged;   // If true, command will be executed at timestamp

  cmd_callback_id_t id;  // Command ID
} cmd_msg_t;

#define MAX_CMD_MSG_SIZE sizeof(cmd_msg_t)
