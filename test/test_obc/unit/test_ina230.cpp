#include "obc_errors.h"
#include "ina230.h"
#include "mock_i2c_hal.h"

#include <gtest/gtest.h>

#define INA230_TEST_FLOAT_TOLERANCE 0.01f

// GTest setup class
class TestINA230 : public ::testing::Test {
 protected:
  ina230_t test_sensor;

  void SetUp() override {
    // These are preconfigured values based on the board, we should initiliaze them per INA instance.
    test_sensor.i2cAddress = 0x40;
    test_sensor.rShuntOhms = 0.05f;

    // current lsb = maxExpectedAmps / 2^15 = (32.768)/(32768) = 0.001
    // We are using this value as current_lsb has been hardcoded in mock_i2c_hal.c as 0.001
    test_sensor.maxExpectedAmps = 32.768f;

    // We are just initializing these two to zero, the driver will overwrite them during init call
    test_sensor.currentLsb = 0.0f;
    test_sensor.powerLsb = 0.0f;

    // Init the sensor and check math
    ASSERT_EQ(ina230Init(&test_sensor, INA230_CONFIG_OBC_DEFAULT), OBC_ERR_CODE_SUCCESS);
    ASSERT_FLOAT_EQ(test_sensor.currentLsb, 0.001f);
  }
};

// Init and Alerting Tests
TEST_F(TestINA230, InitInvalidArguments) {
  ina230_t bad_sensor = {0x40, 0.0f, 32.768f, 0, 0};  // Invalid: 0 ohm shunt
  EXPECT_EQ(ina230Init(&bad_sensor, INA230_CONFIG_OBC_DEFAULT), OBC_ERR_CODE_INVALID_ARG);

  bad_sensor.rShuntOhms = 0.05f;
  bad_sensor.maxExpectedAmps = -5.0f;  // Invalid: Negative max amps
  EXPECT_EQ(ina230Init(&bad_sensor, INA230_CONFIG_OBC_DEFAULT), OBC_ERR_CODE_INVALID_ARG);

  EXPECT_EQ(ina230Init(NULL, INA230_CONFIG_OBC_DEFAULT), OBC_ERR_CODE_INVALID_ARG);
}

TEST_F(TestINA230, SetOvercurrentAlertSuccess) {
  // Tests the new hardware alert function we added
  EXPECT_EQ(ina230SetOvercurrentAlert(&test_sensor, 3.0f), OBC_ERR_CODE_SUCCESS);
}

TEST_F(TestINA230, SetOvercurrentAlertInvalidArguments) {
  EXPECT_EQ(ina230SetOvercurrentAlert(NULL, 3.0f), OBC_ERR_CODE_INVALID_ARG);
  EXPECT_EQ(ina230SetOvercurrentAlert(&test_sensor, -1.0f), OBC_ERR_CODE_INVALID_ARG);  // Cannot have negative limit
}

// Shunt Voltage Tests
TEST_F(TestINA230, ShuntVoltageValues) {
  float voltage = 0;

  // Max register value (0xFFFF) = -0.0000025 V
  setMockShuntVoltageValue(-0.0000025f);
  EXPECT_EQ(ina230GetShuntVoltage(&test_sensor, &voltage), OBC_ERR_CODE_SUCCESS);
  EXPECT_FLOAT_EQ(voltage, -0.0000025f);

  // Min register value (0x0000) = 0 V
  setMockShuntVoltageValue(0.0f);
  EXPECT_EQ(ina230GetShuntVoltage(&test_sensor, &voltage), OBC_ERR_CODE_SUCCESS);
  EXPECT_FLOAT_EQ(voltage, 0.0f);

  // Lowest register value (0x8000) = -32768 * 2.5 uV = -0.08192 V
  setMockShuntVoltageValue(-0.08192f);
  EXPECT_EQ(ina230GetShuntVoltage(&test_sensor, &voltage), OBC_ERR_CODE_SUCCESS);
  EXPECT_FLOAT_EQ(voltage, -0.08192f);

  // Positive value
  setMockShuntVoltageValue(0.08f);
  EXPECT_EQ(ina230GetShuntVoltage(&test_sensor, &voltage), OBC_ERR_CODE_SUCCESS);
  EXPECT_FLOAT_EQ(voltage, 0.08f);
}

TEST_F(TestINA230, ShuntVoltageInvalidArguments) {
  float voltage = 0;
  EXPECT_EQ(ina230GetShuntVoltage(NULL, &voltage), OBC_ERR_CODE_INVALID_ARG);
  EXPECT_EQ(ina230GetShuntVoltage(&test_sensor, NULL), OBC_ERR_CODE_INVALID_ARG);
}

// Bus Voltage Value Tests
TEST_F(TestINA230, BusVoltageValues) {
  float voltage = 0;

  // Max register value (0xFFFF) = 65535 * 1.25 mV = 81.91875 V
  setMockBusVoltageValue(81.91875f);
  EXPECT_EQ(ina230GetBusVoltage(&test_sensor, &voltage), OBC_ERR_CODE_SUCCESS);
  EXPECT_FLOAT_EQ(voltage, 81.91875f);

  // Min register value (0x0000)
  setMockBusVoltageValue(0.0f);
  EXPECT_EQ(ina230GetBusVoltage(&test_sensor, &voltage), OBC_ERR_CODE_SUCCESS);
  EXPECT_FLOAT_EQ(voltage, 0.0f);

  // Standard positive value
  setMockBusVoltageValue(5.12f);
  EXPECT_EQ(ina230GetBusVoltage(&test_sensor, &voltage), OBC_ERR_CODE_SUCCESS);
  EXPECT_FLOAT_EQ(voltage, 5.12f);
}

TEST_F(TestINA230, BusVoltageInvalidArguments) {
  float voltage = 0;
  EXPECT_EQ(ina230GetBusVoltage(NULL, &voltage), OBC_ERR_CODE_INVALID_ARG);
  EXPECT_EQ(ina230GetBusVoltage(&test_sensor, NULL), OBC_ERR_CODE_INVALID_ARG);
}

// Current Value Tests
TEST_F(TestINA230, CurrentValues) {
  float current = 0;

  // Max register value (0xFFFF) = -0.001 A
  setMockCurrentValue(-0.001f);
  EXPECT_EQ(ina230GetCurrent(&test_sensor, &current), OBC_ERR_CODE_SUCCESS);
  EXPECT_FLOAT_EQ(current, -0.001f);

  // Min register value (0x0000)
  setMockCurrentValue(0.0f);
  EXPECT_EQ(ina230GetCurrent(&test_sensor, &current), OBC_ERR_CODE_SUCCESS);
  EXPECT_FLOAT_EQ(current, 0.0f);

  // Lowest register value (0x8000) = -32.768 A
  setMockCurrentValue(-32.768f);
  EXPECT_EQ(ina230GetCurrent(&test_sensor, &current), OBC_ERR_CODE_SUCCESS);
  EXPECT_FLOAT_EQ(current, -32.768f);

  // Standard positive value
  setMockCurrentValue(5.12f);
  EXPECT_EQ(ina230GetCurrent(&test_sensor, &current), OBC_ERR_CODE_SUCCESS);
  EXPECT_NEAR(current, 5.12f, INA230_TEST_FLOAT_TOLERANCE);
}

TEST_F(TestINA230, CurrentInvalidArguments) {
  float current = 0;
  EXPECT_EQ(ina230GetCurrent(NULL, &current), OBC_ERR_CODE_INVALID_ARG);
  EXPECT_EQ(ina230GetCurrent(&test_sensor, NULL), OBC_ERR_CODE_INVALID_ARG);
}

// Power Value Tests
TEST_F(TestINA230, PowerValues) {
  float power = 0;

  // Max register value (0xFFFF) = 65535 * (25 * 0.001) = 1638.375 W
  setMockPowerValue(1638.375f);
  EXPECT_EQ(ina230GetPower(&test_sensor, &power), OBC_ERR_CODE_SUCCESS);
  EXPECT_FLOAT_EQ(power, 1638.375f);

  // Min register value (0x0000)
  setMockPowerValue(0.0f);
  EXPECT_EQ(ina230GetPower(&test_sensor, &power), OBC_ERR_CODE_SUCCESS);
  EXPECT_FLOAT_EQ(power, 0.0f);

  // Standard positive value
  setMockPowerValue(5.12f);
  EXPECT_EQ(ina230GetPower(&test_sensor, &power), OBC_ERR_CODE_SUCCESS);
  EXPECT_NEAR(power, 5.12f, INA230_TEST_FLOAT_TOLERANCE);
}

TEST_F(TestINA230, PowerInvalidArguments) {
  float power = 0;
  EXPECT_EQ(ina230GetPower(NULL, &power), OBC_ERR_CODE_INVALID_ARG);
  EXPECT_EQ(ina230GetPower(&test_sensor, NULL), OBC_ERR_CODE_INVALID_ARG);
}
