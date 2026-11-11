#pragma once
#include <stdint.h>
#include "obc_gs_command_data.h"

#define FCMD_FLAG_START_ADDRESS 0xF0204000
#define FCMD_FLAG_SECTION_SIZE 0x00004000

#define ENABLE_APP_MAGIC_NUM 0x4F4F4F4F

typedef struct __attribute__((packed)) {
  fcmd_msg_t fCmd;
  uint32_t flag;
} fcmd_flag_t;
