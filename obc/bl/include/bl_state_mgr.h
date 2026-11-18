#pragma once

#include "obc_errors.h"

#define APP_STATE_HAMMING_DISTANCE (0x55U)

typedef enum {
  SUSPENDED = 0x00,
  TESTING = APP_STATE_HAMMING_DISTANCE,
  DEPRIORITIZED = APP_STATE_HAMMING_DISTANCE * 2,
  VALIDATED = APP_STATE_HAMMING_DISTANCE * 3,
} app_state_t;

obc_error_code_t blStateMgr();
