#include "telemetry_manager.h"

#include <FreeRTOS.h>
#include <gio.h>
#include <os_portmacro.h>
#include <os_queue.h>
#include <os_semphr.h>
#include <os_task.h>
#include <redposix.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys_common.h>

#include "comms_manager.h"
#include "downlink_encoder.h"
#include "obc_assert.h"
#include "obc_errors.h"
#include "obc_logging.h"
#include "obc_scheduler_config.h"
#include "telemetry_fs_utils.h"

/* Telemetry data queue config */
#define TELEMETRY_DATA_QUEUE_LENGTH 128U
#define TELEMETRY_DATA_QUEUE_ITEM_SIZE sizeof(telemetry_data_t)
#define TELEMETRY_DATA_QUEUE_WAIT_PERIOD pdMS_TO_TICKS(1000)

#define STARTING_TELEMETRY_BATCH_ID 0UL

#ifdef CONFIG_SDCARD
/**
 * @brief Check if it's time to downlink telemetry.
 * @return bool True if it's time to downlink telemetry, false otherwise
 */
static bool checkDownlinkAlarm(void);
#endif  // CONFIG_SDCARD

// Telemetry Data Queue
static QueueHandle_t telemetryDataQueueHandle = NULL;
static StaticQueue_t telemetryDataQueue;
static uint8_t telemetryDataQueueStack[TELEMETRY_DATA_QUEUE_LENGTH * TELEMETRY_DATA_QUEUE_ITEM_SIZE];

static SemaphoreHandle_t downlinkReady = NULL;
static StaticSemaphore_t downlinkReadyBuffer;

void obcTaskInitTelemetryMgr(void) {
  memset(&telemetryDataQueue, 0, sizeof(telemetryDataQueue));
  memset(&telemetryDataQueueStack, 0, sizeof(telemetryDataQueueStack));

  ASSERT((telemetryDataQueueStack != NULL) && (&telemetryDataQueue != NULL));
  telemetryDataQueueHandle = xQueueCreateStatic(TELEMETRY_DATA_QUEUE_LENGTH, TELEMETRY_DATA_QUEUE_ITEM_SIZE,
                                                telemetryDataQueueStack, &telemetryDataQueue);

  ASSERT(&downlinkReadyBuffer != NULL);
  downlinkReady = xSemaphoreCreateBinaryStatic(&downlinkReadyBuffer);
}

void obcTaskFunctionTelemetryMgr(void *pvParameters) {
#ifdef CONFIG_SDCARD
  obc_error_code_t errCode;

  // TODO: Get batch ID from the FRAM
  uint32_t telemetryBatchId = STARTING_TELEMETRY_BATCH_ID;
  int32_t telemetryFileId = -1;

  // TODO: Deal with errors
  LOG_IF_ERROR_CODE(mkTelemetryDir());

  // TODO: Deal with errors
  LOG_IF_ERROR_CODE(createTelemetryFile(telemetryBatchId, &telemetryFileId));

  while (1) {
    telemetry_data_t telemData;
    if (xQueueReceive(telemetryDataQueueHandle, &telemData, TELEMETRY_DATA_QUEUE_WAIT_PERIOD) == pdPASS) {
      // TODO: Deal with errors
      LOG_IF_ERROR_CODE(openTelemetryFileRW(telemetryBatchId, &telemetryFileId));
      LOG_IF_ERROR_CODE(writeTelemetryToFile(telemetryFileId, telemData));
    }
    // Check if we need to downlink telemetry
    if (!checkDownlinkAlarm()) {
      continue;
    }

    // Important to close the file before sending it to the comms task
    LOG_IF_ERROR_CODE(closeTelemetryFile(telemetryFileId));
    if (errCode != OBC_ERR_CODE_SUCCESS) {
      // TODO: Handle this error
    }

    encode_event_t encodeEvent = {.eventID = DOWNLINK_TELEMETRY_FILE, .telemetryBatchId = telemetryBatchId};

    LOG_IF_ERROR_CODE(sendToDownlinkEncodeQueue(&encodeEvent));
    if (errCode != OBC_ERR_CODE_SUCCESS) {
      // TODO: Handle this error, specifically if the queue is full. Other
      // errors should be caught during testing.
    }

    // The lifetime of the CubeSat should not allow for this to overflow.
    // However, if it does, we can wrap around to 0 and start overwriting old files.
    // telemetryBatchId++;

    // TODO: Save batch ID to FRAM

    // LOG_IF_ERROR_CODE(createAndOpenTelemetryFileRW(telemetryBatchId, &telemetryFileId));
    if (errCode != OBC_ERR_CODE_SUCCESS) {
      // TODO: Deal with errors
    }
  }
#else
  vTaskSuspend(NULL);
#endif  // CONFIG_SDCARD
}

obc_error_code_t addTelemetryData(telemetry_data_t *data) {
  if (telemetryDataQueueHandle == NULL) {
    return OBC_ERR_CODE_INVALID_STATE;
  }

  if (data == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  if (xQueueSend(telemetryDataQueueHandle, (void *)data, TELEMETRY_DATA_QUEUE_WAIT_PERIOD) == pdPASS) {
    return OBC_ERR_CODE_SUCCESS;
  }

  return OBC_ERR_CODE_QUEUE_FULL;
}

#ifdef CONFIG_SDCARD
static bool checkDownlinkAlarm(void) { return xSemaphoreTake(downlinkReady, 0) == pdPASS; }
#endif  // CONFIG_SDCARD

obc_error_code_t setTelemetryManagerDownlinkReady(void) {
  if (xSemaphoreGive(downlinkReady) != pdPASS) {
    return OBC_ERR_CODE_SEMAPHORE_FULL;
  }

  return OBC_ERR_CODE_SUCCESS;
}
