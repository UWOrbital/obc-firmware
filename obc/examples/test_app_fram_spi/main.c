#include "obc_spi_io.h"
#include "obc_print.h"
#include "fm25v20a.h"

#include <gio.h>
#include <sci.h>
#include <spi.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define UART_MUTEX_BLOCK_TIME portMAX_DELAY

static StaticTask_t taskBuffer;
static StackType_t taskStack[1024];

void vTask1(void *pvParameters) {
  uint8_t chipID[FRAM_ID_LEN];
  char msg[50] = {0};

  initFRAM();

  // Read Manufacture ID
  framReadID(chipID, FRAM_ID_LEN);
  snprintf(msg, 50, "ID:%X %X %X %X %X %X %X %X %X\r\n", chipID[0], chipID[1], chipID[2], chipID[3], chipID[4],
           chipID[5], chipID[6], chipID[7], chipID[8]);
  // Note: This will send through the USB port on the LaunchPad
  sciPrintText((unsigned char *)msg, strlen(msg), UART_MUTEX_BLOCK_TIME);

  // Write 1 byte to 0x31415

  uint32_t addr = 0x31415;
  uint8_t byteData = 0xAB;
  snprintf(msg, 50, "Writting %X to %lX\r\n", byteData, addr);
  sciPrintText((unsigned char *)msg, strlen(msg), UART_MUTEX_BLOCK_TIME);
  framWrite(addr, &byteData, 1);

  // Read 1 byte from 0x31415
  addr = 0x31415;
  byteData = 0;
  framRead(addr, &byteData, 1);
  snprintf(msg, 50, "Read %X from %lX\r\n", byteData, addr);
  sciPrintText((unsigned char *)msg, strlen(msg), UART_MUTEX_BLOCK_TIME);

  // Multipe Bytes
  addr = 0x12345;
  unsigned char hello_world[12] = {'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd'};
  snprintf(msg, 50, "Writting %s to %lX\r\n", hello_world, addr);
  sciPrintText((unsigned char *)msg, strlen(msg), UART_MUTEX_BLOCK_TIME);
  // Write Hello World to 0x12345
  framWrite(addr, hello_world, sizeof(hello_world));

  // Read Hello World from 0x12345
  memset(hello_world, 0, sizeof(hello_world));
  framRead(addr, hello_world, sizeof(hello_world));
  snprintf(msg, 50, "Read %s from %lX\r\n", hello_world, addr);
  sciPrintText((unsigned char *)msg, strlen(msg), UART_MUTEX_BLOCK_TIME);

  // Read Status Register
  framReadStatusReg(&byteData);
  snprintf(msg, 50, "Status Register: %X\r\n", byteData);
  sciPrintText((unsigned char *)msg, strlen(msg), UART_MUTEX_BLOCK_TIME);

  uint8_t oldStatusReg = byteData;
  byteData = 0b00001100;
  snprintf(msg, 50, "Writting %X to Status Register\r\n", byteData);
  sciPrintText((unsigned char *)msg, strlen(msg), UART_MUTEX_BLOCK_TIME);
  framWriteStatusReg(byteData);

  // Read Status Register
  framReadStatusReg(&byteData);
  snprintf(msg, 50, "Status Register: %X, Expected: 4C\r\n", byteData);
  sciPrintText((unsigned char *)msg, strlen(msg), UART_MUTEX_BLOCK_TIME);

  // Reset Status Reg
  framWriteStatusReg(oldStatusReg);

  // Sleep
  sciPrintText((unsigned char *)"Going to sleep\r\n", strlen("Going to sleep\r\n"), UART_MUTEX_BLOCK_TIME);
  framSleep();
  if (framRead(addr, hello_world, sizeof(hello_world)) == OBC_ERR_CODE_FRAM_IS_ASLEEP) {
    sciPrintText((unsigned char *)"FRAM is asleep\r\n", strlen("FRAM is asleep\r\n"), UART_MUTEX_BLOCK_TIME);
  } else {
    sciPrintText((unsigned char *)"FRAM is not asleep!\r\n", strlen("FRAM is not asleep!\r\n"), UART_MUTEX_BLOCK_TIME);
  }
  framWakeUp();
  // Read Hello World from 0x1234
  memset(hello_world, 0, sizeof(hello_world));
  framRead(addr, hello_world, sizeof(hello_world));
  snprintf(msg, 50, "Read %s from %lX after wakeup\r\n", hello_world, addr);
  sciPrintText((unsigned char *)msg, strlen(msg), UART_MUTEX_BLOCK_TIME);

  while (1);
}
int main(void) {
  // Initialize hardware.
  gioInit();
  sciInit();
  spiInit();

  // Initialize the SCI mutex.
  initSciPrint();
  initSpiMutex();

  sciPrintf("Starting FRAM Demo\r\n");

  xTaskCreateStatic(vTask1, "FRAMDemo", 1024, NULL, 1, taskStack, &taskBuffer);

  vTaskStartScheduler();

  while (1);

  return 0;
}
