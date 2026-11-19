#include "app_flag.h"
#include "magic_numbers.h"
#include "obc_errors.h"
#include "os_portmacro.h"
#include "os_projdefs.h"
#include "obc_metadata.h"
#include "command.h"
#include "test_mgr.h"
#include "alarm_handler.h"
#include "obc_time.h"
#include "obc_reset.h"

#include <FreeRTOS.h>
#include <stdint.h>
#include <sys_common.h>
#include <os_task.h>
#include <os_queue.h>

static bool testLogs[NUM_MODULES] = {0};

extern const onboard_test_callback_t onboardTests[];

#define ONBOARD_TEST_TIMEOUT 15

QueueHandle_t testResQueueHandle;
static StaticQueue_t testResQueue;
static uint8_t testResQueueStack[TEST_RES_QUEUE_LENGTH * TEST_RES_QUEUE_ITEM_SIZE];

void obcTaskInitTestMgr(void) {
  ASSERT((testResQueueStack != NULL) && (&testResQueue != NULL));
  if (testResQueueHandle == NULL) {
    testResQueueHandle =
        xQueueCreateStatic(TEST_RES_QUEUE_LENGTH, TEST_RES_QUEUE_ITEM_SIZE, testResQueueStack, &testResQueue);
  }
}

obc_error_code_t sendToTestResQueue(test_res_t *testRes) {
  if (testResQueueHandle == NULL) {
    return OBC_ERR_CODE_INVALID_STATE;
  }

  if (testRes == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  if (xQueueSend(testResQueueHandle, (void *)testRes, portMAX_DELAY) == pdPASS) {
    return OBC_ERR_CODE_SUCCESS;
  }

  return OBC_ERR_CODE_QUEUE_FULL;
}

void obcTaskFunctionTestMgr(void *pvParameters) {
  uint32_t startTime = getCurrentUnixTime();
  uint8_t occupied_slot = ((metadata_t *)(CUSTOM_START_ADDRESS + APP_ADDRESS_OFFSET))->occupied_slot;
  uint32_t activeApp = {0};

  if (occupied_slot == 0) {
    // Do nothing
  } else if (occupied_slot == SHORT_ENABLE_APP_MAGIC_NUMBER) {
    activeApp = ENABLE_APP_B_MAGIC_NUM;
  } else {
    // TODO: Should never reach here, add handling later
  }

  for (uint8_t i = 0; i < NUM_MODULES; ++i) {
    if (onboardTests[i]() != OBC_ERR_CODE_SUCCESS) {
      // TODO: log something
    }
  }

  while (1) {
    test_res_t testRes = {0};
    uint8_t validated = 0xFF;  // Magic number with large hamming distance from 0

    if (xQueueReceive(testResQueueHandle, &testRes, portMAX_DELAY) == pdPASS) {
      testLogs[testRes.moduleId] = testRes.errCode == OBC_ERR_CODE_SUCCESS ? true : false;

      for (uint8_t i = 0; i < NUM_MODULES; ++i) {
        if (testLogs[i] == 0) {
          validated = 0;
        }
      }
    }

    if (validated == 0xFF) {
      app_flag_t replacement_app_flag = {
          .app_state = VALIDATED,
          .boot_attempts = 0,
      };

      editAppFlag(replacement_app_flag, activeApp);
    }

    if (getCurrentUnixTime() - startTime > ONBOARD_TEST_TIMEOUT) {
      app_flag_t replacement_app_flag = {
          .app_state = SUSPENDED,
          .boot_attempts = 0,
      };

      editAppFlag(replacement_app_flag, activeApp);

      resetSystem(RESET_REASON_CMD_EXEC_OBC_RESET);
    }
  }
}
