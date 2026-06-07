#include "digital_watchdog_mgr.h"
#include "obc_digital_watchdog.h"
#include "obc_privilege.h"
#include "obc_scheduler_config.h"
#include "obc_logging.h"

#include <system.h>

#include <stdint.h>

// Must feed the watchdog before the timeout period expires
// This value should provide some margin
// Should be atleast twice as fast as the fastest timeout
#define FEEDING_PERIOD pdMS_TO_TICKS(50)

/* Task timeouts for watchdog */
#define TASK_STATE_MGR_WATCHDOG_TIMEOUT portMAX_DELAY
#define TASK_TELEMETRY_MGR_WATCHDOG_TIMEOUT portMAX_DELAY
#define TASK_COMMAND_MGR_WATCHDOG_TIMEOUT portMAX_DELAY
#define TASK_COMMS_MGR_WATCHDOG_TIMEOUT portMAX_DELAY
#define TASK_COMMS_DOWNLINK_ENCODER_WATCHDOG_TIMEOUT portMAX_DELAY
#define TASK_COMMS_UPLINK_DECODER_WATCHDOG_TIMEOUT portMAX_DELAY
#define TASK_EPS_MGR_WATCHDOG_TIMEOUT portMAX_DELAY
#define TASK_PAYLOAD_MGR_WATCHDOG_TIMEOUT portMAX_DELAY
#define TASK_TIMEKEEPER_WATCHDOG_TIMEOUT portMAX_DELAY
#define TASK_ALARM_MGR_WATCHDOG_TIMEOUT portMAX_DELAY
#define TASK_THERMAL_MGR_WATCHDOG_TIMEOUT portMAX_DELAY
#define TASK_STATS_COLLECTOR_WATCHDOG_TIMEOUT portMAX_DELAY
#define TASK_LOGGER_WATCHDOG_TIMEOUT portMAX_DELAY
#define TASK_DIGITAL_WATCHDOG_MGR_WATCHDOG_TIMEOUT portMAX_DELAY
#define TASK_GNC_MGR_WATCHDOG_TIMEOUT pdMS_TO_TICKS(100)

typedef struct {
  uint32_t taskTimeoutTicks;
  uint32_t taskLastCheckInTick;
} watchdog_task_info_t;

static watchdog_task_info_t watchdogTaskArray[] = {
    [OBC_SCHEDULER_CONFIG_ID_STATE_MGR] =
        {
            .taskTimeoutTicks = TASK_STATE_MGR_WATCHDOG_TIMEOUT,
        },
    [OBC_SCHEDULER_CONFIG_ID_TELEMETRY_MGR] =
        {
            .taskTimeoutTicks = TASK_TELEMETRY_MGR_WATCHDOG_TIMEOUT,
        },
    [OBC_SCHEDULER_CONFIG_ID_COMMAND_MGR] =
        {
            .taskTimeoutTicks = TASK_COMMAND_MGR_WATCHDOG_TIMEOUT,
        },
    [OBC_SCHEDULER_CONFIG_ID_COMMS_MGR] =
        {
            .taskTimeoutTicks = TASK_COMMS_MGR_WATCHDOG_TIMEOUT,
        },
    [OBC_SCHEDULER_CONFIG_ID_COMMS_DOWNLINK_ENCODER] =
        {
            .taskTimeoutTicks = TASK_COMMS_DOWNLINK_ENCODER_WATCHDOG_TIMEOUT,
        },
    [OBC_SCHEDULER_CONFIG_ID_COMMS_UPLINK_DECODER] =
        {
            .taskTimeoutTicks = TASK_COMMS_UPLINK_DECODER_WATCHDOG_TIMEOUT,
        },
    [OBC_SCHEDULER_CONFIG_ID_EPS_MGR] =
        {
            .taskTimeoutTicks = TASK_EPS_MGR_WATCHDOG_TIMEOUT,
        },
    [OBC_SCHEDULER_CONFIG_ID_PAYLOAD_MGR] =
        {
            .taskTimeoutTicks = TASK_PAYLOAD_MGR_WATCHDOG_TIMEOUT,
        },
    [OBC_SCHEDULER_CONFIG_ID_TIMEKEEPER] =
        {
            .taskTimeoutTicks = TASK_TIMEKEEPER_WATCHDOG_TIMEOUT,
        },
    [OBC_SCHEDULER_CONFIG_ID_ALARM_MGR] =
        {
            .taskTimeoutTicks = TASK_ALARM_MGR_WATCHDOG_TIMEOUT,
        },
    [OBC_SCHEDULER_CONFIG_ID_THERMAL_MGR] =
        {
            .taskTimeoutTicks = TASK_THERMAL_MGR_WATCHDOG_TIMEOUT,
        },
    [OBC_SCHEDULER_CONFIG_ID_LOGGER] =
        {
            .taskTimeoutTicks = TASK_LOGGER_WATCHDOG_TIMEOUT,
        },
    [OBC_SCHEDULER_CONFIG_ID_GNC_MGR] =
        {
            .taskTimeoutTicks = TASK_GNC_MGR_WATCHDOG_TIMEOUT,
        },

#if ENABLE_TASK_STATS_COLLECTOR == 1
    [OBC_SCHEDULER_CONFIG_ID_STATS_COLLECTOR] =
        {
            .taskTimeoutTicks = TASK_STATS_COLLECTOR_WATCHDOG_TIMEOUT,
        },
#endif
    [OBC_SCHEDULER_CONFIG_ID_DIGITAL_WATCHDOG_MGR] =
        {
            .taskTimeoutTicks = TASK_DIGITAL_WATCHDOG_MGR_WATCHDOG_TIMEOUT,
        },
};

void obcTaskFunctionSwWatchdog(void *params) {
  // Set up the watchdog
  prvRaisePrivilege();

  initDigitalWatchdog();

  // initialize all tasks as checked in
  for (uint8_t i = 0; i < OBC_SCHEDULER_TASK_COUNT; i++) {
    digitalWatchdogTaskCheckIn(i);
  }

  while (1) {
    bool allTasksCheckedIn = true;
    TickType_t currentTick = xTaskGetTickCount();
    uint8_t i;
    for (i = 0; i < OBC_SCHEDULER_TASK_COUNT; i++) {
      TickType_t ticksSinceLastCheckin = (currentTick >= watchdogTaskArray[i].taskLastCheckInTick)
                                             ? (currentTick - watchdogTaskArray[i].taskLastCheckInTick)
                                             : ((UINT32_MAX - watchdogTaskArray[i].taskLastCheckInTick) + currentTick +
                                                1);  // Calculate the tick between last checkin and current tick

      // The task does not respond after timeout period
      if (ticksSinceLastCheckin > watchdogTaskArray[i].taskTimeoutTicks) {
        allTasksCheckedIn = false;
        break;
      }
    }
    if (allTasksCheckedIn) {
      feedDigitalWatchdog();
    } else {
      LOG_ERROR_CODE(DIGITAL_WATCHDOG_ERROR_CODE_OFFSET + i);
      vTaskSuspend(NULL);  // suspend this task and wait for reset
    }
    vTaskDelay(FEEDING_PERIOD);
  }
}

void digitalWatchdogTaskCheckIn(obc_scheduler_config_id_t taskNum) {
  watchdogTaskArray[taskNum].taskLastCheckInTick = xTaskGetTickCount();
}
