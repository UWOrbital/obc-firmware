#include <FreeRTOS.h>
#include <os_task.h>

#include "obc_errors.h"
#include "obc_scheduler_config.h"
#include "onboard_test_mgr.h"

const obc_scheduler_config_id_t onboardTestTasks[NUM_MODULES] = {
    [PLACEHOLDER_TASK] = OBC_SCHEDULER_CONFIG_ID_PLACEHOLDER_TEST,
};  // Add tasks here

// Implement test functions below

// Each function should follow the format:
// a) Is an RTOS task that runs a while loop
// b) Calls a peripheral and awaits expected behavior
// c) Sends a test_res_t instance to testResQueue (Using testResQueueHandle) upon
//    receiving expected behavior, stays silent otherwise.
//
// Note that you must reflect functions in scheduler_config.toml and digital_watchdog_mgr.c

// Ex:
// void obcTaskInitCommsManagerTest(void) {}
// void obcTaskFunctionCommsManagerTest( void *pvParameters ) { vTaskDelete(NULL); }

void obcTaskInitPlaceholderTest(void) {};
void obcTaskFunctionPlaceholderTest(void *pvParameters) {
  test_res_t testRes = {
      .errCode = OBC_ERR_CODE_SUCCESS,
      .moduleId = PLACEHOLDER_TASK,
  };

  sendToTestResQueue(&testRes);
  vTaskDelete(NULL);
};
