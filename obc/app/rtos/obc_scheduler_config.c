// This code is generated, do not modify directly!
#include "obc_scheduler_config.h"

#include "obc_errors.h"
#include "obc_assert.h"

#include <FreeRTOS.h>
#include <FreeRTOSConfig.h>
#include <os_task.h>
#include <sys_common.h>

#include <stdint.h>

/* DEFINES */
#define OBC_SCHEDULER_MAX_PRIORITY configMAX_PRIORITIES - 1U
#define TASK_IDLE_PRIORITY 0U
#define TASK_COMMS_PRIORITY 2U  // Comms tasks must have the same priority

/* TYPEDEFS */
typedef struct {
  TaskHandle_t *taskHandle;
  StaticTask_t *taskBuffer;
  StackType_t *taskStack;
  uint32_t stackSize;
  uint32_t priority;
  const char *taskName;
  void (*taskFunc)(void *);
  void (*taskInit)(void);  // Optional
} obc_scheduler_config_t;

/* TASK INIT PROTOTYPES - These run before their respective tasks are created */
extern void obcTaskInitStateMgr(void);
extern void obcTaskInitTelemetryMgr(void);
extern void obcTaskInitCommandMgr(void);
extern void obcTaskInitCommsMgr(void);
extern void obcTaskInitCommsDownlinkEncoder(void);
extern void obcTaskInitCommsUplinkDecoder(void);
extern void obcTaskInitEpsMgr(void);
extern void obcTaskInitPayloadMgr(void);
extern void obcTaskInitTimekeeper(void);
extern void obcTaskInitAlarmMgr(void);
extern void obcTaskInitThermalMgr(void);
extern void obcTaskInitStatsCollector(void);
extern void obcTaskInitLogger(void);
extern void obcTaskInitGncMgr(void);

/* TASK FUNCTION PROTOTYPES */
extern void obcTaskFunctionStateMgr(void *params);
extern void obcTaskFunctionTelemetryMgr(void *params);
extern void obcTaskFunctionCommandMgr(void *params);
extern void obcTaskFunctionCommsMgr(void *params);
extern void obcTaskFunctionCommsDownlinkEncoder(void *params);
extern void obcTaskFunctionCommsUplinkDecoder(void *params);
extern void obcTaskFunctionEpsMgr(void *params);
extern void obcTaskFunctionPayloadMgr(void *params);
extern void obcTaskFunctionTimekeeper(void *params);
extern void obcTaskFunctionSwWatchdog(void *params);
extern void obcTaskFunctionAlarmMgr(void *params);
extern void obcTaskFunctionThermalMgr(void *params);
extern void obcTaskFunctionStatsCollector(void *params);
extern void obcTaskFunctionLogger(void *params);
extern void obcTaskFunctionGncMgr(void *params);

/* PRIVATE FUNCTION PROTOTYPES */
static obc_scheduler_config_t *obcSchedulerGetConfig(obc_scheduler_config_id_t taskID);

/* PRIVATE DATA */
static StackType_t obcTaskStackStateMgr[1024U];
static StaticTask_t obcTaskBufferStateMgr;
static StackType_t obcTaskStackTelemetryMgr[1024U];
static StaticTask_t obcTaskBufferTelemetryMgr;
static StackType_t obcTaskStackCommandMgr[1024U];
static StaticTask_t obcTaskBufferCommandMgr;
static StackType_t obcTaskStackCommsMgr[1024U];
static StaticTask_t obcTaskBufferCommsMgr;
static StackType_t obcTaskStackCommsDownlinkEncoder[512U];
static StaticTask_t obcTaskBufferCommsDownlinkEncoder;
static StackType_t obcTaskStackCommsUplinkDecoder[1024U];
static StaticTask_t obcTaskBufferCommsUplinkDecoder;
static StackType_t obcTaskStackEpsMgr[1024U];
static StaticTask_t obcTaskBufferEpsMgr;
static StackType_t obcTaskStackPayloadMgr[1024U];
static StaticTask_t obcTaskBufferPayloadMgr;
static StackType_t obcTaskStackTimekeeper[1024U];
static StaticTask_t obcTaskBufferTimekeeper;
static StackType_t obcTaskStackSwWatchdog[128U];
static StaticTask_t obcTaskBufferSwWatchdog;
static StackType_t obcTaskStackAlarmMgr[512U];
static StaticTask_t obcTaskBufferAlarmMgr;
static StackType_t obcTaskStackThermalMgr[256U];
static StaticTask_t obcTaskBufferThermalMgr;
#if ENABLE_TASK_STATS_COLLECTOR == 1
static StackType_t obcTaskStackStatsCollector[1024U];
static StaticTask_t obcTaskBufferStatsCollector;
#endif
static StackType_t obcTaskStackLogger[512U];
static StaticTask_t obcTaskBufferLogger;
static StackType_t obcTaskStackGncMgr[1024U];
static StaticTask_t obcTaskBufferGncMgr;

static obc_scheduler_config_t obcSchedulerConfig[] = {
    [OBC_SCHEDULER_CONFIG_ID_STATE_MGR] =
        {
            .taskName = "state_mgr",
            .taskStack = obcTaskStackStateMgr,
            .taskBuffer = &obcTaskBufferStateMgr,
            .stackSize = 1024U,
            .priority = 5U,
            .taskFunc = obcTaskFunctionStateMgr,
            .taskInit = obcTaskInitStateMgr,
        },
    [OBC_SCHEDULER_CONFIG_ID_TELEMETRY_MGR] =
        {
            .taskName = "telemetry_mgr",
            .taskStack = obcTaskStackTelemetryMgr,
            .taskBuffer = &obcTaskBufferTelemetryMgr,
            .stackSize = 1024U,
            .priority = 1U,
            .taskFunc = obcTaskFunctionTelemetryMgr,
            .taskInit = obcTaskInitTelemetryMgr,
        },
    [OBC_SCHEDULER_CONFIG_ID_COMMAND_MGR] =
        {
            .taskName = "command_mgr",
            .taskStack = obcTaskStackCommandMgr,
            .taskBuffer = &obcTaskBufferCommandMgr,
            .stackSize = 1024U,
            .priority = 1U,
            .taskFunc = obcTaskFunctionCommandMgr,
            .taskInit = obcTaskInitCommandMgr,
        },
    [OBC_SCHEDULER_CONFIG_ID_COMMS_MGR] =
        {
            .taskName = "comms_mgr",
            .taskStack = obcTaskStackCommsMgr,
            .taskBuffer = &obcTaskBufferCommsMgr,
            .stackSize = 1024U,
            .priority = TASK_COMMS_PRIORITY,
            .taskFunc = obcTaskFunctionCommsMgr,
            .taskInit = obcTaskInitCommsMgr,
        },
    [OBC_SCHEDULER_CONFIG_ID_COMMS_DOWNLINK_ENCODER] =
        {
            .taskName = "comms_encoder",
            .taskStack = obcTaskStackCommsDownlinkEncoder,
            .taskBuffer = &obcTaskBufferCommsDownlinkEncoder,
            .stackSize = 512U,
            .priority = TASK_COMMS_PRIORITY,
            .taskFunc = obcTaskFunctionCommsDownlinkEncoder,
            .taskInit = obcTaskInitCommsDownlinkEncoder,
        },
    [OBC_SCHEDULER_CONFIG_ID_COMMS_UPLINK_DECODER] =
        {
            .taskName = "comms_decoder",
            .taskStack = obcTaskStackCommsUplinkDecoder,
            .taskBuffer = &obcTaskBufferCommsUplinkDecoder,
            .stackSize = 1024U,
            .priority = TASK_COMMS_PRIORITY,
            .taskFunc = obcTaskFunctionCommsUplinkDecoder,
            .taskInit = obcTaskInitCommsUplinkDecoder,
        },
    [OBC_SCHEDULER_CONFIG_ID_EPS_MGR] =
        {
            .taskName = "eps_mgr",
            .taskStack = obcTaskStackEpsMgr,
            .taskBuffer = &obcTaskBufferEpsMgr,
            .stackSize = 1024U,
            .priority = 3U,
            .taskFunc = obcTaskFunctionEpsMgr,
            .taskInit = obcTaskInitEpsMgr,
        },
    [OBC_SCHEDULER_CONFIG_ID_PAYLOAD_MGR] =
        {
            .taskName = "payload_mgr",
            .taskStack = obcTaskStackPayloadMgr,
            .taskBuffer = &obcTaskBufferPayloadMgr,
            .stackSize = 1024U,
            .priority = 1U,
            .taskFunc = obcTaskFunctionPayloadMgr,
            .taskInit = obcTaskInitPayloadMgr,
        },
    [OBC_SCHEDULER_CONFIG_ID_TIMEKEEPER] =
        {
            .taskName = "timekeeper",
            .taskStack = obcTaskStackTimekeeper,
            .taskBuffer = &obcTaskBufferTimekeeper,
            .stackSize = 1024U,
            .priority = 6U,
            .taskFunc = obcTaskFunctionTimekeeper,
            .taskInit = obcTaskInitTimekeeper,
        },
    [OBC_SCHEDULER_CONFIG_ID_DIGITAL_WATCHDOG_MGR] =
        {
            .taskName = "digital_wdg_mgr",
            .taskStack = obcTaskStackSwWatchdog,
            .taskBuffer = &obcTaskBufferSwWatchdog,
            .stackSize = 128U,
            .priority = OBC_SCHEDULER_MAX_PRIORITY,
            .taskFunc = obcTaskFunctionSwWatchdog,
            .taskInit = NULL,
        },
    [OBC_SCHEDULER_CONFIG_ID_ALARM_MGR] =
        {
            .taskName = "alarm_handler",
            .taskStack = obcTaskStackAlarmMgr,
            .taskBuffer = &obcTaskBufferAlarmMgr,
            .stackSize = 512U,
            .priority = 4U,
            .taskFunc = obcTaskFunctionAlarmMgr,
            .taskInit = obcTaskInitAlarmMgr,
        },
    [OBC_SCHEDULER_CONFIG_ID_THERMAL_MGR] =
        {
            .taskName = "thermal_mgr",
            .taskStack = obcTaskStackThermalMgr,
            .taskBuffer = &obcTaskBufferThermalMgr,
            .stackSize = 256U,
            .priority = 1U,
            .taskFunc = obcTaskFunctionThermalMgr,
            .taskInit = obcTaskInitThermalMgr,
        },
#if ENABLE_TASK_STATS_COLLECTOR == 1
    [OBC_SCHEDULER_CONFIG_ID_STATS_COLLECTOR] =
        {
            .taskName = "stats_collector",
            .taskStack = obcTaskStackStatsCollector,
            .taskBuffer = &obcTaskBufferStatsCollector,
            .stackSize = 1024U,
            .priority = 1U,
            .taskFunc = obcTaskFunctionStatsCollector,
            .taskInit = obcTaskInitStatsCollector,
        },
#endif
    [OBC_SCHEDULER_CONFIG_ID_LOGGER] =
        {
            .taskName = "logger",
            .taskStack = obcTaskStackLogger,
            .taskBuffer = &obcTaskBufferLogger,
            .stackSize = 512U,
            .priority = 5U,
            .taskFunc = obcTaskFunctionLogger,
            .taskInit = obcTaskInitLogger,
        },
    [OBC_SCHEDULER_CONFIG_ID_GNC_MGR] =
        {
            .taskName = "gnc_mgr",
            .taskStack = obcTaskStackGncMgr,
            .taskBuffer = &obcTaskBufferGncMgr,
            .stackSize = 1024U,
            .priority = 3U,
            .taskFunc = obcTaskFunctionGncMgr,
            .taskInit = obcTaskInitGncMgr,
        },
};

STATIC_ASSERT_EQ(sizeof(obcSchedulerConfig) / sizeof(obc_scheduler_config_t), OBC_SCHEDULER_TASK_COUNT);

/* PUBLIC FUNCTION DEFINITIONS */
void obcSchedulerCreateTask(obc_scheduler_config_id_t taskID) { obcSchedulerCreateTaskWithArgs(taskID, NULL); }

void obcSchedulerCreateTaskWithArgs(obc_scheduler_config_id_t taskID, void *args) {
  obc_scheduler_config_t *taskConfig = obcSchedulerGetConfig(taskID);
  ASSERT(taskConfig != NULL);

  const bool taskConfigExists =
      (taskConfig->taskFunc != NULL) && (taskConfig->taskStack != NULL) && (taskConfig->taskBuffer != NULL);

  if (taskConfigExists) {
    taskConfig->taskHandle = xTaskCreateStatic(taskConfig->taskFunc, taskConfig->taskName, taskConfig->stackSize, args,
                                               taskConfig->priority, taskConfig->taskStack, taskConfig->taskBuffer);
  }
}

void obcSchedulerInitTask(obc_scheduler_config_id_t taskID) {
  obc_scheduler_config_t *taskConfig = obcSchedulerGetConfig(taskID);

  ASSERT(taskConfig != NULL);

  if (taskConfig->taskInit != NULL) {
    taskConfig->taskInit();
  }
}

/* PRIVATE FUNCTION DEFINITIONS */
static obc_scheduler_config_t *obcSchedulerGetConfig(obc_scheduler_config_id_t taskID) {
  if (taskID >= OBC_SCHEDULER_TASK_COUNT) return NULL;
  return &obcSchedulerConfig[taskID];
}
