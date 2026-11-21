#include "qeynet_beacon_uart.h"
#include "obc_errors.h"

#include <cstring>
#include <gtest/gtest.h>

// From ICD
TEST(TestQeynetUART, UnpackConvertICDPacket) {
    const char* buffer = "CN:00BE,SA:00C8,VI:08A2,GD:000B,TM:0ADF,T1:049F,T2:039F\r\n";
    qeynet_beacon_uart_packet_t packet = {0};

    ASSERT_EQ(qeynetBeaconUARTParsePacket(buffer, strlen(buffer), &packet), OBC_ERR_CODE_SUCCESS);

    EXPECT_EQ(packet.count, 190);
    EXPECT_EQ(packet.samples, 200);
    EXPECT_EQ(packet.adcVIn, 2210);
    EXPECT_EQ(packet.adcVGnd, 11);
    EXPECT_EQ(packet.adcVTM, 2783);
    EXPECT_EQ(packet.adcVT1, 1183);
    EXPECT_EQ(packet.adcVT2, 927);

    // Differs from ICD: lists V = 0.8098, calculated V = 0.8095
    EXPECT_NEAR(qeynetAdcToVoltage(packet.adcVIn), 0.8095f, 0.0001f);
    EXPECT_NEAR(qeynetAdcToVoltage(packet.adcVGnd), 0.004f, 0.001f);
    EXPECT_NEAR(qeynetAdcToVoltage(packet.adcVTM), 1.0195f , 0.0001f);
    EXPECT_NEAR(qeynetAdcToVoltage(packet.adcVT1), 0.4333f, 0.0001f);
    EXPECT_NEAR(qeynetAdcToVoltage(packet.adcVT2), 0.3395f, 0.0001f);

    EXPECT_NEAR(qeynetVoltSupply(&packet), 26.99f, 0.01f);
    // Differs from ICD: lists T = 65.89, calculated T = 65.86
    EXPECT_NEAR(qeynetTempMC(&packet), 65.86f, 0.01f);
    // Differs from ICD: lists R = 5.304, calculated R = 5.306
    EXPECT_NEAR(qeynetResistT1(&packet), 5.306f, 0.001f);
    // Differs from ICD: lists R = 3.738, calculated R = 3.730
    EXPECT_NEAR(qeynetResistT2(&packet), 3.730f, 0.001f);
    // Differs from ICD: lists T = 42.4, calculated T = 43.4
    EXPECT_NEAR(qeynetTempT1(&packet), 43.4f, 0.1f);
    // Differs from ICD: lists T = 53.0, calculated T = 54.7
    EXPECT_NEAR(qeynetTempT2(&packet), 54.7f, 0.1f);
}
