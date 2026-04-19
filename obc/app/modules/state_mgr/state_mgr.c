#include "state_mgr.h"
#include "comms_manager.h"  // for comms_state_t
#include "obc_board_config.h"
#include "obc_errors.h"
#include "obc_logging.h"
#include "obc_reliance_fs.h"
#include "obc_scheduler_config.h"
#include "obc_time.h"

#include "fm25v20a.h"
#include "lm75bd.h"  // TODO: Handle within thermal manager
#include "cc1120_txrx.h"
#include "cc1120.h"
#include "arducam.h"

#include <FreeRTOS.h>
#include <os_portmacro.h>
#include <os_queue.h>
#include <os_task.h>
#include <sys_common.h>

#if defined(DEBUG) && !defined(OBC_REVISION_2)
#include <gio.h>
#endif

extern void *__stack_chk_guard;
#define STACK_CANARY_BYTES 4

/* Supervisor queue config */
#define STATE_MGR_QUEUE_LENGTH 10U
#define STATE_MGR_QUEUE_ITEM_SIZE sizeof(state_mgr_event_t)
#define STATE_MGR_QUEUE_RX_WAIT_PERIOD pdMS_TO_TICKS(100)
#define STATE_MGR_QUEUE_TX_WAIT_PERIOD pdMS_TO_TICKS(10)

static QueueHandle_t stateMgrQueueHandle = NULL;
static StaticQueue_t stateMgrQueue;
static uint8_t stateMgrQueueStack[STATE_MGR_QUEUE_LENGTH * STATE_MGR_QUEUE_ITEM_SIZE];

static comms_state_t commsManagerState = COMMS_STATE_AWAITING_CONN;

/**
 * @brief Send all startup messages from the stateMgr task to other tasks.
 */
static void sendStartupMessages(void);

void obcTaskInitStateMgr(void) {
  ASSERT((stateMgrQueueStack != NULL) && (&stateMgrQueue != NULL));
  if (stateMgrQueueHandle == NULL) {
    stateMgrQueueHandle =
        xQueueCreateStatic(STATE_MGR_QUEUE_LENGTH, STATE_MGR_QUEUE_ITEM_SIZE, stateMgrQueueStack, &stateMgrQueue);
  }
}

obc_error_code_t sendToStateMgrEventQueue(state_mgr_event_t *event) {
  ASSERT(stateMgrQueueHandle != NULL);

  if (event == NULL) return OBC_ERR_CODE_INVALID_ARG;

  if (xQueueSend(stateMgrQueueHandle, (void *)event, STATE_MGR_QUEUE_TX_WAIT_PERIOD) == pdPASS)
    return OBC_ERR_CODE_SUCCESS;

  return OBC_ERR_CODE_QUEUE_FULL;
}

static void sendStartupMessages(void) {}

uint32_t stack_chk_guard_change(void) {
  obc_error_code_t errCode;
  uint32_t newStackGuard = 0;
  for (uint8_t i = 0; i < STACK_CANARY_BYTES; i++) {
    uint8_t randomByte;
    LOG_IF_ERROR_CODE(cc1120Rng(&randomByte));
    if (errCode == OBC_ERR_CODE_SUCCESS) {
      (newStackGuard) = (newStackGuard << 8) | randomByte;
    } else {
      return 0xDEADBEEF;
    }
  }
  return newStackGuard;
}

static inline void __attribute__((no_stack_protector)) construct_stk_chk_guard() {
  if (__stack_chk_guard == (void *)0xDEADBEEF) {
    __stack_chk_guard = (void *)stack_chk_guard_change();
  }
}

void obcTaskFunctionStateMgr(void *pvParameters) {
  obc_error_code_t errCode;

  ASSERT(stateMgrQueueHandle != NULL);

  obcSchedulerInitTask(OBC_SCHEDULER_CONFIG_ID_LOGGER);
  obcSchedulerCreateTask(OBC_SCHEDULER_CONFIG_ID_LOGGER);

  /* Initialize critical peripherals */

#ifdef CONFIG_SDCARD
  LOG_IF_ERROR_CODE(setupFileSystem());  // microSD card (commented out due to bug)
#endif                                   // CONFIG_SDCARD
#ifdef CONFIG_DS3232
  LOG_IF_ERROR_CODE(initTime());  // RTC
#endif                            // CONFIG_DS3232
#ifdef CONFIG_CC1120
  LOG_IF_ERROR_CODE(cc1120Init());
  construct_stk_chk_guard();
#endif

  // TODO add other peripherals
  // - Add Init for OV5642
  // - Add Init for Arducam

  lm75bd_config_t config = {
      .devAddr = LM75BD_OBC_I2C_ADDR,
      .devOperationMode = LM75BD_DEV_OP_MODE_NORMAL,
      .osFaultQueueSize = 2,
      .osPolarity = LM75BD_OS_POL_ACTIVE_LOW,
      .osOperationMode = LM75BD_OS_OP_MODE_COMP,
      .overTempThresholdCelsius = 125.0f,
      .hysteresisThresholdCelsius = 75.0f,
  };

  LOG_IF_ERROR_CODE(lm75bdInit(&config));  // LM75BD temperature sensor (OBC)

  initFRAM();  // FRAM storage (OBC)

  // Initialize the state of each module. This will not start any tasks.
  obcSchedulerInitTask(OBC_SCHEDULER_CONFIG_ID_TIMEKEEPER);
  obcSchedulerInitTask(OBC_SCHEDULER_CONFIG_ID_ALARM_MGR);
  obcSchedulerInitTask(OBC_SCHEDULER_CONFIG_ID_TELEMETRY_MGR);
  obcSchedulerInitTask(OBC_SCHEDULER_CONFIG_ID_COMMAND_MGR);
  obcSchedulerInitTask(OBC_SCHEDULER_CONFIG_ID_COMMS_MGR);
  obcSchedulerInitTask(OBC_SCHEDULER_CONFIG_ID_COMMS_UPLINK_DECODER);
  obcSchedulerInitTask(OBC_SCHEDULER_CONFIG_ID_COMMS_DOWNLINK_ENCODER);
  obcSchedulerInitTask(OBC_SCHEDULER_CONFIG_ID_EPS_MGR);
  obcSchedulerInitTask(OBC_SCHEDULER_CONFIG_ID_PAYLOAD_MGR);
  obcSchedulerInitTask(OBC_SCHEDULER_CONFIG_ID_HEALTH_COLLECTOR);
  obcSchedulerInitTask(OBC_SCHEDULER_CONFIG_ID_GNC_MGR);
#if ENABLE_TASK_STATS_COLLECTOR == 1
  obcSchedulerInitTask(OBC_SCHEDULER_CONFIG_ID_STATS_COLLECTOR);
#endif

  /* Create all tasks*/
  taskENTER_CRITICAL();
  obcSchedulerCreateTask(OBC_SCHEDULER_CONFIG_ID_TIMEKEEPER);
  obcSchedulerCreateTask(OBC_SCHEDULER_CONFIG_ID_ALARM_MGR);
  obcSchedulerCreateTask(OBC_SCHEDULER_CONFIG_ID_TELEMETRY_MGR);
  obcSchedulerCreateTask(OBC_SCHEDULER_CONFIG_ID_COMMAND_MGR);
  obcSchedulerCreateTaskWithArgs(OBC_SCHEDULER_CONFIG_ID_COMMS_MGR, &commsManagerState);
  obcSchedulerCreateTask(OBC_SCHEDULER_CONFIG_ID_COMMS_UPLINK_DECODER);
  obcSchedulerCreateTask(OBC_SCHEDULER_CONFIG_ID_COMMS_DOWNLINK_ENCODER);
  obcSchedulerCreateTask(OBC_SCHEDULER_CONFIG_ID_EPS_MGR);
  obcSchedulerCreateTask(OBC_SCHEDULER_CONFIG_ID_PAYLOAD_MGR);
  obcSchedulerCreateTask(OBC_SCHEDULER_CONFIG_ID_HEALTH_COLLECTOR);
  obcSchedulerCreateTask(OBC_SCHEDULER_CONFIG_ID_GNC_MGR);
#if ENABLE_TASK_STATS_COLLECTOR == 1
  obcSchedulerCreateTask(OBC_SCHEDULER_CONFIG_ID_STATS_COLLECTOR);
#endif
  obcSchedulerCreateTask(OBC_SCHEDULER_CONFIG_ID_DIGITAL_WATCHDOG_MGR);
  taskEXIT_CRITICAL();

  /* Send initial messages to system queues */
  sendStartupMessages();

  while (1) {
    state_mgr_event_t inMsg;

    if (xQueueReceive(stateMgrQueueHandle, &inMsg, STATE_MGR_QUEUE_RX_WAIT_PERIOD) != pdPASS) {
#if defined(DEBUG) && !defined(OBC_REVISION_2)
      gioToggleBit(STATE_MGR_DEBUG_LED_GIO_PORT, STATE_MGR_DEBUG_LED_GIO_BIT);
#endif
      continue;
    }

    switch (inMsg.eventID) {
      default:
        LOG_ERROR_CODE(OBC_ERR_CODE_UNSUPPORTED_EVENT);
    }
  }
}
