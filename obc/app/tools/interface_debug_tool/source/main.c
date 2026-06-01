#include "obc_board_config.h"
#include "obc_spi_io.h"
#include "obc_sci_io.h"
#include "obc_print.h"
#include "obc_i2c_io.h"
#include "obc_logging.h"

#include "op_codes.h"

#include "test_sci.h"
#include "test_spi.h"
#include "test_i2c.h"
#include "test_can.h"
#include "test_adc.h"
#include "test_gio.h"
#include "test_lm75bd.h"
#include <FreeRTOS.h>
#include <os_task.h>

#include <sys_common.h>
#include <sci.h>
#include <spi.h>
#include <i2c.h>

#include <string.h>

#define TASK_STACK_SIZE 1024

static StaticTask_t taskBuffer;
static StackType_t taskStack[TASK_STACK_SIZE];

typedef void (*testFunc_t)(void);

#define NUM_COMMANDS_MAX 255  // Each command initiated by a single character

static const testFunc_t testFuncs[NUM_COMMANDS_MAX] = {
    [OP_CODE_SPI_TEST] = testSPI,       [OP_CODE_SCI_TEST] = testSCI, [OP_CODE_I2C_TEST] = testI2C,
    [OP_CODE_CAN_TEST] = testCAN,       [OP_CODE_ADC_TEST] = testADC, [OP_CODE_GIO_TEST] = testGIO,
    [OP_CODE_LM75BD_TEST] = testLm75bd,
};

void utilityCLI(void *pvParameters) {
  sciPrintf("Starting Bringup Utility...\r\n");

  while (1) {
    unsigned char cmdChar;
    sciPrintf("Enter a command: ");
    sciReadBytes(&cmdChar, 1, portMAX_DELAY, portMAX_DELAY, UART_READ_REG);
    sciPrintf("\r\n");

    if (cmdChar >= NUM_COMMANDS_MAX) {
      sciPrintf("Invalid command\r\n");
      continue;
    }

    if (testFuncs[cmdChar] == NULL) {
      sciPrintf("Invalid command\r\n");
      continue;
    }

    testFuncs[cmdChar]();
  }
}

int main(void) {
  sciInit();
  spiInit();
  i2cInit();
  gioInit();

  initSciPrint();
  initSpiMutex();
  initI2CMutex();

  xTaskCreateStatic(utilityCLI, "Bringup Utility", TASK_STACK_SIZE, NULL, 1, taskStack, &taskBuffer);

  vTaskStartScheduler();

  while (1);
}
