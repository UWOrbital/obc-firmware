#pragma once

#include <sys_common.h>

#include "obc_errors.h"

/**
 * @enum	state_mgr_event_id_t
 * @brief	State Manager event ID enum.
 *
 * Enum containing all possible event IDs passed to the state_mgr event queue.
 */
typedef enum {
  STATE_MGR_NULL_EVENT_ID,
} state_mgr_event_id_t;

/**
 * @union	state_mgr_event_data_t
 * @brief	State Manager event data union
 */
typedef union {
  int i;
  float f;
} state_mgr_event_data_t;

/**
 * @struct	state_mgr_event_t
 * @brief	State Manager event struct
 *
 * Holds the message data for each event sent/received by the state_mgr manager queue.
 */
typedef struct {
  state_mgr_event_id_t eventID;
  state_mgr_event_data_t data;
} state_mgr_event_t;

/**
 * @brief	Send an event to the state_mgr queue.
 * @param	event	Event to send.
 * @return The error code
 */
obc_error_code_t sendToStateMgrEventQueue(state_mgr_event_t *event);
