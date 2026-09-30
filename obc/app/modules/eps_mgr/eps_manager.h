#pragma once

#include <sys_common.h>

#include "obc_errors.h"

/**
 * @enum	eps_event_id_t
 * @brief	eps event ID enum.
 *
 * Enum containing all possible event IDs passed to the eps event queue.
 */
typedef enum { EPS_MANAGER_NULL_EVENT_ID } eps_event_id_t;

/**
 * @union	eps_event_data_t
 * @brief	eps event data union
 */
typedef union {
  int i;
  float f;
} eps_event_data_t;

/**
 * @struct	eps_event_t
 * @brief	eps event struct
 *
 * Holds the message data for each event sent/received by the eps manager queue.
 */
typedef struct {
  eps_event_id_t eventID;
  eps_event_data_t data;
} eps_event_t;

/* EPS queue config */
#define EPS_MANAGER_QUEUE_LENGTH 10U
#define EPS_MANAGER_QUEUE_ITEM_SIZE sizeof(eps_event_t)
#define EPS_MANAGER_QUEUE_RX_WAIT_PERIOD pdMS_TO_TICKS(10)
#define EPS_MANAGER_QUEUE_TX_WAIT_PERIOD pdMS_TO_TICKS(10)

/**
 * @brief	Send an event to the EPS Manager queue.
 * @param	event	Event to send.
 * @return The error code
 */
obc_error_code_t sendToEPSQueue(eps_event_t *event);
