#include "eps_manager.h"
#include "obc_assert.h"
#include "obc_logging.h"

#include <FreeRTOS.h>
#include <os_portmacro.h>
#include <os_queue.h>
#include <os_task.h>

// GPIO expander
static const tca6424a_t ioExpander1 = {.i2cAddress = 0x22};  // For all the load switches
static const tca6424a_t ioExpander2 = {.i2cAddress = 0x23};  // For all the mppt stuff

// load switch array
static eps_load_switch_t loadSwitches[EPS_LS_COUNT] = {
    [EPS_LS_EPS_3V3] = {TCA6424A_PIN_21,
                        true,
                        TCA6424A_PIN_20,
                        {.i2cAddress = 0x40, .rShuntOhms = EPS_SHUNT_OHMS, .maxExpectedAmps = EPS_LS_MAX_AMPS}},
    [EPS_LS_OBC_5V] = {TCA6424A_PIN_03,
                       true,
                       TCA6424A_PIN_02,
                       {.i2cAddress = 0x41, .rShuntOhms = EPS_SHUNT_OHMS, .maxExpectedAmps = EPS_LS_MAX_AMPS}},
    [EPS_LS_ADCS_3V3] = {TCA6424A_PIN_17,
                         true,
                         TCA6424A_PIN_16,
                         {.i2cAddress = 0x42, .rShuntOhms = EPS_SHUNT_OHMS, .maxExpectedAmps = EPS_LS_MAX_AMPS}},
    [EPS_LS_CUSTOM_TX_3V3] = {TCA6424A_PIN_23,
                              true,
                              TCA6424A_PIN_22,
                              {.i2cAddress = 0x43, .rShuntOhms = EPS_SHUNT_OHMS, .maxExpectedAmps = EPS_LS_MAX_AMPS}},
    [EPS_LS_ADCS_5V] = {TCA6424A_PIN_05,
                        true,
                        TCA6424A_PIN_04,
                        {.i2cAddress = 0x44, .rShuntOhms = EPS_SHUNT_OHMS, .maxExpectedAmps = EPS_LS_MAX_AMPS}},
    [EPS_LS_ADCS_8V] = {TCA6424A_PIN_14,
                        true,
                        TCA6424A_PIN_13,
                        {.i2cAddress = 0x45, .rShuntOhms = EPS_SHUNT_OHMS, .maxExpectedAmps = EPS_LS_MAX_AMPS}},
    [EPS_LS_PAYLOAD_5V] = {TCA6424A_PIN_11,
                           true,
                           TCA6424A_PIN_10,
                           {.i2cAddress = 0x46, .rShuntOhms = EPS_SHUNT_OHMS, .maxExpectedAmps = EPS_LS_MAX_AMPS}},
    [EPS_LS_CUSTOM_TX_5V] = {TCA6424A_PIN_07,
                             true,
                             TCA6424A_PIN_06,
                             {.i2cAddress = 0x47, .rShuntOhms = EPS_SHUNT_OHMS, .maxExpectedAmps = EPS_LS_MAX_AMPS}},
    [EPS_LS_BATT_5V] = {TCA6424A_PIN_12, false, 0, {0}},  // load switch only, no current sensor for the BATT_5V line.
};

// MPPT array
static eps_mppt_t mppts[3] = {
    // MPPT 1 (mppt[0])
    {TCA6424A_PIN_14,
     TCA6424A_PIN_15,
     TCA6424A_PIN_16,
     {.i2cAddress = 0x48, .rShuntOhms = EPS_SHUNT_OHMS, .maxExpectedAmps = EPS_MPPT_MAX_AMPS},
     {.i2cAddress = 0x2C},
     EPS_DIGI_POT_INIT_VALUE,
     0.0f},
    // MPPT 2 (mppt[1])
    {TCA6424A_PIN_11,
     TCA6424A_PIN_12,
     TCA6424A_PIN_13,
     {.i2cAddress = 0x49, .rShuntOhms = EPS_SHUNT_OHMS, .maxExpectedAmps = EPS_MPPT_MAX_AMPS},
     {.i2cAddress = 0x2D},
     EPS_DIGI_POT_INIT_VALUE,
     0.0f},
    // MPPT 3 (mppt[2])
    {TCA6424A_PIN_07,
     TCA6424A_PIN_06,
     TCA6424A_PIN_05,
     {.i2cAddress = 0x4A, .rShuntOhms = EPS_SHUNT_OHMS, .maxExpectedAmps = EPS_MPPT_MAX_AMPS},
     {.i2cAddress = 0x2E},
     EPS_DIGI_POT_INIT_VALUE,
     0.0f},
};

// Queue memory
static QueueHandle_t epsQueueHandle = NULL;
static StaticQueue_t epsQueue;
static uint8_t epsQueueStack[EPS_MANAGER_QUEUE_LENGTH * EPS_MANAGER_QUEUE_ITEM_SIZE];

/* Initialise the hardware for EPS subsystem */
static void epsInitHardware(void) {
  obc_error_code_t errCode;

  // Configure all the load switches
  for (uint8_t i = 0; i < EPS_LS_COUNT; i++) {
    // Drive the enable low before switching it to an output and then turn everything on.
    LOG_IF_ERROR_CODE(tca6424aWritePin(&ioExpander1, loadSwitches[i].enablePin, TCA6424A_LEVEL_LOW));
    LOG_IF_ERROR_CODE(tca6424aSetIO(&ioExpander1, loadSwitches[i].enablePin, TCA6424A_IO_OUTPUT));
    LOG_IF_ERROR_CODE(tca6424aWritePin(&ioExpander1, loadSwitches[i].enablePin, TCA6424A_LEVEL_HIGH));

    if (loadSwitches[i].hasIna) {
      LOG_IF_ERROR_CODE(tca6424aSetIO(&ioExpander1, loadSwitches[i].alertPin, TCA6424A_IO_INPUT));
      LOG_IF_ERROR_CODE(ina230Init(&loadSwitches[i].ina, INA230_CONFIG_OBC_DEFAULT));
      LOG_IF_ERROR_CODE(ina230SetOvercurrentAlert(&loadSwitches[i].ina, EPS_LS_MAX_AMPS));
    }
  }

  // Configure all the mppts
  for (uint8_t i = 0; i < EPS_MPPT_COUNT; i++) {
    LOG_IF_ERROR_CODE(tca6424aWritePin(&ioExpander2, mppts[i].buckEnablePin, TCA6424A_LEVEL_LOW));
    LOG_IF_ERROR_CODE(tca6424aSetIO(&ioExpander2, mppts[i].buckEnablePin, TCA6424A_IO_OUTPUT));
    LOG_IF_ERROR_CODE(tca6424aWritePin(&ioExpander2, mppts[i].efuseEnablePin, TCA6424A_LEVEL_LOW));
    LOG_IF_ERROR_CODE(tca6424aSetIO(&ioExpander2, mppts[i].efuseEnablePin, TCA6424A_IO_OUTPUT));
    LOG_IF_ERROR_CODE(tca6424aSetIO(&ioExpander2, mppts[i].efuseFaultPin, TCA6424A_IO_INPUT));
    LOG_IF_ERROR_CODE(mcp4562WriteWiper(&mppts[i].digiPot, mppts[i].currentWiperState));
    LOG_IF_ERROR_CODE(ina230Init(&mppts[i].ina, INA230_CONFIG_OBC_DEFAULT));
    // do we even need this? (//TODO: @panthpatel ask elec team)
    LOG_IF_ERROR_CODE(ina230SetOvercurrentAlert(&mppts[i].ina, EPS_MPPT_MAX_AMPS));
  }
}

/* Poll the expander active-low alert inputs and cut anything that has tripped */
static void epsPollFaults(void) {
  obc_error_code_t errCode;
  tca6424a_level_t level;

  // LS Polling
  for (uint8_t i = 0; i < EPS_LS_COUNT; i++) {
    if (!loadSwitches[i].hasIna) continue;
    // 1. If the alert pin is low (active low), that means the overcurrent tripped
    LOG_IF_ERROR_CODE(tca6424aReadPin(&ioExpander1, loadSwitches[i].alertPin, &level));
    if (errCode == OBC_ERR_CODE_SUCCESS && level == TCA6424A_LEVEL_LOW) {
      // 2. Drive the enable pin low for the respective load
      LOG_IF_ERROR_CODE(tca6424aWritePin(&ioExpander1, loadSwitches[i].enablePin, TCA6424A_LEVEL_LOW));
      LOG_ERROR_CODE(OBC_ERR_CODE_EPS_OVERCURRENT);
    }
  }

  // MPPT Polling
  for (uint8_t i = 0; i < EPS_MPPT_COUNT; i++) {
    // 1. MPPT Alert Checks
    // The MPPT e-fuse trips on overcurrent so cut the buck converter and log it
    // This buck converter might be a step-down on the new revistion
    // @panthpatel - check with the elec team
    LOG_IF_ERROR_CODE(tca6424aReadPin(&ioExpander2, mppts[i].efuseFaultPin, &level));
    if (errCode == OBC_ERR_CODE_SUCCESS && level == TCA6424A_LEVEL_LOW) {
      LOG_IF_ERROR_CODE(tca6424aWritePin(&ioExpander2, mppts[i].buckEnablePin, TCA6424A_LEVEL_LOW));
      LOG_ERROR_CODE(OBC_ERR_CODE_EPS_OVERCURRENT);
    }

    // 2. Read Telemetry
    float currentPower = 0.0f;
    if (ina230GetPower(&mppts[i].ina, &currentPower) == OBC_ERR_CODE_SUCCESS) {
      // 3. Perturb & Observe Logic
      // This logic should optimize for P = I^2 * R, by moving the resistance using digipot wiper
      float powerDelta = currentPower - mppts[i].previousPowerState;

      if (powerDelta > 0.01f) {  // Added a small threshold to ignore sensor noise
        // As the power went up, we should keep increasing the resistance
        if (mppts[i].currentWiperState < 255) {
          mppts[i].currentWiperState++;
          mcp4562WriteWiper(&mppts[i].digiPot, mppts[i].currentWiperState);
        }
      } else if (powerDelta < -0.01f) {
        // As the power went down, we should decrease the resistance
        if (mppts[i].currentWiperState > 0) {
          mppts[i].currentWiperState--;
          mcp4562WriteWiper(&mppts[i].digiPot, mppts[i].currentWiperState);
        }
      }
      // Save state for the next cycle
      mppts[i].previousPowerState = currentPower;
    }
  }
}

/* Handle an incoming command from the command manager to turn on/off a load switch */
static void epsHandleCommand(const eps_event_t *event) {
  obc_error_code_t errCode;

  switch (event->eventID) {
    case EPS_MANAGER_CMD_SET_LOAD_SWITCH: {
      uint8_t switchId = event->data.switchCmd.switchId;
      bool turnOn = event->data.switchCmd.turnOn;

      if (switchId >= EPS_LS_COUNT) {
        LOG_ERROR_CODE(OBC_ERR_CODE_INVALID_ARG);
        break;
      }

      uint8_t targetPin = loadSwitches[switchId].enablePin;
      tca6424a_level_t level = turnOn ? TCA6424A_LEVEL_HIGH : TCA6424A_LEVEL_LOW;
      LOG_IF_ERROR_CODE(tca6424aWritePin(&ioExpander1, targetPin, level));
      break;
    }

    default:
      LOG_ERROR_CODE(OBC_ERR_CODE_INVALID_ARG);
      break;
  }
}

void obcTaskFunctionEpsMgr(void *pvParameters) {
  ASSERT(epsQueueHandle != NULL);
  epsInitHardware();

  while (1) {
    // Block until a command arrives, or fall through on timeout to poll for faults.
    eps_event_t queueMsg;
    if (xQueueReceive(epsQueueHandle, &queueMsg, EPS_QUEUE_RX_WAIT_PERIOD) == pdPASS) {
      epsHandleCommand(&queueMsg);
    } else {
      epsPollFaults();
    }
  }
}

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

  if (xQueueSend(epsQueueHandle, (void *)event, EPS_QUEUE_TX_WAIT_PERIOD) == pdPASS) {
    return OBC_ERR_CODE_SUCCESS;
  }
  return OBC_ERR_CODE_QUEUE_FULL;
}
