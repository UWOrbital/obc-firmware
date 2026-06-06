#include "obc_adc.h"
#include "obc_sci_io.h"
#include "obc_errors.h"
#include "obc_print.h"
#include "obc_scheduler_config.h"

#include <FreeRTOS.h>
#include <os_task.h>
#include <sci.h>
#include <adc.h>

#include <sys_common.h>
#include <sys_core.h>

static StaticTask_t taskBuffer;
static StackType_t taskStack[1024];

void vTask1(void *pvParameters) {
  obc_error_code_t errCode;
  sciPrintf("ADC Demo\r\n");
  adc_module_t adc = ADC_MODULE_1;
  adc_group_t group = ADC_GROUP_1;

  initADC();

  while (1) {
    uint16_t readings[16];  // Assuming up to 16 channels for Group1
    float analogVal[16];

    errCode = adcGetGroupData(adc, group, readings, pdMS_TO_TICKS(10));
    if (errCode != OBC_ERR_CODE_SUCCESS) {
      sciPrintf("Error reading voltage: %d\r\n", errCode);
    } else {
      // Loop through the readings (example for 2 channels)
      for (int i = 0; i < 8; i++) {
        errCode = adcDigitalToAnalog(readings[i], (analogVal + i));
        if (errCode != OBC_ERR_CODE_SUCCESS) {
          sciPrintf("Error converting voltage for Channel %d: %d\r\n", i, errCode);
        } else {
          sciPrintf("Digital Reading Ch%d: %u\r\n", i, readings[i]);
          sciPrintf("Voltage (analog) Ch%d:  %f\r\n", i, analogVal[i]);
        }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

int main(void) {
  sciInit();
  initSciPrint();

  xTaskCreateStatic(vTask1, "Demo", 1024, NULL, 1, taskStack, &taskBuffer);

  vTaskStartScheduler();

  while (1);
}
