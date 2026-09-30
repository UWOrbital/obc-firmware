#include "eps_manager.h"

#include <FreeRTOS.h>
#include <gio.h>
#include <os_portmacro.h>
#include <os_queue.h>
#include <os_task.h>
#include <sys_common.h>

#include "obc_scheduler_config.h"

static QueueHandle_t epsQueueHandle = NULL;
static StaticQueue_t epsQueue;
static uint8_t epsQueueStack[EPS_MANAGER_QUEUE_LENGTH * EPS_MANAGER_QUEUE_ITEM_SIZE];

void obcTaskInitEpsMgr(void) {
  ASSERT((epsQueueStack != NULL) && (&epsQueue != NULL));
  if (epsQueueHandle == NULL) {
    epsQueueHandle =
        xQueueCreateStatic(EPS_MANAGER_QUEUE_LENGTH, EPS_MANAGER_QUEUE_ITEM_SIZE, epsQueueStack, &epsQueue);
  }
}

obc_error_code_t sendToEPSQueue(eps_event_t *event) {
  ASSERT(epsQueueHandle != NULL);

  if (event == NULL) return OBC_ERR_CODE_INVALID_ARG;

  if (xQueueSend(epsQueueHandle, (void *)event, EPS_MANAGER_QUEUE_TX_WAIT_PERIOD) == pdPASS)
    return OBC_ERR_CODE_SUCCESS;

  return OBC_ERR_CODE_QUEUE_FULL;
}

void obcTaskFunctionEpsMgr(void *pvParameters) {
  ASSERT(epsQueueHandle != NULL);

  while (1) {
    eps_event_t queueMsg;
    if (xQueueReceive(epsQueueHandle, &queueMsg, EPS_MANAGER_QUEUE_RX_WAIT_PERIOD) != pdPASS)
      queueMsg.eventID = EPS_MANAGER_NULL_EVENT_ID;

    switch (queueMsg.eventID) {
      case EPS_MANAGER_NULL_EVENT_ID:
        break;
    }
  }
}
