#pragma once
#include "obc_gs_command_data.h"
#include <stdint.h>

#define FMSG_FLAG_START_ADDRESS 0xF0204000
#define FMSG_FLAG_SECTION_SIZE 0x00004000

#define ENABLE_FORWARD_MAGIC_NUM 0x4F4F4F4F

typedef struct __attribute__((packed)) {
  cmd_msg_t fCmd;
  uint32_t fEnabled;
} fcmd_msg_t;
// Struct to store cmd message and enabled status for app->bl forwarded command messages
