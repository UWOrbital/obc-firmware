#include "bl_state_mgr.h"
#include <stdbool.h>
#include "bl_config.h"
#include "bl_utils.h"
#include "obc_errors.h"
#include "bl_uart.h"
#include "app_flag.h"

#define KEY(app_A_state, app_B_state) ((((uint16_t)(app_A_state)) << 8) | ((uint16_t)(app_B_state)))

#define TESTING_BOOT_THRESHOLD 1U
#define VALIDATED_BOOT_THRESHOLD 5U

obc_error_code_t attemptBootAndUpdate(uint32_t activeApp, bool panic) {
  app_flag_t *app_flag_pointer;
  uint8_t threshold = {0};
  app_state_t nextState = {0};

  if (activeApp == 0) {
    app_flag_pointer = (app_flag_t *)APP_A_FLAG_START_ADDRESS;
  } else if (activeApp == ENABLE_APP_B_MAGIC_NUM) {
    app_flag_pointer = (app_flag_t *)APP_B_FLAG_START_ADDRESS;
  } else {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  switch (app_flag_pointer->app_state) {
    case SUSPENDED:
      return OBC_ERR_CODE_INVALID_STATE;
    case TESTING:
      threshold = TESTING_BOOT_THRESHOLD;
      nextState = SUSPENDED;
      break;
    case DEPRIORITIZED:
      threshold = VALIDATED_BOOT_THRESHOLD;
      nextState = TESTING;
      break;
    case VALIDATED:
      threshold = VALIDATED_BOOT_THRESHOLD;
      nextState = TESTING;
      break;
  }

  if (app_flag_pointer->boot_attempts <= threshold) {
    blJumpToApp(activeApp);
  } else if (app_flag_pointer->app_state >= TESTING || panic) {
    panic ? (app_flag_pointer->boot_attempts = 0) : (app_flag_pointer->app_state = nextState);
  } else {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  app_flag_pointer->boot_attempts += 1;

  return editAppFlag(*app_flag_pointer, activeApp);
}

obc_error_code_t blStateMgr() {
  obc_error_code_t errCode = OBC_ERR_CODE_SUCCESS;

  app_flag_t *app_A_flag_pointer = (app_flag_t *)APP_A_FLAG_START_ADDRESS;
  app_flag_t *app_B_flag_pointer = (app_flag_t *)APP_B_FLAG_START_ADDRESS;

  uint16_t key = KEY(app_A_flag_pointer->app_state, app_B_flag_pointer->app_state);

  if (key == KEY(VALIDATED, DEPRIORITIZED)) {
    blUartWriteBytes(strlen("APP A: VALIDATED\r\n APP B: DEPRIORITIZED\r\n"),
                     (uint8_t *)"APP A: VALIDATED\r\n APP B: DEPRIORITIZED\r\n");
    attemptBootAndUpdate(ENABLE_APP_B_MAGIC_NUM, false);
  } else if (app_B_flag_pointer->app_state == DEPRIORITIZED) {
    app_B_flag_pointer->app_state = VALIDATED;

    editAppFlag(*app_B_flag_pointer, ENABLE_APP_B_MAGIC_NUM);
  }

  switch (key) {
    case KEY(VALIDATED, VALIDATED):
      blUartWriteBytes(strlen("APP A: VALIDATED\r\n APP B: VALIDATED\r\n"),
                       (uint8_t *)"APP A: VALIDATED\r\n APP B: VALIDATED\r\n");
      attemptBootAndUpdate(ENABLE_APP_B_MAGIC_NUM, false);
      break;

    case KEY(VALIDATED, TESTING):
      blUartWriteBytes(strlen("APP A: VALIDATED\r\n APP B: TESTING\r\n"),
                       (uint8_t *)"APP A: VALIDATED\r\n APP B: TESTING\r\n");
      attemptBootAndUpdate(ENABLE_APP_B_MAGIC_NUM, false);
      break;

    case KEY(VALIDATED, SUSPENDED):
      blUartWriteBytes(strlen("APP A: VALIDATED\r\n APP B: SUSPENDED\r\n"),
                       (uint8_t *)"APP A: VALIDATED\r\n APP B: SUSPENDED\r\n");
      attemptBootAndUpdate(0, false);
      break;

    case KEY(TESTING, VALIDATED):
      blUartWriteBytes(strlen("APP A: TESTING\r\n APP B: SUSPENDED\r\n"),
                       (uint8_t *)"APP A: TESTING\r\n APP B: SUSPENDED\r\n");
      attemptBootAndUpdate(0, false);
      break;

    case KEY(SUSPENDED, VALIDATED):
      blUartWriteBytes(strlen("APP A: SUSPENDED\r\n APP B: SUSPENDED\r\n"),
                       (uint8_t *)"APP A: SUSPENDED\r\n APP B: SUSPENDED\r\n");
      attemptBootAndUpdate(ENABLE_APP_B_MAGIC_NUM, false);
      break;

    // Panic states (Neither app is validated)
    case KEY(TESTING, TESTING):
      blUartWriteBytes(strlen("APP A: TESTING\r\n APP B: TESTING\r\n"),
                       (uint8_t *)"APP A: TESTING\r\n APP B: TESTING\r\n");
      if (app_A_flag_pointer->boot_attempts < app_B_flag_pointer->boot_attempts) {
        attemptBootAndUpdate(0, false);
      } else {
        attemptBootAndUpdate(ENABLE_APP_B_MAGIC_NUM, false);
      }
      break;

    case KEY(TESTING, SUSPENDED):
      blUartWriteBytes(strlen("APP A: TESTING\r\n APP B: SUSPENDED\r\n"),
                       (uint8_t *)"APP A: TESTING\r\n APP B: SUSPENDED\r\n");
      attemptBootAndUpdate(0, true);
      break;
    case KEY(SUSPENDED, TESTING):
      blUartWriteBytes(strlen("APP A: SUSPENDED\r\n APP B: TESTING\r\n"),
                       (uint8_t *)"APP A: SUSPENDED\r\n APP B: TESTING\r\n");
      attemptBootAndUpdate(ENABLE_APP_B_MAGIC_NUM, true);
      break;

    case KEY(SUSPENDED, SUSPENDED):
      blUartWriteBytes(strlen("APP A: SUSPENDED\r\n APP B: SUSPENDED\r\n"),
                       (uint8_t *)"APP A: SUSPENDED\r\n APP B: SUSPENDED\r\n");

      // Reset for a hail mary.
      app_A_flag_pointer->app_state = TESTING;
      app_B_flag_pointer->app_state = TESTING;

      editAppFlag(*app_A_flag_pointer, 0);
      editAppFlag(*app_B_flag_pointer, ENABLE_APP_B_MAGIC_NUM);
      break;

    default:
      errCode = OBC_ERR_CODE_INVALID_STATE;
      break;
  }

  return errCode;
}
