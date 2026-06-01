#pragma once

#include "obc_errors.h"
#include "ina230.h"
#include "tca6424.h"
#include "mcp4562.h"

#include <sys_common.h>
#include <stdbool.h>

/* EPS queue config */
#define EPS_MANAGER_QUEUE_LENGTH 10
#define EPS_MANAGER_QUEUE_ITEM_SIZE (sizeof(eps_event_t))
#define EPS_QUEUE_RX_WAIT_PERIOD pdMS_TO_TICKS(100)  // Timeout as the heart beat for task polling. (100ms tick)
#define EPS_QUEUE_TX_WAIT_PERIOD pdMS_TO_TICKS(10)

/* EPS hardware specfications */
#define EPS_DIGI_POT_INIT_VALUE 128  // Start at 128 coz its half the resolution (0->255)
#define EPS_SHUNT_OHMS 0.05          // Same shunt resistor for all board components
#define EPS_LS_MAX_AMPS 1.6
#define EPS_MPPT_MAX_AMPS 0.3
#define EPS_MPPT_COUNT 3

/**
 * @enum	eps_load_switch_id_t
 * @brief	eps load switch ID names.
 *
 * Enum containing all the load switch rails with their respective names.
 */
typedef enum {
  EPS_LS_EPS_3V3 = 0,
  EPS_LS_OBC_5V,
  EPS_LS_ADCS_3V3,
  EPS_LS_CUSTOM_TX_3V3,
  EPS_LS_ADCS_5V,
  EPS_LS_ADCS_8V,
  EPS_LS_PAYLOAD_5V,
  EPS_LS_CUSTOM_TX_5V,
  EPS_LS_BATT_5V,
  EPS_LS_COUNT
} eps_load_switch_id_t;

/**
 * @struct eps_load_switch_t
 * @brief	eps load switches.
 *
 * A struct to hold the load switch pin maps and current sensors.
 */
typedef struct {
  uint8_t enablePin;  // Driving the SIP32408 EN (active high)
  bool hasIna;        // false for rails with no current sensor (BATT_5V)
  uint8_t alertPin;   // Reading the INA230 ALERT line (active low)
  ina230_t ina;       // current sense
} eps_load_switch_t;

/**
 * @struct eps_mppt_t
 * @brief	eps mppt channels.
 *
 * A struct to hold the mppt pin maps, current sensors and digi pot.
 */
typedef struct {
  uint8_t buckEnablePin;       // U12 pin, MPPT_BUCK_EN (active high)
  uint8_t efuseEnablePin;      // U12 pin, MPPT_EFUSE_EN (active high)
  uint8_t efuseFaultPin;       // U12 pin, EFUSE_FLT (active low)
  ina230_t ina;                // Current/power monitor
  mcp4562_t digiPot;           // Digital potentiometer for FB node
  uint16_t currentWiperState;  // Algorithm state: 0 to 255
  float previousPowerState;    // Algorithm state: previous power calculation
} eps_mppt_t;

/**
 * @enum	eps_event_id_t
 * @brief	eps event ID enum.
 *
 * Enum containing all possible event IDs passed to the eps event queue.
 */
typedef enum {
  EPS_MANAGER_NULL_EVENT_ID,
  EPS_MANAGER_CMD_SET_LOAD_SWITCH,
} eps_event_id_t;

/**
 * @struct eps_switch_cmd_t
 * @brief	eps switch command data.
 *
 * A struct to hold the switch command data.
 */
typedef struct {
  uint8_t switchId;  // Should match eps_load_switch_id_t
  bool turnOn;       // true = ON, false = OFF
} eps_switch_cmd_t;

/**
 * @union	eps_event_data_t
 * @brief	eps event data union
 */
typedef union {
  int i;
  float f;
  eps_switch_cmd_t switchCmd;
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

/**
 * for example if the cmd manager wants to turn ON the ADCS 5V rai, we just need to build this message and queue it.
 *
 *     eps_event_t cmd = {
 *         .eventID = EPS_MANAGER_CMD_SET_LOAD_SWITCH,
 *         .data.switchCmd = { .switchId = EPS_LS_ADCS_5V, .turnOn = true }
 *     };
 *     sendToEPSQueue(&cmd);
 */

void obcTaskInitEpsMgr(void);

/**
 * @brief	Send an event to the EPS Manager queue.
 * @param	event	Event to send.
 * @return The error code
 */
obc_error_code_t sendToEPSQueue(eps_event_t *event);
