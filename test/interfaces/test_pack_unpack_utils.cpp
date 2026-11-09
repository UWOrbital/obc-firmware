#include "data_unpack_utils.h"
#include "data_pack_utils.h"

#include <gtest/gtest.h>

TEST(TestPackAndUnpack, ValidUint8PackUnpack) {
  uint8_t val = 0x12;
  uint8_t buf[sizeof(val)];

  uint32_t len = 0;
  packUint8(val, buf, &len);
  EXPECT_EQ(len, sizeof(uint8_t));

  len = 0;
  uint8_t unpackedVal = unpackUint8(buf, (uint32_t *)&len);

  EXPECT_EQ(len, sizeof(uint8_t));
  EXPECT_EQ(val, unpackedVal);
}

TEST(TestPackAndUnpack, ValidUint16PackUnpack) {
  uint16_t val = 0x1234;
  uint8_t buf[sizeof(val)];

  uint32_t len = 0;
  packUint16(val, buf, &len);
  EXPECT_EQ(len, sizeof(uint16_t));

  len = 0;
  uint16_t unpackedVal = unpackUint16(buf, (uint32_t *)&len);

  EXPECT_EQ(len, sizeof(uint16_t));
  EXPECT_EQ(val, unpackedVal);
}

TEST(TestPackAndUnpack, ValidUint32PackUnpack) {
  uint32_t val = 0x12345678;
  uint8_t buf[sizeof(val)];

  uint32_t len = 0;
  packUint32(val, buf, &len);
  EXPECT_EQ(len, sizeof(uint32_t));

  len = 0;
  uint32_t unpackedVal = unpackUint32(buf, (uint32_t *)&len);

  EXPECT_EQ(len, sizeof(uint32_t));
  EXPECT_EQ(val, unpackedVal);
}

TEST(TestPackAndUnpack, ValidInt8PackUnpack) {
  int8_t val = 0x12;
  uint8_t buf[sizeof(val)];

  uint32_t len = 0;
  packInt8(val, buf, &len);
  EXPECT_EQ(len, sizeof(int8_t));

  len = 0;
  int8_t unpackedVal = unpackInt8(buf, (uint32_t *)&len);

  EXPECT_EQ(len, sizeof(int8_t));
  EXPECT_EQ(val, unpackedVal);
}

TEST(TestPackAndUnpack, ValidInt16PackUnpack) {
  int16_t val = 0x1234;
  uint8_t buf[sizeof(val)];

  uint32_t len = 0;
  packInt16(val, buf, &len);
  EXPECT_EQ(len, sizeof(int16_t));

  len = 0;
  int16_t unpackedVal = unpackInt16(buf, (uint32_t *)&len);

  EXPECT_EQ(len, sizeof(int16_t));
  EXPECT_EQ(val, unpackedVal);
}

TEST(TestPackAndUnpack, ValidInt32PackUnpack) {
  int32_t val = 0x12345678;
  uint8_t buf[sizeof(val)];

  uint32_t len = 0;
  packInt32(val, buf, &len);
  EXPECT_EQ(len, sizeof(int32_t));

  len = 0;
  int32_t unpackedVal = unpackInt32(buf, (uint32_t *)&len);

  EXPECT_EQ(len, sizeof(int32_t));
  EXPECT_EQ(val, unpackedVal);
}

TEST(TestPackAndUnpack, ValidFloatPackUnpack) {
  float val = 12.3456789;
  uint8_t buf[sizeof(val)];

  uint32_t len = 0;
  packFloat(val, buf, &len);
  EXPECT_EQ(len, sizeof(float));

  len = 0;
  float unpackedVal = unpackFloat(buf, (uint32_t *)&len);

  EXPECT_EQ(len, sizeof(float));
  EXPECT_EQ(val, unpackedVal);
}

TEST(TestPackAndUnpack, LittleEndianUint16) {
  const uint8_t expected[] = {0x34, 0x12};
  uint8_t buf[sizeof(expected) + 1] = {0};
  buf[sizeof(expected)] = 0xA5;
  ASSERT_EQ(packUint16LE(buf, 0x1234U), OBC_GS_ERR_CODE_SUCCESS);
  for (size_t i = 0; i < sizeof(expected); i++) {
    EXPECT_EQ(buf[i], expected[i]);
  }
  EXPECT_EQ(buf[sizeof(expected)], 0xA5);

  uint16_t value = 0;
  ASSERT_EQ(unpackUint16LE(expected, &value), OBC_GS_ERR_CODE_SUCCESS);
  EXPECT_EQ(value, 0x1234U);
}

TEST(TestPackAndUnpack, LittleEndianUint16RejectsNullPointers) {
  const uint8_t buf[2] = {0};
  uint16_t value = 0x1234U;
  EXPECT_EQ(packUint16LE(nullptr, value), OBC_GS_ERR_CODE_INVALID_ARG);
  EXPECT_EQ(unpackUint16LE(nullptr, &value), OBC_GS_ERR_CODE_INVALID_ARG);
  EXPECT_EQ(value, 0x1234U);
  EXPECT_EQ(unpackUint16LE(buf, nullptr), OBC_GS_ERR_CODE_INVALID_ARG);
}

TEST(TestPackAndUnpack, LittleEndianUint32) {
  const uint8_t expected[] = {0x78, 0x56, 0x34, 0x12};
  uint8_t buf[sizeof(expected) + 1] = {0};
  buf[sizeof(expected)] = 0xA5;
  ASSERT_EQ(packUint32LE(buf, 0x12345678U), OBC_GS_ERR_CODE_SUCCESS);
  for (size_t i = 0; i < sizeof(expected); i++) {
    EXPECT_EQ(buf[i], expected[i]);
  }
  EXPECT_EQ(buf[sizeof(expected)], 0xA5);

  uint32_t value = 0;
  ASSERT_EQ(unpackUint32LE(expected, &value), OBC_GS_ERR_CODE_SUCCESS);
  EXPECT_EQ(value, 0x12345678U);
}

TEST(TestPackAndUnpack, LittleEndianUint32RejectsNullPointers) {
  const uint8_t buf[4] = {0};
  uint32_t value = 0x12345678U;
  EXPECT_EQ(packUint32LE(nullptr, value), OBC_GS_ERR_CODE_INVALID_ARG);
  EXPECT_EQ(unpackUint32LE(nullptr, &value), OBC_GS_ERR_CODE_INVALID_ARG);
  EXPECT_EQ(value, 0x12345678U);
  EXPECT_EQ(unpackUint32LE(buf, nullptr), OBC_GS_ERR_CODE_INVALID_ARG);
}
