#include "obc_gs_ax25.h"
#include "obc_gs_fec.h"
#include "obc_gs_errors.h"

#include <string.h>

#include <gtest/gtest.h>

// NOTE: AKITO is the groundStationCallsign
// NOTE: ATLAS is the cubeSatCallSign
TEST(TestAx25SendRecv, iFrameLittleStuff) {
  uint8_t telemData[RS_ENCODED_SIZE] = {0};

  setCurrentLinkDestCallSign(GROUND_STATION_CALLSIGN, CALLSIGN_LENGTH, DEFAULT_SSID);

  unstuffed_ax25_i_frame_t unstuffedAx25Data = {0};
  ASSERT_EQ(ax25SendIFrame(telemData, RS_ENCODED_SIZE, &unstuffedAx25Data), OBC_GS_ERR_CODE_SUCCESS);

  packed_ax25_i_frame_t ax25Data = {0};
  ASSERT_EQ(ax25Stuff(unstuffedAx25Data.data, unstuffedAx25Data.length, ax25Data.data, &ax25Data.length),
            OBC_GS_ERR_CODE_SUCCESS);

  unstuffed_ax25_i_frame_t unstuffedPacket = {0};
  ASSERT_EQ(ax25Unstuff(ax25Data.data, ax25Data.length, unstuffedPacket.data, &unstuffedPacket.length),
            OBC_GS_ERR_CODE_SUCCESS);
  EXPECT_EQ(unstuffedPacket.length, AX25_MINIMUM_I_FRAME_LEN);

  u_frame_cmd_t command;
  ASSERT_EQ(ax25Recv(&unstuffedPacket, &command), OBC_GS_ERR_CODE_SUCCESS);
  for (uint16_t i = 0; i < RS_ENCODED_SIZE; ++i) {
    EXPECT_EQ(telemData[i], unstuffedPacket.data[AX25_INFO_FIELD_POSITION + i]);
  }
}

TEST(TestAx25SendRecv, iFrameMaxStuff) {
  uint8_t telemData[RS_ENCODED_SIZE] = {0};
  memset(telemData, 0xFF, RS_ENCODED_SIZE);

  setCurrentLinkDestCallSign(GROUND_STATION_CALLSIGN, CALLSIGN_LENGTH, DEFAULT_SSID);
  unstuffed_ax25_i_frame_t unstuffedAx25Data = {0};
  ASSERT_EQ(ax25SendIFrame(telemData, RS_ENCODED_SIZE, &unstuffedAx25Data), OBC_GS_ERR_CODE_SUCCESS);

  packed_ax25_i_frame_t ax25Data = {0};
  ASSERT_EQ(ax25Stuff(unstuffedAx25Data.data, unstuffedAx25Data.length, ax25Data.data, &ax25Data.length),
            OBC_GS_ERR_CODE_SUCCESS);
  EXPECT_TRUE(ax25Data.length > AX25_MINIMUM_I_FRAME_LEN && ax25Data.length < AX25_MAXIMUM_PKT_LEN);

  unstuffed_ax25_i_frame_t unstuffedPacket = {0};
  ASSERT_EQ(ax25Unstuff(ax25Data.data, ax25Data.length, unstuffedPacket.data, &unstuffedPacket.length),
            OBC_GS_ERR_CODE_SUCCESS);
  EXPECT_EQ(unstuffedPacket.length, AX25_MINIMUM_I_FRAME_LEN);

  u_frame_cmd_t command;
  ASSERT_EQ(ax25Recv(&unstuffedPacket, &command), OBC_GS_ERR_CODE_SUCCESS);
  for (uint16_t i = 0; i < RS_ENCODED_SIZE; ++i) {
    EXPECT_EQ(telemData[i], unstuffedPacket.data[AX25_INFO_FIELD_POSITION + i]);
  }
}

TEST(TestAx25SendRecv, iFrameSomeStuff) {
  uint8_t telemData[RS_ENCODED_SIZE] = {0};
  uint32_t seed = 0xF72FA1;  // random number
  for (uint16_t i = 0; i < RS_ENCODED_SIZE; ++i) {
    // Pseudorandom generation using a simple algorithm
    seed = (seed * 1103515245 + 12345) % (1 << 31);
    telemData[i] = (uint8_t)(seed & 0xFF);
  }

  setCurrentLinkDestCallSign(GROUND_STATION_CALLSIGN, CALLSIGN_LENGTH, DEFAULT_SSID);
  unstuffed_ax25_i_frame_t unstuffedAx25Data = {0};
  ASSERT_EQ(ax25SendIFrame(telemData, RS_ENCODED_SIZE, &unstuffedAx25Data), OBC_GS_ERR_CODE_SUCCESS);

  packed_ax25_i_frame_t ax25Data = {0};
  ASSERT_EQ(ax25Stuff(unstuffedAx25Data.data, unstuffedAx25Data.length, ax25Data.data, &ax25Data.length),
            OBC_GS_ERR_CODE_SUCCESS);

  EXPECT_TRUE(ax25Data.length > AX25_MINIMUM_I_FRAME_LEN && ax25Data.length < AX25_MAXIMUM_PKT_LEN);

  unstuffed_ax25_i_frame_t unstuffedPacket = {0};
  ASSERT_EQ(ax25Unstuff(ax25Data.data, ax25Data.length, unstuffedPacket.data, &unstuffedPacket.length),
            OBC_GS_ERR_CODE_SUCCESS);
  EXPECT_EQ(unstuffedPacket.length, AX25_MINIMUM_I_FRAME_LEN);

  u_frame_cmd_t command;
  ASSERT_EQ(ax25Recv(&unstuffedPacket, &command), OBC_GS_ERR_CODE_SUCCESS);
  for (uint16_t i = 0; i < RS_ENCODED_SIZE; ++i) {
    EXPECT_EQ(telemData[i], unstuffedPacket.data[AX25_INFO_FIELD_POSITION + i]);
  }
}

// TODO: Get flagsharing corrected
/*
TEST(TestAx25SendRecv, iFrameSendRecvFlagShare) {
  uint8_t ax25Data[(3 * AX25_MINIMUM_I_FRAME_LEN_SHARE_FLAG) + 1] = {0};
  uint8_t telemData[3 * AX25_INFO_BYTES] = {0};
  uint16_t telemDataLen = (3 * AX25_INFO_BYTES);
  uint16_t ax25DataLen = 0;

  ASSERT_EQ(ax25SendIFrameWithFlagSharing(telemData, telemDataLen, ax25Data,
                                          ax25UnstuffedWithFlagShareLen(telemDataLen), &groundStationCallsign),
            OBC_GS_ERR_CODE_SUCCESS);
  uint8_t stuffedAx25Data[(3 * AX25_MAXIMUM_PKT_LEN)] = {0};
  uint16_t count[3] = {0};

  for (int i = 0; i < 3; ++i) {
    EXPECT_EQ(ax25Data[(AX25_MINIMUM_I_FRAME_LEN_SHARE_FLAG * i)], AX25_FLAG);
    ASSERT_EQ(ax25Stuff(ax25Data + (AX25_MINIMUM_I_FRAME_LEN_SHARE_FLAG * i), AX25_MINIMUM_I_FRAME_LEN,
                        stuffedAx25Data + (AX25_MINIMUM_I_FRAME_LEN_SHARE_FLAG * i), count + i),
              OBC_GS_ERR_CODE_SUCCESS);
  }

  int index = 0;

  uint8_t unstuffedPacket[(3 * AX25_MAXIMUM_PKT_LEN)] = {0};
  uint16_t unstuffedCount[3] = {0};
  int flagCount = 0;
  for (int i = 0; i < 3; ++i) {
    for (int index = 0; index < (3 * AX25_MAXIMUM_PKT_LEN); ++index) {
      if (stuffedAx25Data[i] == AX25_FLAG) {
        ASSERT_EQ(ax25Unstuff(stuffedAx25Data + index, count[i],
                              unstuffedPacket + (AX25_MINIMUM_I_FRAME_LEN_SHARE_FLAG * i), unstuffedCount + i),
                  OBC_GS_ERR_CODE_SUCCESS);
        EXPECT_EQ(unstuffedCount[i], AX25_MINIMUM_I_FRAME_LEN);

        unstuffed_ax25_i_frame_t recvPacket = {0};
        memcpy(recvPacket.data, unstuffedPacket + (AX25_MINIMUM_I_FRAME_LEN_SHARE_FLAG * i), AX25_MINIMUM_I_FRAME_LEN);
        recvPacket.length = AX25_MINIMUM_I_FRAME_LEN;
        u_frame_cmd_t command;
        EXPECT_EQ(ax25Recv(&recvPacket, &command), OBC_GS_ERR_CODE_SUCCESS);
        flagCount++;
      }
      if (flagCount == 4) break;
    }
  }
}

TEST(TestAx25SendRecv, iFrameSendRecvFlagShareStuff) {
  uint8_t ax25Data[(3 * AX25_MINIMUM_I_FRAME_LEN_SHARE_FLAG) + 1] = {0};
  uint8_t telemData[3 * AX25_INFO_BYTES] = {0};
  memset(telemData, 0xFF, 3 * AX25_INFO_BYTES);
  uint16_t telemDataLen = (3 * AX25_INFO_BYTES);
  uint16_t ax25DataLen = 0;

  ASSERT_EQ(ax25SendIFrameWithFlagSharing(telemData, telemDataLen, ax25Data,
                                          ax25UnstuffedWithFlagShareLen(telemDataLen), &groundStationCallsign),
            OBC_GS_ERR_CODE_SUCCESS);
  uint8_t stuffedAx25Data[(3 * AX25_MAXIMUM_PKT_LEN)] = {0};
  uint16_t count[3] = {0};

  for (int i = 0; i < 3; ++i) {
    EXPECT_EQ(ax25Data[(AX25_MINIMUM_I_FRAME_LEN_SHARE_FLAG * i)], AX25_FLAG);
    ASSERT_EQ(ax25Stuff(ax25Data + (AX25_MINIMUM_I_FRAME_LEN_SHARE_FLAG * i), AX25_MINIMUM_I_FRAME_LEN,
                        stuffedAx25Data + (AX25_MINIMUM_I_FRAME_LEN_SHARE_FLAG * i), count + i),
              OBC_GS_ERR_CODE_SUCCESS);
  }

  int index = 0;

  uint8_t unstuffedPacket[(3 * AX25_MAXIMUM_PKT_LEN)] = {0};
  uint16_t unstuffedCount[3] = {0};
  int flagCount = 0;
  for (int i = 0; i < 3; ++i) {
    for (int index = 0; index < (3 * AX25_MAXIMUM_PKT_LEN); ++index) {
      if (stuffedAx25Data[i] == AX25_FLAG) {
        ASSERT_EQ(ax25Unstuff(stuffedAx25Data + index, count[i],
                              unstuffedPacket + (AX25_MINIMUM_I_FRAME_LEN_SHARE_FLAG * i), unstuffedCount + i),
                  OBC_GS_ERR_CODE_SUCCESS);
        EXPECT_EQ(unstuffedCount[i], AX25_MINIMUM_I_FRAME_LEN);

        unstuffed_ax25_i_frame_t recvPacket = {0};
        memcpy(recvPacket.data, unstuffedPacket + (AX25_MINIMUM_I_FRAME_LEN_SHARE_FLAG * i), AX25_MINIMUM_I_FRAME_LEN);
        recvPacket.length = AX25_MINIMUM_I_FRAME_LEN;
        u_frame_cmd_t command;
        EXPECT_EQ(ax25Recv(&recvPacket, &command), OBC_GS_ERR_CODE_SUCCESS);
        flagCount++;
      }
      if (flagCount == 4) break;
    }
  }
}
*/

TEST(TestAx25SendRecv, uFrameSendRecvConn) {
  packed_ax25_u_frame_t ax25Data = {0};
  uint8_t pollFinalBit = 1;

  setCurrentLinkDestCallSign(CUBE_SAT_CALLSIGN, CALLSIGN_LENGTH, DEFAULT_SSID);
  ASSERT_EQ(ax25SendUFrame(&ax25Data, U_FRAME_CMD_CONN, pollFinalBit), OBC_GS_ERR_CODE_SUCCESS);

  unstuffed_ax25_i_frame_t unstuffedPacket = {0};
  ASSERT_EQ(ax25Unstuff(ax25Data.data, ax25Data.length, unstuffedPacket.data, &unstuffedPacket.length),
            OBC_GS_ERR_CODE_SUCCESS);

  u_frame_cmd_t command;
  ASSERT_EQ(ax25Recv(&unstuffedPacket, &command), OBC_GS_ERR_CODE_SUCCESS);
  EXPECT_EQ(command, U_FRAME_CMD_CONN);
}

TEST(TestAx25SendRecv, uFrameSendRecvDisc) {
  packed_ax25_u_frame_t ax25Data = {0};
  uint8_t pollFinalBit = 1;

  setCurrentLinkDestCallSign(GROUND_STATION_CALLSIGN, CALLSIGN_LENGTH, DEFAULT_SSID);
  ASSERT_EQ(ax25SendUFrame(&ax25Data, U_FRAME_CMD_DISC, pollFinalBit), OBC_GS_ERR_CODE_SUCCESS);

  unstuffed_ax25_i_frame_t unstuffedPacket = {0};
  ASSERT_EQ(ax25Unstuff(ax25Data.data, ax25Data.length, unstuffedPacket.data, &unstuffedPacket.length),
            OBC_GS_ERR_CODE_SUCCESS);

  u_frame_cmd_t command;
  ASSERT_EQ(ax25Recv(&unstuffedPacket, &command), OBC_GS_ERR_CODE_SUCCESS);
  EXPECT_EQ(command, U_FRAME_CMD_DISC);
}

TEST(TestAx25SendRecv, uFrameSendRecvAck) {
  packed_ax25_u_frame_t ax25Data = {0};
  uint8_t pollFinalBit = 1;

  setCurrentLinkDestCallSign(GROUND_STATION_CALLSIGN, CALLSIGN_LENGTH, DEFAULT_SSID);
  ASSERT_EQ(ax25SendUFrame(&ax25Data, U_FRAME_CMD_ACK, pollFinalBit), OBC_GS_ERR_CODE_SUCCESS);

  unstuffed_ax25_i_frame_t unstuffedPacket = {0};
  ASSERT_EQ(ax25Unstuff(ax25Data.data, ax25Data.length, unstuffedPacket.data, &unstuffedPacket.length),
            OBC_GS_ERR_CODE_SUCCESS);

  u_frame_cmd_t command;
  ASSERT_EQ(ax25Recv(&unstuffedPacket, &command), OBC_GS_ERR_CODE_SUCCESS);
  EXPECT_EQ(command, U_FRAME_CMD_ACK);
}

TEST(TestAx25SendRecV, Ax25SourceAddressGenerator) {
  ax25_addr_t sourceAddress;
  memset(sourceAddress.data, 0, 7);
  sourceAddress.length = 7;

  uint8_t expectedAddress[] = {0x9C, 0x6E, 0x98, 0x8A, 0x9A, 0x40, 0x61};  // Source subfield from documentation
  uint8_t callSign[] = {'N', '7', 'L', 'E', 'M'};
  uint8_t callSign2[] = {'N', '7', 'L', 'E', 'M', '4', '2'};

  ASSERT_EQ(ax25GetSourceAddress(&sourceAddress, callSign2, 7, 0, 0), 1);

  ax25GetSourceAddress(&sourceAddress, callSign, 5, 0, 0);
  ASSERT_EQ(memcmp(&sourceAddress, &expectedAddress, 7), 0);
}

TEST(TestAx25SendRecV, Ax25DestAddressGenerator) {
  ax25_addr_t sourceAddress;
  memset(sourceAddress.data, 0, 7);
  sourceAddress.length = 7;

  uint8_t expectedAddress[] = {0x9C, 0x94, 0x6E, 0xA0, 0x40, 0x40, 0xE0};  // Source subfield from documentation
  uint8_t callSign[] = {'N', 'J', '7', 'P'};
  uint8_t callSign2[] = {'N', '7', 'L', 'E', 'M', '4', '2'};

  ASSERT_EQ(ax25GetDestAddress(&sourceAddress, callSign2, 7, 0, 0), 1);

  ax25GetDestAddress(&sourceAddress, callSign, 4, 0, 1);
  ASSERT_EQ(memcmp(&sourceAddress, &expectedAddress, 7), 0);
}
