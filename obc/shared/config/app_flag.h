#pragma once
#include <stdint.h>

#define ENABLE_APP_B_MAGIC_NUM 0x4F4F4F4F
#define APP_STATE_HAMMING_DISTANCE (0x55U)

typedef enum {
  SUSPENDED = 0x00,
  TESTING = APP_STATE_HAMMING_DISTANCE,
  DEPRIORITIZED = APP_STATE_HAMMING_DISTANCE * 2,
  VALIDATED = APP_STATE_HAMMING_DISTANCE * 3,
} app_state_t;

typedef struct {
  uint8_t boot_attempts;
  app_state_t app_state;
} app_flag_t;
