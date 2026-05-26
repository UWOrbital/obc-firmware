#include "obc_persistent.h"

#include "obc_errors.h"
#include "fm25v20a.h"

// Test subjects, add more as persistent grows
#include "obc_time_utils.h"
#include "alarm_handler.h"

#include <gtest/gtest.h>

TEST(TestOBCPersistent, InvalidArgs) {
  obc_time_persist_data_t timeData = {0};
  timeData.unixTime = 0x12345678;

  // Test setPersistentData
  EXPECT_EQ(setPersistentData(OBC_PERSIST_SECTION_ID_OBC_TIME, nullptr, sizeof(obc_time_persist_data_t)),
            OBC_ERR_CODE_INVALID_ARG);
  EXPECT_EQ(setPersistentData(OBC_PERSIST_SECTION_ID_COUNT, &timeData, sizeof(obc_time_persist_data_t)),
            OBC_ERR_CODE_INVALID_ARG);
  EXPECT_EQ(setPersistentData(OBC_PERSIST_SECTION_ID_OBC_TIME, &timeData, 0), OBC_ERR_CODE_BUFF_TOO_SMALL);

  // Test getPersistentData
  EXPECT_EQ(getPersistentData(OBC_PERSIST_SECTION_ID_OBC_TIME, nullptr, sizeof(obc_time_persist_data_t)),
            OBC_ERR_CODE_INVALID_ARG);
  EXPECT_EQ(getPersistentData(OBC_PERSIST_SECTION_ID_COUNT, &timeData, sizeof(obc_time_persist_data_t)),
            OBC_ERR_CODE_INVALID_ARG);
  EXPECT_EQ(getPersistentData(OBC_PERSIST_SECTION_ID_OBC_TIME, &timeData, 0), OBC_ERR_CODE_BUFF_TOO_SMALL);

  // Test getPersistentDataByIndex
  EXPECT_EQ(getPersistentDataByIndex(OBC_PERSIST_SECTION_ID_OBC_TIME, 0, nullptr, sizeof(obc_time_persist_data_t)),
            OBC_ERR_CODE_INVALID_ARG);
  EXPECT_EQ(getPersistentDataByIndex(OBC_PERSIST_SECTION_ID_OBC_TIME, 1, &timeData, sizeof(obc_time_persist_data_t)),
            OBC_ERR_CODE_INVALID_ARG);  // Only 1 index
  EXPECT_EQ(getPersistentDataByIndex(OBC_PERSIST_SECTION_ID_COUNT, 0, &timeData, sizeof(obc_time_persist_data_t)),
            OBC_ERR_CODE_INVALID_ARG);
  EXPECT_EQ(getPersistentDataByIndex(OBC_PERSIST_SECTION_ID_OBC_TIME, 0, &timeData, 0), OBC_ERR_CODE_BUFF_TOO_SMALL);

  // Test setPersistentDataByIndex
  EXPECT_EQ(setPersistentDataByIndex(OBC_PERSIST_SECTION_ID_OBC_TIME, 0, nullptr, sizeof(obc_time_persist_data_t)),
            OBC_ERR_CODE_INVALID_ARG);
  EXPECT_EQ(setPersistentDataByIndex(OBC_PERSIST_SECTION_ID_OBC_TIME, 1, &timeData, sizeof(obc_time_persist_data_t)),
            OBC_ERR_CODE_INVALID_ARG);  // Only 1 index
  EXPECT_EQ(setPersistentDataByIndex(OBC_PERSIST_SECTION_ID_COUNT, 0, &timeData, sizeof(obc_time_persist_data_t)),
            OBC_ERR_CODE_INVALID_ARG);
  EXPECT_EQ(setPersistentDataByIndex(OBC_PERSIST_SECTION_ID_OBC_TIME, 0, &timeData, 0), OBC_ERR_CODE_BUFF_TOO_SMALL);
}

TEST(TestOBCPersistent, ValidOBCTime) {
  const uint32_t UNIX_TTIME = 0x12345678;
  obc_time_persist_data_t timeData = {0};
  timeData.unixTime = UNIX_TTIME;

  // Test setPersistentData
  ASSERT_EQ(setPersistentData(OBC_PERSIST_SECTION_ID_OBC_TIME, &timeData, sizeof(obc_time_persist_data_t)),
            OBC_ERR_CODE_SUCCESS);

  timeData.unixTime = 0x00;

  // Test getPersistentData
  obc_time_persist_data_t timeData2 = {0};
  ASSERT_EQ(getPersistentData(OBC_PERSIST_SECTION_ID_OBC_TIME, &timeData2, sizeof(obc_time_persist_data_t)),
            OBC_ERR_CODE_SUCCESS);

  EXPECT_EQ(UNIX_TTIME, timeData2.unixTime);
}

TEST(TestOBCPersistent, CorruptOBCTime) {
  obc_time_persist_data_t timeData = {0};
  timeData.unixTime = 0x12345678;

  // Test setPersistentData
  ASSERT_EQ(setPersistentData(OBC_PERSIST_SECTION_ID_OBC_TIME, &timeData, sizeof(obc_time_persist_data_t)),
            OBC_ERR_CODE_SUCCESS);

  // Corrupt the data
  size_t data = 0x00;
  ASSERT_EQ(framWrite(0, (uint8_t *)&data, sizeof(size_t)), OBC_ERR_CODE_SUCCESS);

  // Test getPersistentData
  EXPECT_EQ(getPersistentData(OBC_PERSIST_SECTION_ID_OBC_TIME, &timeData, sizeof(obc_time_persist_data_t)),
            OBC_ERR_CODE_PERSISTENT_CORRUPTED);
}

// Writes all the data to the alarms. This is used to test the get/set persistent by index
static void writeToAllAlarms() {
  for (uint32_t i = 0; i < OBC_PERSISTENT_MAX_SUBINDEX_ALARM; ++i) {
    alarm_mgr_persist_data_t alarmIn = {
        .unixTime = i,
        .type = ALARM_TYPE_TIME_TAGGED_CMD_PERSIST,
    };
    ASSERT_EQ(setPersistentDataByIndex(OBC_PERSIST_SECTION_ID_ALARM_MGR, i, &alarmIn, sizeof(alarm_mgr_persist_data_t)),
              OBC_ERR_CODE_SUCCESS);
  }
}

// Reads all the data from the alarms
static void readAllAlarmsRegular() {
  for (uint32_t i = 0; i < OBC_PERSISTENT_MAX_SUBINDEX_ALARM; ++i) {
    alarm_mgr_persist_data_t alarmOut = {0};
    ASSERT_EQ(
        getPersistentDataByIndex(OBC_PERSIST_SECTION_ID_ALARM_MGR, i, &alarmOut, sizeof(alarm_mgr_persist_data_t)),
        OBC_ERR_CODE_SUCCESS);
    EXPECT_EQ(alarmOut.unixTime, i);
    EXPECT_EQ(alarmOut.type, ALARM_TYPE_TIME_TAGGED_CMD);
  }
}

TEST(TestOBCPersistent, ValidAllAlarms) {
  writeToAllAlarms();
  readAllAlarmsRegular();
}

TEST(TestOBCPersistent, CorruptAllAlarms) {
  // Write data for to all alarms
  writeToAllAlarms();

  // Corrupt all of the alarm's unixTimes
  for (int i = 0; i < OBC_PERSISTENT_MAX_SUBINDEX_ALARM; ++i) {
    uint32_t corrupt = 0xFFFF;
    // unixTimeAddr is calculated the same way as in the set/get persistent by sub index
    //  but outside of testing SHOULD NOT BE USED, use the provided functions
    uint32_t unixTimeAddr = OBC_PERSIST_ADDR_OF(alarmMgr[0].data) + sizeof(alarm_mgr_persist_t) * i;
    ASSERT_EQ(framWrite(unixTimeAddr, (uint8_t *)&corrupt, sizeof(uint32_t)), OBC_ERR_CODE_SUCCESS);
  }

  // Read out all the data for the alarms
  for (int i = 0; i < OBC_PERSISTENT_MAX_SUBINDEX_ALARM; ++i) {
    alarm_mgr_persist_data_t alarmOut = {0};
    EXPECT_EQ(
        getPersistentDataByIndex(OBC_PERSIST_SECTION_ID_ALARM_MGR, i, &alarmOut, sizeof(alarm_mgr_persist_data_t)),
        OBC_ERR_CODE_PERSISTENT_CORRUPTED);
  }
}

TEST(TestOBCPersistent, TimeAndAlarms) {
  writeToAllAlarms();
  obc_time_persist_data_t timeDataIn = {0};
  timeDataIn.unixTime = 0x12345678;

  ASSERT_EQ(setPersistentData(OBC_PERSIST_SECTION_ID_OBC_TIME, &timeDataIn, sizeof(obc_time_persist_data_t)),
            OBC_ERR_CODE_SUCCESS);

  obc_time_persist_data_t timeDataOut = {0};
  readAllAlarmsRegular();
  ASSERT_EQ(getPersistentData(OBC_PERSIST_SECTION_ID_OBC_TIME, &timeDataOut, sizeof(obc_time_persist_data_t)),
            OBC_ERR_CODE_SUCCESS);
  EXPECT_EQ(timeDataOut.unixTime, timeDataIn.unixTime);
}
