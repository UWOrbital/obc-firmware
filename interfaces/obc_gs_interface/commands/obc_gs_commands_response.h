#pragma once

#include "obc_gs_command_id.h"

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#define CMD_RESPONSE_SUCCESS_MASK 0x01
#define CMD_RESPONSE_DATA_MAX_SIZE 220

// NOTE: Update python error codes as well when these are updated
typedef enum { CMD_RESPONSE_SUCCESS = 0x01, CMD_RESPONSE_ERROR = 0x7F } cmd_response_error_code_t;

typedef struct {
  cmd_callback_id_t cmdId;
  cmd_response_error_code_t errCode;
  uint8_t dataLen;
} cmd_response_header_t;
