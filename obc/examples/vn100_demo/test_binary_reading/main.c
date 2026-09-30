#include <FreeRTOS.h>
#include <gio.h>
#include <os_task.h>
#include <sci.h>
#include <sys_common.h>
#include <sys_core.h>

#include "obc_board_config.h"
#include "obc_print.h"
#include "obc_sci_io.h"
#include "vn100.h"

static StaticTask_t taskBuffer;
static StackType_t taskStack[1024];

void vTaskCode(void* pvParameters) {
  initVn100();

  obc_error_code_t errCode;
  vn100_binary_packet_t packet;

  while (1) {
    errCode = vn100ReadBinaryOutputs(&packet);

    if (errCode != OBC_ERR_CODE_SUCCESS) {
      sciPrintf("Error reading from VN100 - %d\r\n", errCode);
    } else {
      sciPrintf("Yaw:   %f | MagX: %f | AccelX: %f | GyroX: %f \r\n", packet.yaw, packet.magX, packet.accelX,
                packet.gyroX);
      sciPrintf("Pitch: %f | MagY: %f | AccelY: %f | GyroY: %f \r\n", packet.pitch, packet.magY, packet.accelY,
                packet.gyroY);
      sciPrintf("Roll:  %f | MagZ: %f | AccelZ: %f | GyroZ: %f \r\n", packet.roll, packet.magZ, packet.accelZ,
                packet.gyroZ);
      sciPrintf("Temperature:  %f | Pressure: %f \r\n", packet.temp, packet.pres);
    }
    // Toggle the LED.
    gioToggleBit(gioPORTB, 1);

    // Simple delay.
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

int main(void) {
  // Run hardware initialization code
  gioInit();
  sciInit();

  _enable_interrupt_();

  // Initialize bus mutexes
  initSciPrint();
  // Assume all tasks are created correctly
  xTaskCreateStatic(vTaskCode, "Demo", 1024, NULL, 1, taskStack, &taskBuffer);

  vTaskStartScheduler();
}
