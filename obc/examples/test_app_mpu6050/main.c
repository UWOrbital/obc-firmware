#include "obc_sci_io.h"
#include "obc_i2c_io.h"
#include "mpu6050.h"

#include <FreeRTOS.h>
#include <os_portmacro.h>
#include <os_task.h>
#include <sys_common.h>

#include <sci.h>
#include <i2c.h>

void taskA(void* pvParameters);
void taskB(void* pvParameters);
void taskC(void* pvParameters);
void taskD(void* pvParameters);

int main(void) {
  // run hardware initialization code (TODO: refactor all this into one function call)
  sciInit();
  i2cInit();

  initSciMutex();
  initI2CMutex();

  xTaskHandle taskHandleA, taskHandleB, taskHandleC, taskHandleD;

  // Assume all tasks are created correctly
  xTaskCreate(taskA, "taskA", 1024, NULL, 1, &taskHandleA);
  xTaskCreate(taskB, "taskB", 1024, NULL, 1, &taskHandleB);
  xTaskCreate(taskC, "taskC", 1024, NULL, 1, &taskHandleC);
  xTaskCreate(taskD, "taskD", 1024, NULL, 1, &taskHandleD);

  // start task scheduler
  vTaskStartScheduler();
}

void taskA(void* pvParameters) {
  wakeupMPU6050();
  while (1) {
    double accX, accY, accZ;
    readAccelDataMPU6050(&accX, &accY, &accZ);
    vTaskDelay(2000);
  }
}

void taskB(void* pvParameters) {
  while (1) {
    for (volatile int i = 0; i < 100000; i++);
    vTaskDelay(1000);
  }
}

void taskC(void* pvParameters) {
  while (1) {
    for (volatile int i = 0; i < 100000; i++);
    vTaskDelay(2000);
  }
}

void taskD(void* pvParameters) {
  while (1) {
    for (volatile int i = 0; i < 100000; i++);
    vTaskDelay(500);
  }
}
