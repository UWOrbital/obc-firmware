#include "comms_manager.h"
#include "cc1120.h"
#include "cc1120_txrx.h"
#include "downlink_encoder.h"
#include "obc_board_config.h"
#include "obc_errors.h"
#include "obc_gs_aes128.h"
#include "obc_gs_ax25.h"
#include "obc_gs_fec.h"
#include "obc_gs_telemetry_pack.h"
#include "obc_logging.h"
#include "obc_privilege.h"
#include "obc_reliance_fs.h"
#include "obc_scheduler_config.h"
#include "os_mpu_wrappers.h"
#include "os_projdefs.h"
#include "reg_sci.h"
#include "rffm6404.h"
#include "sci.h"
#include "telemetry_fs_utils.h"
#include "telemetry_manager.h"
#include "uplink_decoder.h"
#include <stdint.h>

#if COMMS_PHY == COMMS_PHY_UART
#include "obc_sci_io.h"
#endif

#include <FreeRTOS.h>
#include <os_portmacro.h>
#include <os_queue.h>
#include <os_task.h>

#include <redposix.h>

#include <gio.h>
#include <sys_common.h>

#define COMMS_MAX_DOWNLINK_FRAMES 1000U
#define RFFM6404_VAPC_REGULAR_POWER_VAL 1.9f

/* Comms Manager event queue config */
#define COMMS_MANAGER_QUEUE_LENGTH 10U
#define COMMS_MANAGER_QUEUE_ITEM_SIZE sizeof(comms_event_t)
#define COMMS_MANAGER_QUEUE_RX_WAIT_PERIOD pdMS_TO_TICKS(1000)
#define COMMS_MANAGER_QUEUE_TX_WAIT_PERIOD pdMS_TO_TICKS(10)

static QueueHandle_t commsQueueHandle = NULL;
static StaticQueue_t commsQueue;
static uint8_t commsQueueStack[COMMS_MANAGER_QUEUE_LENGTH * COMMS_MANAGER_QUEUE_ITEM_SIZE];

#define CC1120_TRANSMIT_QUEUE_LENGTH 3U
#define CC1120_TRANSMIT_QUEUE_ITEM_SIZE sizeof(transmit_event_t)
#define CC1120_TRANSMIT_QUEUE_RX_WAIT_PERIOD portMAX_DELAY
#define CC1120_TRANSMIT_QUEUE_TX_WAIT_PERIOD portMAX_DELAY

static QueueHandle_t cc1120TransmitQueueHandle = NULL;
static StaticQueue_t cc1120TransmitQueue;
static uint8_t cc1120TransmitQueueStack[CC1120_TRANSMIT_QUEUE_LENGTH * CC1120_TRANSMIT_QUEUE_ITEM_SIZE];

static const uint8_t TEMP_STATIC_KEY[AES_KEY_SIZE] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                                                      0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

#define CC1120_TEMP_QUEUE_LENGTH 1
#define CC1120_TEMP_QUEUE_ITEM_SIZE sizeof(uint32_t)

QueueHandle_t cc1120TempQueueHandle = NULL;
static StaticQueue_t cc1120TempQueue;
static uint8_t cc1120TempQueueStack[COMMS_MANAGER_QUEUE_LENGTH * CC1120_TEMP_QUEUE_ITEM_SIZE];

/**
 * @brief determines what the next Comms Manager state should be and sets it to
 * that state
 *
 * @param event the comms manager event triggering a state transition
 * @param state pointer to comms state variable
 *
 * @return obc_error_code_t - whether or not the state transition was successful
 */
static obc_error_code_t getNextCommsState(comms_event_id_t event, comms_state_t *state);

/* COMMS STATE HANDLER FUNCTIONS START */
static obc_error_code_t handleDisconnectedState(void);

static obc_error_code_t handleAwaitingConnState(void);

static obc_error_code_t handleSendingConnState(void);

static obc_error_code_t handleSendingDiscState(void);

static obc_error_code_t handleSendingAckState(void);

static obc_error_code_t handleAwaitingAckDiscState(void);

static obc_error_code_t handleAwaitingAckConnState(void);

static obc_error_code_t handleUplinkingState(void);

static obc_error_code_t handleDownlinkingState(void);

static obc_error_code_t handleEnterEmergencyState(void);

static obc_error_code_t handleEmergUplinkState(void);
/* COMMS STATE HANDLER FUNCTIONS END */

/**
 * @brief Reading the temperature using driver functions and adding that temperature to
 * the mailbox temperature queue
 * @return error code
 */
static obc_error_code_t postCommsManagerTempQueue();

typedef obc_error_code_t (*comms_state_func_t)(void);

static const comms_state_func_t commsStateFns[] = {
    [COMMS_STATE_DISCONNECTED] = handleDisconnectedState,
    [COMMS_STATE_AWAITING_CONN] = handleAwaitingConnState,
    [COMMS_STATE_SENDING_CONN] = handleSendingConnState,
    [COMMS_STATE_SENDING_DISC] = handleSendingDiscState,
    [COMMS_STATE_SENDING_ACK] = handleSendingAckState,
    [COMMS_STATE_AWAITING_ACK_DISC] = handleAwaitingAckDiscState,
    [COMMS_STATE_AWAITING_ACK_CONN] = handleAwaitingAckConnState,
    [COMMS_STATE_UPLINKING] = handleUplinkingState,
    [COMMS_STATE_DOWNLINKING] = handleDownlinkingState,
    [COMMS_STATE_ENTERING_EMERGENCY] = handleEnterEmergencyState,
    [COMMS_STATE_EMERGENCY_UPLINK] = handleEmergUplinkState
    // Add more functions for other states as needed
};

void obcTaskInitCommsMgr(void) {
  ASSERT((commsQueueStack != NULL) && (&commsQueue != NULL));
  if (commsQueueHandle == NULL) {
    commsQueueHandle =
        xQueueCreateStatic(COMMS_MANAGER_QUEUE_LENGTH, COMMS_MANAGER_QUEUE_ITEM_SIZE, commsQueueStack, &commsQueue);
  }

  ASSERT((cc1120TransmitQueueStack != NULL) && (&cc1120TransmitQueue != NULL))
  if (cc1120TransmitQueueHandle == NULL) {
    cc1120TransmitQueueHandle = xQueueCreateStatic(CC1120_TRANSMIT_QUEUE_LENGTH, CC1120_TRANSMIT_QUEUE_ITEM_SIZE,
                                                   cc1120TransmitQueueStack, &cc1120TransmitQueue);
  }

  ASSERT((cc1120TempQueueStack != NULL) && (&cc1120TempQueue != NULL));
  if (cc1120TempQueueHandle == NULL) {
    cc1120TempQueueHandle = xQueueCreateStatic(CC1120_TEMP_QUEUE_LENGTH, CC1120_TEMP_QUEUE_ITEM_SIZE,
                                               cc1120TempQueueStack, &cc1120TempQueue);
  }

  // TODO: Implement a key exchange algorithm instead of using Pre-Shared/static
  // key
  initializeAesCtx(TEMP_STATIC_KEY);
  initRs();
}

static obc_error_code_t getNextCommsState(comms_event_id_t event, comms_state_t *state) {
  if (state == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }
  switch (*state) {
    case COMMS_STATE_DISCONNECTED:
      switch (event) {
        case COMMS_EVENT_BEGIN_UPLINK:
          *state = COMMS_STATE_AWAITING_CONN;
          return OBC_ERR_CODE_SUCCESS;
        case COMMS_EVENT_ENTER_EMERG:
          *state = COMMS_STATE_ENTERING_EMERGENCY;
          return OBC_ERR_CODE_SUCCESS;
        case COMMS_EVENT_BEGIN_DOWNLINK:
          *state = COMMS_STATE_DOWNLINKING;
          return OBC_ERR_CODE_SUCCESS;
        default:
          return OBC_ERR_CODE_INVALID_STATE_TRANSITION;
      }
    case COMMS_STATE_AWAITING_CONN:
      switch (event) {
        case COMMS_EVENT_CONN_RECEIVED:
          *state = COMMS_STATE_SENDING_ACK;
          return OBC_ERR_CODE_SUCCESS;
        case COMMS_EVENT_ERROR:
          *state = COMMS_STATE_DISCONNECTED;
          return OBC_ERR_CODE_SUCCESS;
        default:
          return OBC_ERR_CODE_INVALID_STATE_TRANSITION;
      }
    case COMMS_STATE_SENDING_CONN:
      switch (event) {
        case COMMS_EVENT_CONN_SENT:
          *state = COMMS_STATE_AWAITING_ACK_CONN;
          return OBC_ERR_CODE_SUCCESS;
        case COMMS_EVENT_ERROR:
          *state = COMMS_STATE_DISCONNECTED;
          return OBC_ERR_CODE_SUCCESS;
        default:
          return OBC_ERR_CODE_INVALID_STATE_TRANSITION;
      }
    case COMMS_STATE_SENDING_DISC:
      switch (event) {
        case COMMS_EVENT_DISC_SENT:
          *state = COMMS_STATE_AWAITING_ACK_DISC;
          return OBC_ERR_CODE_SUCCESS;
        case COMMS_EVENT_ERROR:
          *state = COMMS_STATE_DISCONNECTED;
          return OBC_ERR_CODE_SUCCESS;
        default:
          return OBC_ERR_CODE_INVALID_STATE_TRANSITION;
      }
    case COMMS_STATE_SENDING_ACK:
      switch (event) {
        case COMMS_EVENT_ACK_SENT:
          *state = COMMS_STATE_UPLINKING;
          return OBC_ERR_CODE_SUCCESS;
        case COMMS_EVENT_ERROR:
          *state = COMMS_STATE_DISCONNECTED;
          return OBC_ERR_CODE_SUCCESS;
        default:
          return OBC_ERR_CODE_INVALID_STATE_TRANSITION;
      }
    case COMMS_STATE_AWAITING_ACK_DISC:
      switch (event) {
        case COMMS_EVENT_ACK_RECEIVED:
          *state = COMMS_STATE_DISCONNECTED;
          return OBC_ERR_CODE_SUCCESS;
        case COMMS_EVENT_ERROR:
          *state = COMMS_STATE_DISCONNECTED;
          return OBC_ERR_CODE_SUCCESS;
        default:
          return OBC_ERR_CODE_INVALID_STATE_TRANSITION;
      }
    case COMMS_STATE_AWAITING_ACK_CONN:
      switch (event) {
        case COMMS_EVENT_ACK_RECEIVED:
          *state = COMMS_STATE_EMERGENCY_UPLINK;
          return OBC_ERR_CODE_SUCCESS;
        case COMMS_EVENT_NO_ACK:
          *state = COMMS_STATE_SENDING_CONN;
          return OBC_ERR_CODE_SUCCESS;
        case COMMS_EVENT_ERROR:
          *state = COMMS_STATE_DISCONNECTED;
          return OBC_ERR_CODE_SUCCESS;
        default:
          return OBC_ERR_CODE_INVALID_STATE_TRANSITION;
      }
    case COMMS_STATE_UPLINKING:
      switch (event) {
        case COMMS_EVENT_UPLINK_FINISHED:
          *state = COMMS_STATE_DOWNLINKING;
          return OBC_ERR_CODE_SUCCESS;
        case COMMS_EVENT_START_DISC:
          *state = COMMS_STATE_SENDING_DISC;
          return OBC_ERR_CODE_SUCCESS;
        case COMMS_EVENT_ERROR:
          *state = COMMS_STATE_DISCONNECTED;
          return OBC_ERR_CODE_SUCCESS;
        default:
          return OBC_ERR_CODE_INVALID_STATE_TRANSITION;
      }
    case COMMS_STATE_DOWNLINKING:
      switch (event) {
        case COMMS_EVENT_DOWNLINK_FINISHED:
          *state = COMMS_STATE_UPLINKING;
          return OBC_ERR_CODE_SUCCESS;
        case COMMS_EVENT_ERROR:
          *state = COMMS_STATE_DISCONNECTED;
          return OBC_ERR_CODE_SUCCESS;
        default:
          return OBC_ERR_CODE_INVALID_STATE_TRANSITION;
      }
    case COMMS_STATE_ENTERING_EMERGENCY:
      switch (event) {
        case COMMS_EVENT_EMERG_INITIALIZED:
          *state = COMMS_STATE_SENDING_CONN;
          return OBC_ERR_CODE_SUCCESS;
        case COMMS_EVENT_ERROR:
          *state = COMMS_STATE_DISCONNECTED;
          return OBC_ERR_CODE_SUCCESS;
        default:
          return OBC_ERR_CODE_INVALID_STATE_TRANSITION;
      }
    case COMMS_STATE_EMERGENCY_UPLINK:
      switch (event) {
        case COMMS_EVENT_UPLINK_FINISHED:
          *state = COMMS_STATE_DISCONNECTED;
          return OBC_ERR_CODE_SUCCESS;
        case COMMS_EVENT_ERROR:
          *state = COMMS_STATE_DISCONNECTED;
          return OBC_ERR_CODE_SUCCESS;
        default:
          return OBC_ERR_CODE_INVALID_STATE_TRANSITION;
      }
    default:
      return OBC_ERR_CODE_INVALID_STATE;
  }
}

obc_error_code_t sendToCommsManagerQueue(comms_event_t *event) {
  ASSERT(commsQueueHandle != NULL);

  if (event == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  if (xQueueSend(commsQueueHandle, (void *)event, COMMS_MANAGER_QUEUE_TX_WAIT_PERIOD) == pdPASS) {
    return OBC_ERR_CODE_SUCCESS;
  }

  return OBC_ERR_CODE_QUEUE_FULL;
}

obc_error_code_t sendToFrontCommsManagerQueue(comms_event_t *event) {
  ASSERT(commsQueueHandle != NULL);

  if (event == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  if (xQueueSendToFront(commsQueueHandle, (void *)event, COMMS_MANAGER_QUEUE_TX_WAIT_PERIOD) == pdPASS) {
    return OBC_ERR_CODE_SUCCESS;
  }
  return OBC_ERR_CODE_QUEUE_FULL;
}

static obc_error_code_t postCommsManagerTempQueue() {
  float value = 0.0f;  // dummy value, replace with actual temp reading function
  if (xQueueOverwrite(cc1120TempQueueHandle, &value) != pdPASS) {
    return OBC_ERR_CODE_UNKNOWN;
  }
  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t readCC1120Temp(float *temp) {
  if (xQueuePeek(cc1120TempQueueHandle, temp, pdMS_TO_TICKS(1000)) != pdPASS) {
    return OBC_ERR_CODE_QUEUE_EMPTY;
  }
  return OBC_ERR_CODE_SUCCESS;
}

// NOTE: This is created on startup
void obcTaskFunctionCommsMgr(void *pvParameters) {
  obc_error_code_t errCode;
  comms_state_t commsState = *((comms_state_t *)pvParameters);
  LOG_IF_ERROR_CODE(commsStateFns[commsState]());

  initAllCc1120TxRxSemaphores();

  while (1) {
    comms_event_t queueMsg;

    if (xQueueReceive(commsQueueHandle, &queueMsg, COMMS_MANAGER_QUEUE_RX_WAIT_PERIOD) != pdPASS) {
      postCommsManagerTempQueue();
      continue;
    }

    LOG_IF_ERROR_CODE(getNextCommsState(queueMsg.eventID, &commsState));
    if (errCode != OBC_ERR_CODE_SUCCESS) {
      continue;
    }

    if (commsState >= sizeof(commsStateFns) / sizeof(comms_state_func_t)) {
      LOG_ERROR_CODE(OBC_ERR_CODE_INVALID_STATE);
      commsState = COMMS_STATE_DISCONNECTED;
      continue;
    }

    if (commsStateFns[commsState] == NULL) {
      LOG_ERROR_CODE(OBC_ERR_CODE_INVALID_STATE);
      commsState = COMMS_STATE_DISCONNECTED;
      continue;
    }

    LOG_IF_ERROR_CODE(commsStateFns[commsState]());
    if (errCode != OBC_ERR_CODE_SUCCESS) {
      rffm6404PowerOff();
      comms_event_t event = {.eventID = COMMS_EVENT_ERROR};
      sendToCommsManagerQueue(&event);
    }
  }
}

/**
 * @brief Sends an AX.25 packet to the CC1120 transmit queue
 *
 * @param ax25Pkt - Pointer to the AX.25 packet to send
 * @return obc_error_code_t OBC_ERR_CODE_SUCCESS if the packet was sent to the
 * queue
 */
obc_error_code_t sendToCC1120TransmitQueue(transmit_event_t *event) {
  ASSERT(cc1120TransmitQueueHandle != NULL);

  if (event == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  if (xQueueSend(cc1120TransmitQueueHandle, (void *)event, CC1120_TRANSMIT_QUEUE_TX_WAIT_PERIOD) == pdPASS) {
    return OBC_ERR_CODE_SUCCESS;
  }

  return OBC_ERR_CODE_QUEUE_FULL;
}

static obc_error_code_t handleDisconnectedState(void) {
  obc_error_code_t errCode;
  clearCurrentLinkDestAddress();
  RETURN_IF_ERROR_CODE(rffm6404PowerOff());
  return OBC_ERR_CODE_SUCCESS;
}

static obc_error_code_t handleAwaitingConnState(void) {
  /* Once cc1120Recv is decoupled from decode task this function should be
   * changed to handle this in comms manager and bypass decode task to allow for
   * retries */
  obc_error_code_t errCode;
#if COMMS_PHY == COMMS_PHY_UART
  uint8_t readBytes[AX25_MAXIMUM_U_FRAME_CMD_LENGTH] = {0};
  RETURN_IF_ERROR_CODE(sciReadBytes(readBytes, U_FRAME_COMMS_RECV_SIZE, portMAX_DELAY, portMAX_DELAY, UART_READ_REG));
  for (uint16_t i = 0; i < AX25_MAXIMUM_U_FRAME_CMD_LENGTH; i++) {
    RETURN_IF_ERROR_CODE(sendToDecodeDataQueue(&readBytes[i]));
    if (readBytes[i] == AX25_FLAG && i > 0) {
      break;
    }
  }
#else
  // switch cc1120 to receive mode and start receiving all the bytes for one
  // continuous transmission
  RETURN_IF_ERROR_CODE(rffm6404ActivateRx());
  LOG_IF_ERROR_CODE(cc1120ReceiveToDecodeTask());
  RETURN_IF_ERROR_CODE(cc1120StrobeSpi(CC1120_STROBE_SFSTXON));
#endif
  return OBC_ERR_CODE_SUCCESS;
}

static obc_error_code_t handleSendingConnState(void) {
  packed_ax25_u_frame_t connCmdPkt = {0};
  obc_gs_error_code_t interfaceErr = ax25SendUFrame(&connCmdPkt, U_FRAME_CMD_CONN, 0);
  if (interfaceErr != OBC_GS_ERR_CODE_SUCCESS) {
    return OBC_ERR_CODE_AX25_ENCODE_FAILURE;
  }
  obc_error_code_t errCode;
#if COMMS_PHY == COMMS_PHY_UART
  RETURN_IF_ERROR_CODE(sciSendBytes(connCmdPkt.data, (uint32_t)connCmdPkt.length, portMAX_DELAY, UART_PRINT_REG));
#else
  RETURN_IF_ERROR_CODE(rffm6404ActivateTx(RFFM6404_VAPC_REGULAR_POWER_VAL));
  RETURN_IF_ERROR_CODE(
      cc1120Send(connCmdPkt.data, (uint32_t)connCmdPkt.length, CC1120_TX_FIFO_EMPTY_SEMAPHORE_TIMEOUT));
#endif
  comms_event_t connSentEvent = {.eventID = COMMS_EVENT_CONN_SENT};
  RETURN_IF_ERROR_CODE(sendToCommsManagerQueue(&connSentEvent));
  return OBC_ERR_CODE_SUCCESS;
}

static obc_error_code_t handleSendingDiscState(void) {
  packed_ax25_u_frame_t discCmdPkt = {0};
  obc_gs_error_code_t interfaceErr = ax25SendUFrame(&discCmdPkt, U_FRAME_CMD_DISC, 0);
  if (interfaceErr != OBC_GS_ERR_CODE_SUCCESS) {
    return OBC_ERR_CODE_AX25_ENCODE_FAILURE;
  }
  obc_error_code_t errCode;
#if COMMS_PHY == COMMS_PHY_UART
  RETURN_IF_ERROR_CODE(sciSendBytes(discCmdPkt.data, discCmdPkt.length, portMAX_DELAY, UART_PRINT_REG));
#else
  RETURN_IF_ERROR_CODE(rffm6404ActivateTx(RFFM6404_VAPC_REGULAR_POWER_VAL));
  RETURN_IF_ERROR_CODE(cc1120Send(discCmdPkt.data, discCmdPkt.length, CC1120_TX_FIFO_EMPTY_SEMAPHORE_TIMEOUT));
#endif
  comms_event_t discSentEvent = {.eventID = COMMS_EVENT_DISC_SENT};
  RETURN_IF_ERROR_CODE(sendToCommsManagerQueue(&discSentEvent));
  return OBC_ERR_CODE_SUCCESS;
}

static obc_error_code_t handleSendingAckState(void) {
  packed_ax25_u_frame_t ackCmdPkt = {0};
  obc_gs_error_code_t interfaceErr = ax25SendUFrame(&ackCmdPkt, U_FRAME_CMD_CONN, 0);
  if (interfaceErr != OBC_GS_ERR_CODE_SUCCESS) {
    return OBC_ERR_CODE_AX25_ENCODE_FAILURE;
  }
  obc_error_code_t errCode;

#if COMMS_PHY == COMMS_PHY_UART
  RETURN_IF_ERROR_CODE(sciSendBytes(ackCmdPkt.data, ackCmdPkt.length, portMAX_DELAY, UART_PRINT_REG));
#else
  RETURN_IF_ERROR_CODE(rffm6404ActivateTx(RFFM6404_VAPC_REGULAR_POWER_VAL));
  RETURN_IF_ERROR_CODE(cc1120Send(ackCmdPkt.data, ackCmdPkt.length, CC1120_TX_FIFO_EMPTY_SEMAPHORE_TIMEOUT));
#endif
  comms_event_t ackSentEvent = {.eventID = COMMS_EVENT_ACK_SENT};
  RETURN_IF_ERROR_CODE(sendToCommsManagerQueue(&ackSentEvent));
  return OBC_ERR_CODE_SUCCESS;
}

static obc_error_code_t handleAwaitingAckDiscState(void) {
  /* Once cc1120Recv is decoupled from decode task this function should be
   * changed to handle this in comms manager and bypass decode task to allow
   * for retries */
  obc_error_code_t errCode;
#if COMMS_PHY == COMMS_PHY_UART
  uint8_t rxByte;

  // Read first byte
  RETURN_IF_ERROR_CODE(sciReadBytes(&rxByte, 1, portMAX_DELAY, pdMS_TO_TICKS(1000), UART_READ_REG));

  RETURN_IF_ERROR_CODE(sendToDecodeDataQueue(&rxByte));

  // Read the rest of the bytes until we stop uplinking
  for (uint16_t i = 0; i < AX25_MAXIMUM_PKT_LEN; ++i) {
    RETURN_IF_ERROR_CODE(sciReadBytes(&rxByte, 1, portMAX_DELAY, pdMS_TO_TICKS(10), UART_READ_REG));

    RETURN_IF_ERROR_CODE(sendToDecodeDataQueue(&rxByte));
  }
#else
  // switch cc1120 to receive mode and start receiving all the bytes for one
  // continuous transmission
  RETURN_IF_ERROR_CODE(rffm6404ActivateRx());
  LOG_IF_ERROR_CODE(cc1120ReceiveToDecodeTask());
  RETURN_IF_ERROR_CODE(cc1120StrobeSpi(CC1120_STROBE_SFSTXON));
#endif
  return OBC_ERR_CODE_SUCCESS;
}

static obc_error_code_t handleAwaitingAckConnState(void) {
  /* Once cc1120Recv is decoupled from decode task this function should be
   * changed to handle this in comms manager and bypass decode task to allow
   * for retries */
  obc_error_code_t errCode;
#if COMMS_PHY == COMMS_PHY_UART
  uint8_t rxByte;

  // Read first byte
  RETURN_IF_ERROR_CODE(sciReadBytes(&rxByte, 1, portMAX_DELAY, pdMS_TO_TICKS(1000), UART_READ_REG));

  RETURN_IF_ERROR_CODE(sendToDecodeDataQueue(&rxByte));

  // Read the rest of the bytes until we stop uplinking
  for (uint16_t i = 0; i < AX25_MAXIMUM_PKT_LEN; ++i) {
    RETURN_IF_ERROR_CODE(sciReadBytes(&rxByte, 1, portMAX_DELAY, pdMS_TO_TICKS(10), UART_READ_REG));

    RETURN_IF_ERROR_CODE(sendToDecodeDataQueue(&rxByte));
  }
#else
  // switch cc1120 to receive mode and start receiving all the bytes for one
  // continuous transmission
  RETURN_IF_ERROR_CODE(rffm6404ActivateRx());
  LOG_IF_ERROR_CODE(cc1120ReceiveToDecodeTask());
  RETURN_IF_ERROR_CODE(cc1120StrobeSpi(CC1120_STROBE_SFSTXON));
#endif
  return OBC_ERR_CODE_SUCCESS;
}

static obc_error_code_t handleUplinkingState(void) {
  obc_error_code_t errCode;
#if COMMS_PHY == COMMS_PHY_UART
  uint8_t readBytes[AX25_MAXIMUM_PKT_LEN] = {0};
  RETURN_IF_ERROR_CODE(sciReadBytes(readBytes, I_FRAME_COMMS_RECV_SIZE, portMAX_DELAY, portMAX_DELAY, UART_READ_REG));
  uint16_t i;
  for (i = 0; i < AX25_MAXIMUM_PKT_LEN; i++) {
    RETURN_IF_ERROR_CODE(sendToDecodeDataQueue(&readBytes[i]));
    if (readBytes[i] == AX25_FLAG && i > 0) {
      break;
    }
  }
#else
  // switch cc1120 to receive mode and start receiving all the bytes for one
  // continuous transmission
  LOG_IF_ERROR_CODE(cc1120ReceiveToDecodeTask());
  RETURN_IF_ERROR_CODE(cc1120StrobeSpi(CC1120_STROBE_SFSTXON));
#endif
  comms_event_t uplinkFinishedEvent = {.eventID = COMMS_EVENT_UPLINK_FINISHED};
  RETURN_IF_ERROR_CODE(sendToCommsManagerQueue(&uplinkFinishedEvent));
  return OBC_ERR_CODE_SUCCESS;
}

static obc_error_code_t handleDownlinkingState(void) {
  obc_error_code_t errCode;
#if COMMS_PHY != COMMS_PHY_UART
  RETURN_IF_ERROR_CODE(rffm6404ActivateTx(RFFM6404_VAPC_REGULAR_POWER_VAL));
#endif
  for (uint16_t i = 0; i < COMMS_MAX_DOWNLINK_FRAMES; ++i) {
    transmit_event_t transmitEvent;
    // poll the transmit queue
    if (xQueueReceive(cc1120TransmitQueueHandle, &transmitEvent, CC1120_TRANSMIT_QUEUE_RX_WAIT_PERIOD) != pdPASS) {
      LOG_ERROR_CODE(OBC_ERR_CODE_QUEUE_EMPTY);
    }
    if (transmitEvent.eventID == DOWNLINK_PACKET) {
#if COMMS_PHY == COMMS_PHY_UART
      RETURN_IF_ERROR_CODE(sciSendBytes((uint8_t *)transmitEvent.ax25Pkt.data, transmitEvent.ax25Pkt.length,
                                        portMAX_DELAY, UART_PRINT_REG));
#else
      RETURN_IF_ERROR_CODE(cc1120Send((uint8_t *)transmitEvent.ax25Pkt.data, transmitEvent.ax25Pkt.length,
                                      CC1120_TX_FIFO_EMPTY_SEMAPHORE_TIMEOUT));
#endif
    } else if (transmitEvent.eventID == END_DOWNLINK) {
      break;
    } else {
      LOG_ERROR_CODE(OBC_ERR_CODE_UNSUPPORTED_EVENT);
    }
  }
  comms_event_t finishedDownlinkEvent = {.eventID = COMMS_EVENT_DOWNLINK_FINISHED};
  RETURN_IF_ERROR_CODE(sendToCommsManagerQueue(&finishedDownlinkEvent));
  return OBC_ERR_CODE_SUCCESS;
}

static obc_error_code_t handleEnterEmergencyState(void) {
  obc_error_code_t errCode;
  RETURN_IF_ERROR_CODE(cc1120Init());
  comms_event_t emergInitializedEvent = {.eventID = COMMS_EVENT_EMERG_INITIALIZED};
  RETURN_IF_ERROR_CODE(sendToCommsManagerQueue(&emergInitializedEvent));
  return OBC_ERR_CODE_SUCCESS;
}

static obc_error_code_t handleEmergUplinkState(void) {
  // TODO: Fill this in
  return OBC_ERR_CODE_SUCCESS;
}
