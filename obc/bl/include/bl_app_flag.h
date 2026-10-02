#pragma once
#include <stdint.h>

#define APP_FLAG_START_ADDRESS 0x00020000
#define APP_FLAG_SECTION_SIZE 0x00004000

#define ENABLE_APP_MAGIC_NUM 0x4F4F4F4F

typedef struct __attribute__((packed)) {
  uint32_t app_status;
  uint32_t boot_attempts;
  uint32_t app_b_flag;
  uint32_t crc_selector;
} app_flag_t;
