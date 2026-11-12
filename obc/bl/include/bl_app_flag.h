#pragma once
#include <stdint.h>

#define ENABLE_APP_MAGIC_NUM 0x4F4F4F4F

typedef struct __attribute__((packed)) {
  uint32_t app_status;
  uint32_t boot_attempts;
  uint32_t app_b_flag;
  uint32_t crc_selector;
} app_flag_t;
