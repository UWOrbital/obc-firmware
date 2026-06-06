#include "obc_i2c_io.h"
#include "obc_sci_io.h"
#include "obc_logging.h"
#include "ds3232_mz.h"
#include "obc_print.h"

#include <FreeRTOS.h>
#include <os_task.h>

#include <sys_common.h>
#include <sci.h>
#include <i2c.h>

static StaticTask_t taskBuffer;
static StackType_t taskStack[1024];

void vTask1(void *pvParameters) {
  obc_error_code_t errCode;

  rtc_date_time_t rtcDateTime;
  rtcDateTime.date.year = 23;  // 2023
  rtcDateTime.date.month = 4;
  rtcDateTime.date.date = 1;
  rtcDateTime.time.hours = 7;
  rtcDateTime.time.minutes = 30;
  rtcDateTime.time.seconds = 0;

  // Set current date and time
  LOG_IF_ERROR_CODE(rtcInit(&rtcDateTime));

  // Get the temperature
  float temperature;
  LOG_IF_ERROR_CODE(getTemperatureRTC(&temperature));
  sciPrintf("RTC Temperature: %f", temperature);

  // Enable interrupt on alarm 1
  rtc_control_t control;
  LOG_IF_ERROR_CODE(getControlRTC(&control));
  sciPrintf("RTC Control: EOSC-%u BBSQW-%u CONV-%u A1IE-%u A2IE-%u INTCN-%u", control.EOSC, control.BBSQW, control.CONV,
            control.A1IE, control.A2IE, control.INTCN);

  control.A1IE = 1;
  control.INTCN = 1;

  LOG_IF_ERROR_CODE(setControlRTC(&control));

  // Set alarm 1
  rtc_alarm1_mode_t alarmMode = RTC_ALARM1_MATCH_SECONDS;
  rtc_alarm_time_t alarm1 = {0};

  // GIOA_0 will have a pulse whenever seconds == 30
  // Use scope to see the pulse
  alarm1.time.seconds = 30;
  alarm1.time.minutes = 0;
  alarm1.time.hours = 0;
  alarm1.date = 1;

  LOG_IF_ERROR_CODE(setAlarm1RTC(alarmMode, alarm1));

  while (1) {
    rtc_date_time_t newDateTime;
    LOG_IF_ERROR_CODE(getCurrentDateTimeRTC(&newDateTime));
    sciPrintf("RTC Date: %u/%u/%u", newDateTime.date.month, newDateTime.date.date, newDateTime.date.year);
    sciPrintf("RTC Time: %u:%u:%u", newDateTime.time.hours, newDateTime.time.minutes, newDateTime.time.seconds);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

int main(void) {
  sciInit();
  i2cInit();

  initSciMutex();
  initI2CMutex();

  initLogger();

  LOG_INFO("Starting RTC Demo");

  xTaskCreateStatic(vTask1, "RTC Demo", 1024, NULL, 1, taskStack, &taskBuffer);

  vTaskStartScheduler();

  while (1);
}
