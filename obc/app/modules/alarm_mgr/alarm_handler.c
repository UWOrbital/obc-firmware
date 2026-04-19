#include "alarm_handler.h"
#include "ds3232_mz.h"
#include "obc_gs_commands_response.h"
#include "obc_gs_fec.h"
#include "obc_scheduler_config.h"
#include "obc_errors.h"
#include "obc_logging.h"
#include "obc_time.h"
#include "obc_time_utils.h"
#include "obc_persistent.h"
#include "obc_assert.h"
#include "command_manager.h"

#include <FreeRTOS.h>
#include <os_task.h>
#include <os_queue.h>
#include <sys_common.h>

#define ALARM_QUEUE_SIZE 24U
#define ALARM_HANDLER_QUEUE_LENGTH 64U
#define ALARM_HANDLER_QUEUE_ITEM_SIZE sizeof(alarm_handler_event_t)
#define ALARM_HANDLER_QUEUE_RX_WAIT_PERIOD pdMS_TO_TICKS(10)
#define ALARM_HANDLER_QUEUE_TX_WAIT_PERIOD pdMS_TO_TICKS(10)

static QueueHandle_t alarmHandlerQueueHandle;
static StaticQueue_t alarmHandlerQueue;
static uint8_t alarmHandlerQueueStack[ALARM_HANDLER_QUEUE_LENGTH * ALARM_HANDLER_QUEUE_ITEM_SIZE];

static alarm_handler_alarm_info_t alarmQueue[ALARM_QUEUE_SIZE];
static size_t numActiveAlarms = 0;

static obc_error_code_t enqueueAlarm(alarm_handler_alarm_info_t alarm, size_t *insertedAlarmIndex);

static obc_error_code_t dequeueAlarm(alarm_handler_alarm_info_t *alarm);

static obc_error_code_t peekEarliestAlarm(alarm_handler_alarm_info_t *alarm);

static void datetimeToAlarmTime(rtc_date_time_t *datetime, rtc_alarm_time_t *alarmTime);

STATIC_ASSERT((ALARM_QUEUE_SIZE <= OBC_PERSISTENT_MAX_SUBINDEX_ALARM),
              "queue size exceeds max number of alarms that can be stored in FRAM");

void obcTaskInitAlarmMgr(void) {
  ASSERT((alarmHandlerQueueStack != NULL) && (&alarmHandlerQueue != NULL));
  alarmHandlerQueueHandle = xQueueCreateStatic(ALARM_HANDLER_QUEUE_LENGTH, ALARM_HANDLER_QUEUE_ITEM_SIZE,
                                               alarmHandlerQueueStack, &alarmHandlerQueue);
}

void obcTaskFunctionAlarmMgr(void *pvParameters) {
  obc_error_code_t errCode;
  uint8_t responseData[CMD_RESPONSE_DATA_MAX_SIZE] = {0};
  uint8_t sendBuffer[RS_DECODED_SIZE] = {0};
  cmd_response_header_t cmdResHeader = {0};
  while (1) {
    alarm_handler_event_t event;
    uint8_t responseDataLen = 0;

    if (xQueueReceive(alarmHandlerQueueHandle, &event, ALARM_HANDLER_QUEUE_RX_WAIT_PERIOD) != pdPASS) {
      continue;
    }

    switch (event.id) {
      case ALARM_HANDLER_NEW_ALARM: {
        size_t insertedAlarmIndex;
        LOG_IF_ERROR_CODE(enqueueAlarm(event.alarmInfo, &insertedAlarmIndex));
        if (errCode != OBC_ERR_CODE_SUCCESS) {
          break;
        }

        if (insertedAlarmIndex != 0) {
          break;
        }

        // If the new alarm is the earliest alarm, set the RTC alarm to it
        rtc_date_time_t alarmDateTime;
        LOG_IF_ERROR_CODE(unixToDatetime(event.alarmInfo.unixTime, &alarmDateTime));
        if (errCode != OBC_ERR_CODE_SUCCESS) {
          break;
        }

        rtc_alarm_time_t alarmTime;
        datetimeToAlarmTime(&alarmDateTime, &alarmTime);
        LOG_IF_ERROR_CODE(setAlarm1RTC(RTC_ALARM1_MATCH_DATE_HOURS_MINUTES_SECONDS, alarmTime));

        break;
      }

      case ALARM_HANDLER_ALARM_TRIGGERED: {
        // Reset alarm flag
        LOG_IF_ERROR_CODE(clearAlarm1RTC());
        if (errCode != OBC_ERR_CODE_SUCCESS) {
          break;
        }

        alarm_handler_alarm_info_t alarm;
        LOG_IF_ERROR_CODE(peekEarliestAlarm(&alarm));
        if (errCode != OBC_ERR_CODE_SUCCESS) {
          break;
        }

        // Local time incremented since timekeeper might
        // not have updated the local time yet
        static const uint32_t tol = 2;  // tolerance of 2 seconds
        uint32_t currTime = getCurrentUnixTime();
        if (currTime + tol < alarm.unixTime) {
          LOG_ERROR_CODE(OBC_ERR_CODE_RTC_ALARM_EARLY);
          break;
        }

        uint32_t timestampThresh = alarm.unixTime;

        // Execute callbacks for all alarms that have triggered
        // I.e. any alarm with a timestamp less than or equal to the first
        // alarm in the queue
        for (size_t i = 0; i < numActiveAlarms; i++) {
          LOG_IF_ERROR_CODE(peekEarliestAlarm(&alarm));
          if (errCode != OBC_ERR_CODE_SUCCESS) {
            break;
          }

          if (alarm.unixTime > timestampThresh) {
            break;
          }

          LOG_IF_ERROR_CODE(dequeueAlarm(&alarm));
          if (errCode != OBC_ERR_CODE_SUCCESS) {
            break;
          }

          switch (alarm.type) {
            case ALARM_TYPE_DEFAULT:
              LOG_IF_ERROR_CODE(alarm.callbackDef.defaultCallback());
              break;
            case ALARM_TYPE_TIME_TAGGED_CMD:
              LOG_IF_ERROR_CODE(alarm.callbackDef.cmdCallback(&alarm.cmdMsg, responseData, &responseDataLen));
              LOG_IF_ERROR_CODE(downlinkCmdResponse(&cmdResHeader, &alarm.cmdMsg, errCode, responseData,
                                                    &responseDataLen, sendBuffer));
              memset(responseData, 0, CMD_RESPONSE_DATA_MAX_SIZE);
              break;
            default:
              LOG_ERROR_CODE(OBC_ERR_CODE_UNSUPPORTED_ALARM_TYPE);
              break;
          }
        }
        break;
      }

      default: {
        LOG_ERROR_CODE(OBC_ERR_CODE_UNSUPPORTED_EVENT);
        continue;
      }
    }
  }
}

obc_error_code_t sendToAlarmHandlerQueue(alarm_handler_event_t *event) {
  if (alarmHandlerQueueHandle == NULL) {
    return OBC_ERR_CODE_INVALID_STATE;
  }

  if (event == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  if (xQueueSend(alarmHandlerQueueHandle, (void *)event, ALARM_HANDLER_QUEUE_TX_WAIT_PERIOD) == pdPASS) {
    return OBC_ERR_CODE_SUCCESS;
  }

  return OBC_ERR_CODE_QUEUE_FULL;
}

static obc_error_code_t enqueueAlarm(alarm_handler_alarm_info_t alarm, size_t *insertedAlarmIndex) {
  if (numActiveAlarms >= ALARM_QUEUE_SIZE) {
    return OBC_ERR_CODE_QUEUE_FULL;
  }

  // Insert the alarm into the queue in order of increasing time
  size_t i = 0;
  while (i < numActiveAlarms && alarmQueue[i].unixTime < alarm.unixTime) {
    i++;
  }

  // Shift all alarms after the new alarm back by one
  for (size_t j = numActiveAlarms; j > i; j--) {
    alarmQueue[j] = alarmQueue[j - 1];
  }

  // Insert the new alarm
  alarmQueue[i] = alarm;
  numActiveAlarms++;

  *insertedAlarmIndex = i;

  return OBC_ERR_CODE_SUCCESS;
}

static obc_error_code_t dequeueAlarm(alarm_handler_alarm_info_t *alarm) {
  if (numActiveAlarms == 0) {
    return OBC_ERR_CODE_QUEUE_EMPTY;
  }

  *alarm = alarmQueue[0];

  // Shift all alarms after the new alarm back by one
  for (size_t j = 0; j < numActiveAlarms - 1; j++) {
    if (j >= ALARM_QUEUE_SIZE - 1) {
      break;
    }

    alarmQueue[j] = alarmQueue[j + 1];
  }

  numActiveAlarms--;

  return OBC_ERR_CODE_SUCCESS;
}

static obc_error_code_t peekEarliestAlarm(alarm_handler_alarm_info_t *alarm) {
  if (numActiveAlarms == 0) {
    return OBC_ERR_CODE_QUEUE_EMPTY;
  }

  *alarm = alarmQueue[0];

  return OBC_ERR_CODE_SUCCESS;
}

void alarmInterruptCallback(void) {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  // Currently, it's fine if the alarm handler task received other events
  // after we send the alarm triggered event.

  alarm_handler_event_t event = {.id = ALARM_HANDLER_ALARM_TRIGGERED};
  if (xQueueSendToFrontFromISR(alarmHandlerQueueHandle, (void *)&event, &xHigherPriorityTaskWoken) != pdTRUE) {
    LOG_ERROR_FROM_ISR("Log queue full.");
  }

  if (xHigherPriorityTaskWoken) {
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }
}

static void datetimeToAlarmTime(rtc_date_time_t *datetime, rtc_alarm_time_t *alarmTime) {
  alarmTime->time.seconds = datetime->time.seconds;
  alarmTime->time.minutes = datetime->time.minutes;
  alarmTime->time.hours = datetime->time.hours;
  alarmTime->date = datetime->date.date;
}
