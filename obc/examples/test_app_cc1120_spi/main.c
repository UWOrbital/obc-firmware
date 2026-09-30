#include <gio.h>
#include <sci.h>
#include <spi.h>
#include <sys_common.h>

#include "FreeRTOS.h"
#include "cc1120_spi_tests.h"
#include "obc_logging.h"
#include "obc_sci_io.h"
#include "obc_spi_io.h"
#include "os_portable.h"
#include "os_task.h"

static TaskHandle_t testTaskHandle = NULL;
static StaticTask_t testTaskBuffer;
static StackType_t testTaskStack[1024U];

void initTestTask(void);
static void vTestTask(void* pvParameters);

void initTestTask(void) {
  ASSERT((testTaskStack != NULL) && (&testTaskBuffer != NULL));
  if (testTaskHandle == NULL) {
    testTaskHandle = xTaskCreateStatic(vTestTask, "test task", 1024U, NULL, 1U, testTaskStack, &testTaskBuffer);
  }
}

static void vTestTask(void* pvParameters) {
  // Run the E2E SPI read test
  cc1120TestSpiRead();
  while (1) {
  }
}

int main(void) {
  gioInit();
  sciInit();
  spiInit();

  // Initialize logger
  // initLogger();
  // logSetLevel(LOG_DEBUG);

  // Initialize bus mutexes
  initSciMutex();
  initSpiMutex();

  initTestTask();

  vTaskStartScheduler();
}
