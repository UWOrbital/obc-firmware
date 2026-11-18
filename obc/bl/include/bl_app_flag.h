#pragma once
#include <stdint.h>
#include "bl_state_mgr.h"

#define ENABLE_APP_B_MAGIC_NUM 0x4F4F4F4F

typedef struct {
  uint8_t boot_attempts;
  app_state_t app_state;
} app_flag_t;
