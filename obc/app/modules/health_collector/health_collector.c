#include "health_collector.h"

#include <FreeRTOS.h>
#include <os_task.h>
#include <sys_common.h>

#include "lm75bd.h"
#include "obc_errors.h"
#include "obc_logging.h"
#include "obc_scheduler_config.h"
#include "obc_time.h"
#include "telemetry_manager.h"

#define HEALTH_COLLECTION_PERIOD_MS 60000UL

static obc_error_code_t collectObcLm75bdTemp(void);

void obcTaskInitHealthCollector(void) {}

void obcTaskFunctionHealthCollector(void* pvParameters) {
  obc_error_code_t errCode;

  while (1) {
    LOG_IF_ERROR_CODE(collectObcLm75bdTemp());
    vTaskDelay(pdMS_TO_TICKS(HEALTH_COLLECTION_PERIOD_MS));
  }
}

static obc_error_code_t collectObcLm75bdTemp(void) {
  obc_error_code_t errCode;

  float temp = 0.0f;
  RETURN_IF_ERROR_CODE(readTempLM75BD(LM75BD_OBC_I2C_ADDR, &temp));

  telemetry_data_t obcTempVal = {.obcTemp = temp, .id = TELEM_OBC_TEMP, .timestamp = getCurrentUnixTime()};

  RETURN_IF_ERROR_CODE(addTelemetryData(&obcTempVal));

  return OBC_ERR_CODE_SUCCESS;
}
