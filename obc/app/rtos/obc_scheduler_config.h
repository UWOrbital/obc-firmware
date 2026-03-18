// This code is generated, do not modify directly!
#pragma once

#include <stdint.h>

typedef enum {
  OBC_SCHEDULER_CONFIG_ID_STATE_MGR = 0,
  OBC_SCHEDULER_CONFIG_ID_TELEMETRY_MGR,
  OBC_SCHEDULER_CONFIG_ID_COMMAND_MGR,
  OBC_SCHEDULER_CONFIG_ID_COMMS_MGR,
  OBC_SCHEDULER_CONFIG_ID_COMMS_DOWNLINK_ENCODER,
  OBC_SCHEDULER_CONFIG_ID_COMMS_UPLINK_DECODER,
  OBC_SCHEDULER_CONFIG_ID_EPS_MGR,
  OBC_SCHEDULER_CONFIG_ID_PAYLOAD_MGR,
  OBC_SCHEDULER_CONFIG_ID_TIMEKEEPER,
  OBC_SCHEDULER_CONFIG_ID_DIGITAL_WATCHDOG_MGR,
  OBC_SCHEDULER_CONFIG_ID_ALARM_MGR,
  OBC_SCHEDULER_CONFIG_ID_THERMAL_MGR,
#if ENABLE_TASK_STATS_COLLECTOR == 1
  OBC_SCHEDULER_CONFIG_ID_STATS_COLLECTOR,
#endif
  OBC_SCHEDULER_CONFIG_ID_LOGGER,
  OBC_SCHEDULER_CONFIG_ID_GNC_MGR,
  OBC_SCHEDULER_TASK_COUNT
} obc_scheduler_config_id_t;

/**
 * @brief Create a task with the given ID. The task function will be called with
 * no arguments.
 */
void obcSchedulerCreateTask(obc_scheduler_config_id_t taskID);

/**
 * @brief Create a task with the given ID. The task function will be called with
 * the given arguments.
 */
void obcSchedulerCreateTaskWithArgs(obc_scheduler_config_id_t taskID, void *args);

/**
 * @brief Initialize the task with the given ID. This should be called
 * before the task is created.
 */
void obcSchedulerInitTask(obc_scheduler_config_id_t taskID);
