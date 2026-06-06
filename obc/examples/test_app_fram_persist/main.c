#include "obc_sci_io.h"
#include "obc_print.h"
#include "obc_spi_io.h"
#include "obc_errors.h"
#include "obc_persistent.h"
#include "fm25v20a.h"

#include <FreeRTOS.h>
#include <os_task.h>

#include <sys_common.h>
#include <sci.h>
#include <spi.h>

static StaticTask_t taskBuffer;
static StackType_t taskStack[1024];

static StaticTask_t taskBuffer2;
static StackType_t taskStack2[1024];

void vTask1(void *pvParameters) {
  obc_error_code_t errCode;
  sciPrintf("Persist Demo\r\n");

  obc_time_persist_data_t timeData = {0};
  timeData.unixTime = 0x12345678;

  errCode = setPersistentData(OBC_PERSIST_SECTION_ID_OBC_TIME, &timeData, sizeof(obc_time_persist_data_t));
  if (errCode != OBC_ERR_CODE_SUCCESS) {
    sciPrintf("Error setting time data: %d\r\n", errCode);
  }

  obc_time_persist_data_t readTimeData = {0};
  errCode = getPersistentData(OBC_PERSIST_SECTION_ID_OBC_TIME, &readTimeData, sizeof(obc_time_persist_data_t));
  if (errCode != OBC_ERR_CODE_SUCCESS) {
    sciPrintf("Error getting time data: %d\r\n", errCode);
  } else {
    sciPrintf("Time data: %x\r\n", readTimeData.unixTime);
  }

  sciPrintf("Corrupting FRAM\r\n");

  // Corrupt time data (As of 2023-05-28, address 0x9 is in the obc_time section of FRAM)
  uint8_t corrupt = 0xFF;
  uint32_t unixTimeAddr = OBC_PERSIST_ADDR_OF(obcTime.data);
  framWrite(unixTimeAddr, &corrupt, 1);

  errCode = getPersistentData(OBC_PERSIST_SECTION_ID_OBC_TIME, &readTimeData, sizeof(obc_time_persist_data_t));
  if (errCode != OBC_ERR_CODE_SUCCESS) {
    if (errCode == OBC_ERR_CODE_PERSISTENT_CORRUPTED) {
      sciPrintf("FRAM is corrupt\r\n");
    } else {
      sciPrintf("Error getting time data: %d\r\n", errCode);
    }
  } else {
    sciPrintf("Time data: %x\r\n", readTimeData.unixTime);
  }

  while (1);
}

void vTask2(void *pvParameters) {
  obc_error_code_t errCode;
  sciPrintf("Alarm Persist Test\r\n");

  alarm_mgr_persist_data_t alarmData = {0};
  for (unsigned int i = 0; i < OBC_PERSISTENT_MAX_SUBINDEX_ALARM; ++i) {
    alarmData.unixTime = i;
    errCode =
        setPersistentDataByIndex(OBC_PERSIST_SECTION_ID_ALARM_MGR, i, &alarmData, sizeof(alarm_mgr_persist_data_t));
    if (errCode != OBC_ERR_CODE_SUCCESS) {
      sciPrintf("Error setting alarm data for alarm ID (%d): %d\r\n", i, errCode);
    }
  }

  alarm_mgr_persist_data_t readAlarmData = {0};
  for (unsigned int i = 0; i < OBC_PERSISTENT_MAX_SUBINDEX_ALARM; ++i) {
    errCode =
        getPersistentDataByIndex(OBC_PERSIST_SECTION_ID_ALARM_MGR, i, &readAlarmData, sizeof(alarm_mgr_persist_data_t));
    if (errCode != OBC_ERR_CODE_SUCCESS) {
      sciPrintf("Error getting alarm data for alarm ID(%d): %d\r\n", i, errCode);
    } else {
      sciPrintf("Alarm data for alarm ID(%d): %x\r\n", i, readAlarmData.unixTime);
    }
  }

  sciPrintf("Corrupting FRAM\r\n");

  // Corrupt all of the alarm's unixTimes
  for (unsigned int i = 0; i < OBC_PERSISTENT_MAX_SUBINDEX_ALARM; ++i) {
    uint32_t corrupt = 0xFFFF;
    // unixTimeAddr is calculated the same way as in the set/get persistent by sub index
    //  but outside of testing SHOULD NOT BE USED, use the provided functions
    uint32_t unixTimeAddr = OBC_PERSIST_ADDR_OF(alarmMgr[0].data) + sizeof(alarm_mgr_persist_t) * i;
    framWrite(unixTimeAddr, &corrupt, sizeof(uint32_t));
  }

  // Read out all the data for the alarms
  for (unsigned int i = 0; i < OBC_PERSISTENT_MAX_SUBINDEX_ALARM; ++i) {
    alarm_mgr_persist_data_t alarmOut = {0};
    errCode =
        getPersistentDataByIndex(OBC_PERSIST_SECTION_ID_ALARM_MGR, i, &alarmOut, sizeof(alarm_mgr_persist_data_t));
    if (errCode != OBC_ERR_CODE_SUCCESS) {
      if (errCode == OBC_ERR_CODE_PERSISTENT_CORRUPTED) {
        sciPrintf("FRAM is corrupt\r\n");
      } else {
        sciPrintf("Error getting alarm data: %d\r\n", errCode);
      }
    } else {
      sciPrintf("Alarm data for alarm ID(%d): %x\r\n", readAlarmData.unixTime);
    }
  }

  while (1);
}

int main(void) {
  sciInit();
  spiInit();

  initSciPrint();
  initSpiMutex();

  xTaskCreateStatic(vTask1, "Demo", 1024, NULL, 1, taskStack, &taskBuffer);

  xTaskCreateStatic(vTask2, "AlarmPersist", 1024, NULL, 1, taskStack2, &taskBuffer2);

  vTaskStartScheduler();

  while (1);
}
