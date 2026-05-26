#include "obc_gs_telemetry_pack.h"
#include "obc_gs_telemetry_unpack.h"
#include "obc_gs_telemetry_id.h"
#include "obc_gs_telemetry_data.h"
#include "obc_gs_errors.h"

#include <gtest/gtest.h>

TEST(TestTelemetryPackUnpack, ValidTelemObcTempPackUnpack) {
  obc_gs_error_code_t err;

  telemetry_data_t data = {0};
  data.id = TELEM_OBC_TEMP;
  data.timestamp = 0x12345678;
  data.obcTemp = 102.310f;

  uint8_t buffer[MAX_TELEMETRY_DATA_SIZE] = {0};

  uint32_t numPacked = 0;
  err = packTelemetry((const telemetry_data_t *)&data, buffer, MAX_TELEMETRY_DATA_SIZE, &numPacked);
  ASSERT_EQ(err, OBC_GS_ERR_CODE_SUCCESS);

  telemetry_data_t unpackedData = {0};
  uint32_t numUnpacked = 0;
  err = unpackTelemetry((const uint8_t *)&buffer, &numUnpacked, &unpackedData);
  ASSERT_EQ(err, OBC_GS_ERR_CODE_SUCCESS);

  EXPECT_EQ(numPacked, numUnpacked);
  EXPECT_EQ(data.id, unpackedData.id);
  EXPECT_EQ(data.timestamp, unpackedData.timestamp);
  EXPECT_EQ(data.obcTemp, unpackedData.obcTemp);
}

TEST(TestTelemetryPackUnpack, ValidTelemObcStatePackUnpack) {
  obc_gs_error_code_t err;

  telemetry_data_t data = {0};
  data.id = TELEM_OBC_STATE;
  data.timestamp = 0x12345678;
  data.obcState = 0x12;

  uint8_t buffer[MAX_TELEMETRY_DATA_SIZE] = {0};

  uint32_t numPacked = 0;
  err = packTelemetry((const telemetry_data_t *)&data, buffer, MAX_TELEMETRY_DATA_SIZE, &numPacked);
  ASSERT_EQ(err, OBC_GS_ERR_CODE_SUCCESS);

  telemetry_data_t unpackedData = {0};
  uint32_t numUnpacked = 0;
  err = unpackTelemetry((const uint8_t *)&buffer, &numUnpacked, &unpackedData);
  ASSERT_EQ(err, OBC_GS_ERR_CODE_SUCCESS);

  EXPECT_EQ(numPacked, numUnpacked);
  EXPECT_EQ(data.id, unpackedData.id);
  EXPECT_EQ(data.timestamp, unpackedData.timestamp);
  EXPECT_EQ(data.obcState, unpackedData.obcState);
}

TEST(TestTelemetryPackUnpack, ValidTelemPongPackUnpack) {
  obc_gs_error_code_t err;

  telemetry_data_t data = {0};
  data.id = TELEM_PONG;
  data.timestamp = 0x12345678;

  uint8_t buffer[MAX_TELEMETRY_DATA_SIZE] = {0};

  uint32_t numPacked = 0;
  err = packTelemetry((const telemetry_data_t *)&data, buffer, MAX_TELEMETRY_DATA_SIZE, &numPacked);
  ASSERT_EQ(err, OBC_GS_ERR_CODE_SUCCESS);

  telemetry_data_t unpackedData = {0};
  uint32_t numUnpacked = 0;
  err = unpackTelemetry((const uint8_t *)&buffer, &numUnpacked, &unpackedData);
  ASSERT_EQ(err, OBC_GS_ERR_CODE_SUCCESS);

  EXPECT_EQ(numPacked, numUnpacked);
  EXPECT_EQ(data.id, unpackedData.id);
  EXPECT_EQ(data.timestamp, unpackedData.timestamp);
}
