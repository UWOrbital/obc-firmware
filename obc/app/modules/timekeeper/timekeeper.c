#include "timekeeper.h"
#include "obc_time.h"
#include "obc_errors.h"
#include "obc_assert.h"
#include "obc_logging.h"
#include "obc_persistent.h"
#include "obc_scheduler_config.h"
#include "ds3232_mz.h"

#include <FreeRTOS.h>
#include <os_task.h>
#include <os_timer.h>
#include <sys_common.h>
#include <stdint.h>

#define LOCAL_TIME_SYNC_PERIOD_S 60UL

#define RTC_TEMP_QUEUE_LENGTH 1
#define RTC_TEMP_QUEUE_ITEM_SIZE sizeof(uint32_t)

QueueHandle_t rtcTempQueueHandle = NULL;
static StaticQueue_t rtcTempQueue;
static uint8_t rtcTempQueueStack[RTC_TEMP_QUEUE_LENGTH * RTC_TEMP_QUEUE_ITEM_SIZE];

/**
 * @brief Reading the temperature using driver functions and adding that temperature to
 * the mailbox temperature queue
 * @return error code
 */
static obc_error_code_t postRtcTempQueue();

void obcTaskInitTimekeeper(void) {
  ASSERT((rtcTempQueueStack != NULL) && (&rtcTempQueue != NULL));
  if (rtcTempQueueHandle == NULL) {
    rtcTempQueueHandle =
        xQueueCreateStatic(RTC_TEMP_QUEUE_LENGTH, RTC_TEMP_QUEUE_ITEM_SIZE, rtcTempQueueStack, &rtcTempQueue);
  }
}

static obc_error_code_t postRtcTempQueue() {
  obc_error_code_t errCode;
  float temp;
  RETURN_IF_ERROR_CODE(getTemperatureRTC(&temp));
  if (xQueueOverwrite(rtcTempQueueHandle, &temp) != pdPASS) {
    return OBC_ERR_CODE_UNKNOWN;
  }
  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t readRTCTemp(float *data) {
  if (xQueuePeek(rtcTempQueueHandle, data, 0) != pdPASS) {
    return OBC_ERR_CODE_QUEUE_EMPTY;
  }
  return OBC_ERR_CODE_SUCCESS;
}

void obcTaskFunctionTimekeeper(void *pvParameters) {
  /*
   * The timekeeper is a task instead of a FreeRTOS timer because we have more control
   * over its priority relative to other tasks (inc. the time service daemon). The sync period is
   * often enough (and the MCU clock speed is high enough) that we don't need to
   * worry about significant drift.
   *
   * The I2C clock speed is high enough that, if another task is completing an I2C transaction
   * when the timekeeper task is performing a local RTC sync, the timekeeper task should not have to
   * wait a significant amount of time.
   */

  obc_error_code_t errCode;
  obc_time_persist_data_t unixTime;
  uint8_t syncPeriodCounter = 0;  // Sync whenever this counter is 0

  while (1) {
    if (syncPeriodCounter == 0) {
      // Sync the local time with the RTC every LOCAL_TIME_SYNC_PERIOD_S seconds.
      // TODO: Deal with errors
#ifdef CONFIG_DS3232
      LOG_IF_ERROR_CODE(syncUnixTime());
#endif  // DCONFIG_DS3232
    } else {
      vPortEnterCritical();
      incrementCurrentUnixTime();
      incrementCurrentDateTime();
      vPortExitCritical();
    }

    // Post temperature into mailbox queue

    postRtcTempQueue();

    // Send Unix time to frame
    unixTime.unixTime = getCurrentUnixTime();
    LOG_IF_ERROR_CODE(
        setPersistentData(OBC_PERSIST_SECTION_ID_OBC_TIME, (uint8_t *)&unixTime, sizeof(obc_time_persist_data_t)));
    syncPeriodCounter = (syncPeriodCounter + 1) % LOCAL_TIME_SYNC_PERIOD_S;
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
