#include "vn100_binary_parsing.h"
#include "obc_errors.h"

#include <stdint.h>
#include <stdlib.h>

#include <gtest/gtest.h>

TEST(TestVn100PackUnpack, ValidVn100PackUnpack) {
  const uint8_t header[] = {0xFA, 0x01, 0x28, 0x05};
  const uint8_t payload[] = {0xA8, 0x02, 0xC3, 0x3F, 0xE7, 0x89, 0xB0, 0x42, 0x3D, 0xA9, 0x6A, 0xC1, 0xD0, 0x83,
                             0x54, 0x3A, 0x92, 0x3E, 0x0B, 0xBB, 0x1C, 0x25, 0x30, 0xBA, 0x04, 0x7D, 0x1C, 0x41,
                             0x3B, 0xB2, 0x83, 0x3D, 0x4E, 0xEA, 0x9C, 0xBE, 0xF1, 0xBD, 0x93, 0xBE, 0xA4, 0x55,
                             0x8C, 0xBD, 0x26, 0xC7, 0x77, 0x3E, 0xF4, 0x28, 0xE6, 0x41, 0x57, 0xCE, 0xC1, 0x42};
  const uint8_t crc[] = {0xC0, 0xE9};
  float payloadFloat[14];

  memcpy(payloadFloat, payload, sizeof(payload));

  // Calculate the total size of the data array
  size_t totalSize = sizeof(header) + sizeof(payload) + sizeof(crc);

  // Create a char array to store the combined data
  unsigned char mockVn100Packet[totalSize];

  // Copy the header, payload, and crc into the combinedData array
  memcpy(mockVn100Packet, header, sizeof(header));
  memcpy(mockVn100Packet + sizeof(header), payload, sizeof(payload));
  memcpy(mockVn100Packet + sizeof(header) + sizeof(payload), &crc, sizeof(crc));

  vn100_binary_packet_t receivedPacket;

  ASSERT_EQ(vn100ParsePacket(mockVn100Packet, totalSize, &receivedPacket), OBC_ERR_CODE_SUCCESS);

  EXPECT_EQ(receivedPacket.yaw, payloadFloat[0]);
  EXPECT_EQ(receivedPacket.pitch, payloadFloat[1]);
  EXPECT_EQ(receivedPacket.roll, payloadFloat[2]);

  EXPECT_EQ(receivedPacket.gyroX, payloadFloat[3]);
  EXPECT_EQ(receivedPacket.gyroY, payloadFloat[4]);
  EXPECT_EQ(receivedPacket.gyroZ, payloadFloat[5]);

  EXPECT_EQ(receivedPacket.accelX, payloadFloat[6]);
  EXPECT_EQ(receivedPacket.accelY, payloadFloat[7]);
  EXPECT_EQ(receivedPacket.accelZ, payloadFloat[8]);

  EXPECT_EQ(receivedPacket.magX, payloadFloat[9]);
  EXPECT_EQ(receivedPacket.magY, payloadFloat[10]);
  EXPECT_EQ(receivedPacket.magZ, payloadFloat[11]);

  EXPECT_EQ(receivedPacket.temp, payloadFloat[12]);
  EXPECT_EQ(receivedPacket.pres, payloadFloat[13]);

  unsigned char mockErrorPacket[] = "$VNERR,1";
  ASSERT_EQ(vn100ParsePacket(mockErrorPacket, totalSize, &receivedPacket), OBC_ERR_CODE_VN100_HARDFAULT);
}
