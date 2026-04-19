#include "obc_logging.h"
#include "obc_errors.h"
#include "obc_sci_io.h"
#include "obc_spi_io.h"
#include "obc_i2c_io.h"
#include "obc_print.h"

#include "arducam.h"
#include "camera_control.h"

#include <gio.h>
#include <sci.h>
#include <spi.h>
#include <i2c.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define UART_MUTEX_BLOCK_TIME portMAX_DELAY
// BUFFER_SIZE must be a multiple of 3 for base64 conversion
#define BUFFER_SIZE 4095U

#define TASK_STACK_SIZE 2048U
static StaticTask_t taskBuffer;
static StackType_t taskStack[TASK_STACK_SIZE];

const char b64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void vTask1(void *pvParameters) {
  obc_error_code_t errCode;
  sciPrintf("Starting Arducam Demo\r\n");
  camera_id_t selectedCamera = PRIMARY;
  LOG_IF_ERROR_CODE(initCamera(selectedCamera));
  uint8_t temp;
  LOG_IF_ERROR_CODE(arducamReadSensorPowerControlReg(selectedCamera, &temp));
  sciPrintf("Power Control Reg:0x%X\r\n", temp);

  // Read Camera Sensor ID
  uint16_t cam_id;
  LOG_IF_ERROR_CODE(ov5642GetChipID(&cam_id));
  sciPrintf("Sensor ID: %X\r\n", cam_id);

  // Test Reg operations
  uint8_t byte = 0x55;
  sciPrintf("Writing %d to test reg\r\n", byte);
  LOG_IF_ERROR_CODE(arducamWriteTestReg(selectedCamera, byte));
  byte = 0;
  LOG_IF_ERROR_CODE(arducamReadTestReg(selectedCamera, &byte));
  sciPrintf("Read %d from test reg\r\n", byte);

  // Camera Configuration
  sciPrintf("Configuring Camera\r\n");
  LOG_IF_ERROR_CODE(camConfigureSensor());
  for (int i = 0; i < 20; i++) {
    // Capture
    sciPrintf("Starting Image Capture\r\n");
    LOG_IF_ERROR_CODE(startImageCapture(selectedCamera));
    sciPrintf("Image Capture Started\r\n");

    while (isCaptureDone(selectedCamera) == OBC_ERR_CODE_CAMERA_CAPTURE_INCOMPLETE)
      ;
    sciPrintf("Image Capture Done ^_^\r\n");

    // Read image size
    uint32_t img_len = 0;
    LOG_IF_ERROR_CODE(arducamReadFIFOSize(selectedCamera, &img_len));
    sciPrintf("image len: %d \r\n", img_len);

    // Read image from FIFO
    uint8_t imgBuffer[BUFFER_SIZE];
    sciPrintf("FIFO Burst Read:\r\n");
    size_t bytesRead = 0;
    // Only print last image
    if (i == 19) {
      obc_error_code_t ret;
      do {
        ret = readImage(selectedCamera, imgBuffer, BUFFER_SIZE, &bytesRead);
        for (size_t i = 0; i < bytesRead; i += 3) {
          // modified from https://nachtimwald.com/2017/11/18/base64-encode-and-decode-in-c/
          uint32_t v = imgBuffer[i];
          v = i + 1 < bytesRead ? v << 8 | imgBuffer[i + 1] : v << 8;
          v = i + 2 < bytesRead ? v << 8 | imgBuffer[i + 2] : v << 8;

          sciPrintf("%c", b64chars[(v >> 18) & 0x3F]);
          sciPrintf("%c", b64chars[(v >> 12) & 0x3F]);
          if (i + 1 < bytesRead) {
            sciPrintf("%c", b64chars[(v >> 6) & 0x3F]);
          } else {
            sciPrintf("=");
          }
          if (i + 2 < bytesRead) {
            sciPrintf("%c", b64chars[v & 0x3F]);
          } else {
            sciPrintf("=");
          }
        }
      } while (ret == OBC_ERR_CODE_CAMERA_IMAGE_READ_INCOMPLETE);
    }
    sciPrintf("\r\n");
  }

  // Put Camera on standby (gets pretty hot if left powered on for too long)
  LOG_IF_ERROR_CODE(standbyCamera(selectedCamera));
  while (1)
    ;
}
int main(void) {
  // Initialize hardware.
  gioInit();
  sciInit();
  spiInit();
  i2cInit();

  // Initialize the bus mutex.
  initSciPrint();
  initSpiMutex();
  initI2CMutex();

  initOV5642();

  xTaskCreateStatic(vTask1, "Arducam", TASK_STACK_SIZE, NULL, 1, taskStack, &taskBuffer);

  vTaskStartScheduler();

  while (1)
    ;

  return 0;
}
