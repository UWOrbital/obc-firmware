#include "obc_sci_io.h"
#include "obc_i2c_io.h"
#include "obc_errors.h"
#include "obc_print.h"
#include "lm75bd.h"

#include <FreeRTOS.h>
#include <os_task.h>

#include <sys_common.h>
#include <sci.h>
#include <spi.h>
#include <i2c.h>

static StaticTask_t taskBuffer;
static StackType_t taskStack[1024];

void vTask1(void *pvParameters) {
  obc_error_code_t errCode;
  sciPrintf("LM75BD Demo\r\n");

  lm75bd_config_t config = {
      .devAddr = LM75BD_OBC_I2C_ADDR,
      .devOperationMode = LM75BD_DEV_OP_MODE_NORMAL,
      .osFaultQueueSize = 2,
      .osPolarity = LM75BD_OS_POL_ACTIVE_LOW,
      .osOperationMode = LM75BD_OS_OP_MODE_COMP,
      .overTempThresholdCelsius = 100.0f,
      .hysteresisThresholdCelsius = 50.0f,
  };

  errCode = lm75bdInit(&config);
  if (errCode != OBC_ERR_CODE_SUCCESS) {
    sciPrintf("Error initializing LM75BD: %d\r\n", errCode);
  }

  while (1) {
    float temp;
    errCode = readTempLM75BD(LM75BD_OBC_I2C_ADDR, &temp);
    if (errCode != OBC_ERR_CODE_SUCCESS) {
      sciPrintf("Error reading temperature: %d\r\n", errCode);
    } else {
      sciPrintf("Temperature: %f\r\n", temp);
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

int main(void) {
  sciInit();
  i2cInit();

  initSciPrint();
  initI2CMutex();

  xTaskCreateStatic(vTask1, "Demo", 1024, NULL, 1, taskStack, &taskBuffer);

  vTaskStartScheduler();

  while (1);
}
