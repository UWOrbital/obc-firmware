#include "obc_spi_io.h"
#include "obc_print.h"
#include "obc_gs_fec.h"

#include <gio.h>
#include <sci.h>
#include <spi.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static StaticTask_t taskBuffer;
static StackType_t taskStack[1024];

int vTask1(void *pvParameters) {
  initRs();

  packed_telem_packet_t telemData = {.data = {'-'}};

  char *testData = "Hello world!";
  for (int i = 0; i < 13; i++) {
    telemData.data[i] = (uint8_t)testData[i];
  }

  sciPrintf("Original data: %s\r\n", telemData.data);

  packed_rs_packet_t rsData = {.data = {0}};

  rsEncode(&telemData.data, &rsData);
  sciPrintf("Encoded data: %s\r\n", rsData.data);
  rsData.data[0] = 'a';  // Simulate a bit flip
  sciPrintf("Corrupted data: %s\r\n", rsData.data);

  uint8_t decodedData[RS_DECODED_SIZE] = {0};
  rsDecode(&rsData, decodedData, (uint8_t)RS_ENCODED_SIZE);
  sciPrintf("Decoded data: %s\r\n", decodedData);

  destroyRs();
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

  sciPrintf("Starting RS Demo\r\n");

  xTaskCreateStatic(vTask1, "RSDemo", 1024, NULL, 1, taskStack, &taskBuffer);

  vTaskStartScheduler();

  while (1);

  return 0;
}
