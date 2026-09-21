#include "thermal_mgr.h"
#include "lm75bd.h"
#include "obc_time.h"
#include "telemetry_manager.h"
#include "obc_errors.h"
#include "obc_logging.h"
#include "obc_scheduler_config.h"
#include "comms_manager.h"
#include "timekeeper.h"

#include <FreeRTOS.h>
#include <os_task.h>
#include <sys_common.h>

#define HEALTH_COLLECTION_PERIOD_MS 60000UL

static obc_error_code_t collectThermalData(void);

void obcTaskInitThermalMgr(void) {}

void obcTaskFunctionThermalMgr(void* pvParameters) {
  obc_error_code_t errCode;

  while (1) {
    LOG_IF_ERROR_CODE(collectThermalData());
    vTaskDelay(pdMS_TO_TICKS(HEALTH_COLLECTION_PERIOD_MS));
  }
}

static obc_error_code_t collectThermalData(void) {
  obc_error_code_t errCode;

  float lm75bdTemp = 0.0f;
  RETURN_IF_ERROR_CODE(readTempLM75BD(LM75BD_OBC_I2C_ADDR, &lm75bdTemp));

  float cc1120Temp = 0.0f;
  RETURN_IF_ERROR_CODE(readCC1120Temp(&cc1120Temp));

  float rtcTemp = 0.0f;
  RETURN_IF_ERROR_CODE(readRTCTemp(&rtcTemp));

  telemetry_data_t obcTempVal = {.obcTemp = lm75bdTemp, .id = TELEM_OBC_TEMP, .timestamp = getCurrentUnixTime()};

  RETURN_IF_ERROR_CODE(addTelemetryData(&obcTempVal));

  return OBC_ERR_CODE_SUCCESS;
}
