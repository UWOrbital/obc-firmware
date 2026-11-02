#include <FreeRTOSConfig.h>
#include <FreeRTOS.h>

#include "os_task.h"
#include "obc_reset.h"
#include "obc_logging.h"

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
  LOG_FATAL_FROM_ISR("***********************STACK OVERFLOW DETECTED!!!!!!***********************");
}

__attribute__((noreturn)) void freertosConfigAssert(log_file_ref_t file, unsigned long ulLine) {
  logErrorCode(LOG_FATAL, file, ulLine, OBC_ERR_CODE_FREERTOS_ASSERT_FAIL);

  vTaskDelay(15);

  resetSystem(RESET_REASON_FREERTOS_ASSERT_FAIL);
}
