#include "thermal_mgr.h"
#include "lm75bd.h"
#include "obc_gs_telemetry_id.h"
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
  uint32_t timestamp = getCurrentUnixTime();
  float lm75bdTemp = 0.0f;
  RETURN_IF_ERROR_CODE(readTempLM75BD(LM75BD_OBC_I2C_ADDR, &lm75bdTemp));
  telemetry_data_t lm75bdTempVal = {.obcTemp = lm75bdTemp, .id = TELEM_LM75BD_TEMP, .timestamp = timestamp};
  RETURN_IF_ERROR_CODE(addTelemetryData(&lm75bdTempVal));

  /* TODO: Uncomment this if comms manager task being suspened issue is fixed.
  float cc1120Temp = 0.0f;
  RETURN_IF_ERROR_CODE(readCC1120Temp(&cc1120Temp));
  telemetry_data_t cc1120TempVal = {.obcTemp = cc1120Temp, .id = TELEM_CC1120_TEMP, .timestamp = timestamp};
  RETURN_IF_ERROR_CODE(addTelemetryData(&cc1120TempVal));
  */
  float rtcTemp = 0.0f;
  RETURN_IF_ERROR_CODE(readRTCTemp(&rtcTemp));
  telemetry_data_t rtcTempVal = {.obcTemp = rtcTemp, .id = TELEM_RTC_TEMP, .timestamp = timestamp};
  RETURN_IF_ERROR_CODE(addTelemetryData(&rtcTempVal));

  return OBC_ERR_CODE_SUCCESS;
}
