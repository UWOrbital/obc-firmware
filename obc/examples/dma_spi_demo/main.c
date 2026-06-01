#include "obc_logging.h"
#include "obc_sci_io.h"
#include "obc_print.h"
#include "obc_i2c_io.h"
#include "obc_spi_io.h"
#include "sys_dma.h"
#include "mibspi.h"
#include "obc_spi_dma.h"
#include "obc_privilege.h"

#include <FreeRTOS.h>
#include <os_task.h>

#include <stdio.h>
#include <sys_common.h>
#include <sys_core.h>
#include <gio.h>
#include <sci.h>
#include <i2c.h>
#include <spi.h>
#include <can.h>
#include <het.h>
#include <system.h>
#include <stdint.h>

/* example data Pattern configuration */
#define D_SIZE 127

#define UART_MUTEX_BLOCK_TIME portMAX_DELAY

void task(void *pvParameters);

static StackType_t stack[1024];
static StaticTask_t taskBuf;
uint16_t TX_DATA[D_SIZE];       /* transmit buffer in sys ram */
uint16_t RX_DATA[D_SIZE] = {5}; /* receive  buffer in sys ram */

static const spiDAT1_t spiConfig = {.CS_HOLD = false, .WDEL = false, .DFSEL = 1};

/* USER CODE END */

/** @fn void main(void)
 *   @brief Application main function
 *
 */

/* USER CODE BEGIN (2) */
/* USER CODE END */

int main(void) {
  /* USER CODE BEGIN (3) */
  spiInit();
  sciInit();

  dmaEnable();
  initDmaSpiSemaphores();

  initSciPrint();
  initSpiMutex();

  _enable_interrupt_();

  for (uint8_t i = 0; i < D_SIZE; ++i) {
    TX_DATA[i] = i;
  }

  spiEnableLoopback(spiREG1, 0);
  spiDmaInit(spiREG1);

  TaskHandle_t t = xTaskCreateStatic(task, "name", 1024U, NULL, 500, stack, &taskBuf);

  vTaskStartScheduler();

  return 0;
}

void task(void *pvParameters) {
  assertChipSelect(spiPORT1, 0);
  dmaSpiTransmitandReceiveBytes(spiREG1, TX_DATA, RX_DATA, D_SIZE, 1000, 10000);
  deassertChipSelect(spiPORT1, 0);

  char str[10] = {'\0'};
  char spaceStr[] = "            ";

  for (uint8_t i = 0; i < D_SIZE; ++i) {
    sprintf(str, "%u ", RX_DATA[i]);
    sciPrintText((unsigned char *)str, 5, UART_MUTEX_BLOCK_TIME);
  }
  sciPrintText(spaceStr, 20, UART_MUTEX_BLOCK_TIME);

  for (uint8_t i = 0; i < 50; ++i) {
    TX_DATA[i] = 0xff;
  }

  uint16_t RX_DATA_SECOND[51] = {0};

  assertChipSelect(spiPORT1, 0);
  dmaSpiTransmitandReceiveBytes(spiREG1, TX_DATA, RX_DATA_SECOND + 1, 50, 1000, 10000);
  deassertChipSelect(spiPORT1, 0);

  for (uint8_t i = 0; i < 50; ++i) {
    sprintf(str, "%u ", RX_DATA_SECOND[i + 1]);
    sciPrintText((unsigned char *)str, 5, UART_MUTEX_BLOCK_TIME);
  }
  sciPrintText(spaceStr, 20, UART_MUTEX_BLOCK_TIME);

  for (uint8_t i = 0; i < 10; ++i) {
    TX_DATA[i] = 0x55;
  }

  assertChipSelect(spiPORT1, 0);
  dmaSpiTransmitandReceiveBytes(spiREG1, TX_DATA, RX_DATA, 10, 1000, 10000);
  deassertChipSelect(spiPORT1, 0);

  for (uint8_t i = 0; i < 10; ++i) {
    sprintf(str, "%u ", RX_DATA[i]);
    sciPrintText((unsigned char *)str, 5, UART_MUTEX_BLOCK_TIME);
  }
  sciPrintText(spaceStr, 20, UART_MUTEX_BLOCK_TIME);
  for (uint8_t i = 0; i < 10; ++i) {
    TX_DATA[i] = 0x33;
  }

  assertChipSelect(spiPORT1, 0);
  dmaSpiTransmitandReceiveBytes(spiREG1, TX_DATA, RX_DATA, 10, 1000, 10000);
  deassertChipSelect(spiPORT1, 0);

  for (uint8_t i = 0; i < 10; ++i) {
    sprintf(str, "%u ", RX_DATA[i]);
    sciPrintText((unsigned char *)str, 5, UART_MUTEX_BLOCK_TIME);
  }
  sciPrintText(spaceStr, 20, UART_MUTEX_BLOCK_TIME);
  for (uint8_t i = 0; i < 10; ++i) {
    TX_DATA[i] = 0x22;
  }

  assertChipSelect(spiPORT1, 0);
  dmaSpiTransmitandReceiveBytes(spiREG1, TX_DATA, RX_DATA, 10, 1000, 10000);
  deassertChipSelect(spiPORT1, 0);

  for (uint8_t i = 0; i < 10; ++i) {
    sprintf(str, "%u ", RX_DATA[i]);
    sciPrintText((unsigned char *)str, 5, UART_MUTEX_BLOCK_TIME);
  }
  sciPrintText(spaceStr, 20, UART_MUTEX_BLOCK_TIME);
  for (uint8_t i = 0; i < 10; ++i) {
    TX_DATA[i] = 0x11;
  }

  assertChipSelect(spiPORT1, 0);
  dmaSpiTransmitandReceiveBytes(spiREG1, TX_DATA, RX_DATA, 10, 1000, 10000);
  deassertChipSelect(spiPORT1, 0);

  for (uint8_t i = 0; i < 10; ++i) {
    sprintf(str, "%u ", RX_DATA[i]);
    sciPrintText((unsigned char *)str, 5, UART_MUTEX_BLOCK_TIME);
  }
  sciPrintText(spaceStr, 20, UART_MUTEX_BLOCK_TIME);
  for (uint8_t i = 0; i < 10; ++i) {
    TX_DATA[i] = 0x09;
  }

  assertChipSelect(spiPORT1, 0);
  dmaSpiTransmitandReceiveBytes(spiREG1, TX_DATA, RX_DATA, 10, 1000, 10000);
  deassertChipSelect(spiPORT1, 0);

  for (uint8_t i = 0; i < 10; ++i) {
    sprintf(str, "%u ", RX_DATA[i]);
    sciPrintText((unsigned char *)str, 5, UART_MUTEX_BLOCK_TIME);
  }
  sciPrintText(spaceStr, 20, UART_MUTEX_BLOCK_TIME);
  for (uint8_t i = 0; i < 5; ++i) {
    TX_DATA[i] = 0x08;
  }

  assertChipSelect(spiPORT1, 0);
  dmaSpiTransmitandReceiveBytes(spiREG1, TX_DATA, RX_DATA, 5, 1000, 10000);
  deassertChipSelect(spiPORT1, 0);

  for (uint8_t i = 0; i < 5; ++i) {
    sprintf(str, "%u ", RX_DATA[i]);
    sciPrintText((unsigned char *)str, 5, UART_MUTEX_BLOCK_TIME);
  }
  sciPrintText(spaceStr, 20, UART_MUTEX_BLOCK_TIME);
  for (uint8_t i = 0; i < 3; ++i) {
    TX_DATA[i] = 0x07;
  }

  assertChipSelect(spiPORT1, 0);
  dmaSpiTransmitandReceiveBytes(spiREG1, TX_DATA, RX_DATA, 3, 1000, 10000);
  deassertChipSelect(spiPORT1, 0);

  for (uint8_t i = 0; i < 3; ++i) {
    sprintf(str, "%u ", RX_DATA[i]);
    sciPrintText((unsigned char *)str, 5, UART_MUTEX_BLOCK_TIME);
  }
  sciPrintText(spaceStr, 20, UART_MUTEX_BLOCK_TIME);
  while (1);
}
