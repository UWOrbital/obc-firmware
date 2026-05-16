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
