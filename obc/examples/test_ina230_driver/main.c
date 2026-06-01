#include "ina230.h"
#include "mcp4562.h"
#include "obc_sci_io.h"
#include "obc_i2c_io.h"
#include "obc_print.h"
#include "obc_board_config.h"
#include "obc_errors.h"

#include <sci.h>
#include <stdio.h>
#include <gio.h>
#include <FreeRTOS.h>
#include <os_task.h>

#include <sys_common.h>
#include <sys_core.h>

static StaticTask_t taskBuffer;
static StackType_t taskStack[1024];

// MPPT Breakout Board Hardware Definitions
#define BREAKOUT_INA_ADDRESS 0x40U
#define BREAKOUT_SHUNT_OHMS 0.05f
#define BREAKOUT_MAX_AMPS 0.3f

#define BREAKOUT_DIGIPOT_ADDRESS 0x2CU

void vTaskCode(void* pvParameters) {
  i2cInit();
  initI2CMutex();

  obc_error_code_t errCode;

  // 1. Hardware Instantiation
  ina230_t sensor = {
      .i2cAddress = BREAKOUT_INA_ADDRESS,
      .rShuntOhms = BREAKOUT_SHUNT_OHMS,
      .maxExpectedAmps = BREAKOUT_MAX_AMPS,
  };
  mcp4562_t digipot = {.i2cAddress = BREAKOUT_DIGIPOT_ADDRESS};

  // MPPT Algorithm Tracking State
  uint16_t currentWiperState = 128;  // Start at 50% resistance
  float previousPower = 0.0f;

  // 2. Hardware Initialization
  errCode = ina230Init(&sensor, INA230_CONFIG_OBC_DEFAULT);
  if (errCode != OBC_ERR_CODE_SUCCESS) {
    sciPrintf("INA230 init failed: %d\r\n", errCode);
  }

  errCode = ina230SetOvercurrentAlert(&sensor, BREAKOUT_MAX_AMPS);
  if (errCode != OBC_ERR_CODE_SUCCESS) {
    sciPrintf("INA230 alert config failed: %d\r\n", errCode);
  }

  errCode = mcp4562WriteWiper(&digipot, currentWiperState);
  if (errCode != OBC_ERR_CODE_SUCCESS) {
    sciPrintf("MCP4562 init failed: %d\r\n", errCode);
  } else {
    sciPrintf("MPPT Breakout Board Initialized. \r\n");
  }

  // 3. The Continuous Execution Loop
  while (1) {
    float busVoltage = 0.0f;
    float current = 0.0f;
    float power = 0.0f;
    bool hasAlerted = false;

    // Read Telemetry
    if (ina230GetBusVoltage(&sensor, &busVoltage) == OBC_ERR_CODE_SUCCESS &&
        ina230GetCurrent(&sensor, &current) == OBC_ERR_CODE_SUCCESS &&
        ina230GetPower(&sensor, &power) == OBC_ERR_CODE_SUCCESS) {
      // Check hardware overcurrent flag (since we don't have the IO Expander to interrupt us)
      if (ina230CheckAlert(&sensor, &hasAlerted) == OBC_ERR_CODE_SUCCESS && hasAlerted) {
        sciPrintf("HARDWARE FAULT: INA230 OVERCURRENT TRIPPED! \r\n");
      }

      // Perturb & Observe
      float powerDelta = power - previousPower;

      if (powerDelta > 0.01f) {  // threshold to ignore sensor noise
        // Power increased! Keep moving the digipot in the same direction.
        if (currentWiperState < 255) {
          currentWiperState++;
          mcp4562WriteWiper(&digipot, currentWiperState);
        }
      } else if (powerDelta < -0.01f) {
        // Power decreased! We passed the peak, reverse direction.
        if (currentWiperState > 0) {
          currentWiperState--;
          mcp4562WriteWiper(&digipot, currentWiperState);
        }
      }

      // Save state for the next cycle
      previousPower = power;

      // Print telemetry to the console to watch the algorithm hunt
      sciPrintf("V: %5.3f V | I: %5.3f A | P: %5.3f W | Wiper Step: %d\r\n", busVoltage, current, power,
                currentWiperState);

    } else {
      sciPrintf("INA230 read failed\r\n");
    }

    // Run the algorithm at 10Hz
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

/*
 * #define LS_TCA_ADDRESS         0x22U
 * #define LS_TEST1_INA_ADDRESS   0x40U
 * #define LS_TEST2_INA_ADDRESS   0x41U
 * #define LS_SHUNT_OHMS          0.05f
 * #define LS_MAX_AMPS            1.6f
 * #define LS_TEST1_ENABLE_PIN    TCA6424A_PIN_00   // EN_3v3_Test1    (confirm against schematic)
 * #define LS_TEST1_ALERT_PIN     TCA6424A_PIN_01   // Alert_3v3_Test1 (confirm against schematic)
 * #define LS_TEST2_ENABLE_PIN    TCA6424A_PIN_02   // EN_3v3_Test2    (confirm against schematic)
 * #define LS_TEST2_ALERT_PIN     TCA6424A_PIN_03   // Alert_3v3_Test2 (confirm against schematic)
 *
 * void vLoadSwitchBreakoutTaskCode(void* pvParameters) {
 *   i2cInit();
 *   initI2CMutex();
 *
 *   const tca6424a_t expander = {.i2cAddress = LS_TCA_ADDRESS};
 *   ina230_t test1 = {.i2cAddress = LS_TEST1_INA_ADDRESS, .rShuntOhms = LS_SHUNT_OHMS, .maxExpectedAmps = LS_MAX_AMPS};
 *   ina230_t test2 = {.i2cAddress = LS_TEST2_INA_ADDRESS, .rShuntOhms = LS_SHUNT_OHMS, .maxExpectedAmps = LS_MAX_AMPS};
 *
 *   // Bring up the sensors and arm their overcurrent alerts (this drives the physical ALERT line).
 *   ina230Init(&test1, INA230_CONFIG_OBC_DEFAULT);
 *   ina230SetOvercurrentAlert(&test1, LS_MAX_AMPS);
 *   ina230Init(&test2, INA230_CONFIG_OBC_DEFAULT);
 *   ina230SetOvercurrentAlert(&test2, LS_MAX_AMPS);
 *
 *   // ALERT lines are expander inputs; EN lines are expander outputs. Turn both rails on to start.
 *   tca6424aSetIO(&expander, LS_TEST1_ALERT_PIN, TCA6424A_IO_INPUT);
 *   tca6424aSetIO(&expander, LS_TEST2_ALERT_PIN, TCA6424A_IO_INPUT);
 *   tca6424aWritePin(&expander, LS_TEST1_ENABLE_PIN, TCA6424A_LEVEL_HIGH);
 *   tca6424aSetIO(&expander, LS_TEST1_ENABLE_PIN, TCA6424A_IO_OUTPUT);
 *   tca6424aWritePin(&expander, LS_TEST2_ENABLE_PIN, TCA6424A_LEVEL_HIGH);
 *   tca6424aSetIO(&expander, LS_TEST2_ENABLE_PIN, TCA6424A_IO_OUTPUT);
 *
 *   while (1) {
 *     tca6424a_level_t alert;
 *
 *     // When an ALERT line reads low the INA230 tripped its overcurrent limit: cut that rail.
 *     if (tca6424aReadPin(&expander, LS_TEST1_ALERT_PIN, &alert) == OBC_ERR_CODE_SUCCESS &&
 *         alert == TCA6424A_LEVEL_LOW) {
 *       sciPrintf("Test1 OVERCURRENT: disabling rail\r\n");
 *       tca6424aWritePin(&expander, LS_TEST1_ENABLE_PIN, TCA6424A_LEVEL_LOW);
 *     }
 *     if (tca6424aReadPin(&expander, LS_TEST2_ALERT_PIN, &alert) == OBC_ERR_CODE_SUCCESS &&
 *         alert == TCA6424A_LEVEL_LOW) {
 *       sciPrintf("Test2 OVERCURRENT: disabling rail\r\n");
 *       tca6424aWritePin(&expander, LS_TEST2_ENABLE_PIN, TCA6424A_LEVEL_LOW);
 *     }
 *
 *     vTaskDelay(pdMS_TO_TICKS(500));
 *   }
 * }
 */

int main(void) {
  // Run hardware initialization code
  gioInit();
  sciInit();

  sciEnableNotification(UART_PRINT_REG, SCI_RX_INT);

  _enable_interrupt_();

  // Initialize bus mutexes
  initSciPrint();

  // Assume all tasks are created correctly
  xTaskCreateStatic(vTaskCode, "MPPT_Demo", 1024, NULL, 1, taskStack, &taskBuffer);

  vTaskStartScheduler();
}
