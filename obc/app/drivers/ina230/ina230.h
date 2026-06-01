#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "obc_errors.h"
#include <stdint.h>
#include <stdbool.h>

// INA230 Register Addresses
#define INA230_REG_CONFIG 0x00
#define INA230_REG_SHUNT_VOLT 0x01
#define INA230_REG_BUS_VOLT 0x02
#define INA230_REG_POWER 0x03
#define INA230_REG_CURRENT 0x04
#define INA230_REG_CALIBRATION 0x05
#define INA230_REG_MASK_ENABLE 0x06
#define INA230_REG_ALERT_LIMIT 0x07

// Fixed Hardware Scaling Factors
#define INA230_BUS_VOLTAGE_LSB 0.00125      // 1.25 mV per bit
#define INA230_SHUNT_VOLTAGE_LSB 0.0000025  // 2.5 uV per bit
#define INA230_CURRENT_TO_POWER_LSB 25.0    // power_lsb = 25 * current_lsb

// Config Register value that gives us 16x averaging, 1.1ms conversion times, continuous reading
#define INA230_CONFIG_OBC_DEFAULT 0x4527
// Bit 15 configures the ALRT pin to trigger on shunt over-voltage, Write this to Mask/Enable register
#define INA230_ALERT_SHUNT_OVER_VOLTAGE 0x8000
// Bit 4 of the Mask/Enable register is the Alert Function Flag, set when the configured alert has tripped
#define INA230_FLAG_ALERT_FUNCTION 0x0010
// I2C Timeout for read/write transfers
#define I2C_TRANSFER_TIMEOUT_TICKS pdMS_TO_TICKS(100)

/**
 * @struct ina230_t
 * @brief INA230 Device Handle. Holds the specific hardware configuration and calculated internal states for a single
 * physical sensor.
 * @var i2cAddress The 7-bit I2C address of the device.
 * @var rShuntOhms Physical shunt resistor value in Ohms (e.g., 0.005).
 * @var maxExpectedAmps Maximum expected current in Amps used to calculate LSB.
 * @var currentLsb Internal driver state (calculated during init).
 * @var powerLsb Internal driver state (calculated during init).
 */
typedef struct {
  uint8_t i2cAddress;
  float rShuntOhms;
  float maxExpectedAmps;
  float currentLsb;
  float powerLsb;
} ina230_t;

/**
 * @brief Initializes the INA230 device and calculates internal LSBs.
 * @param device Pointer to the INA230 device handle.
 * @param configRegisterValue 16-bit value to write to the configuration register.
 * @return OBC_ERR_CODE_SUCCESS if initialization succeeded, error code otherwise.
 */
obc_error_code_t ina230Init(ina230_t *device, uint16_t configRegisterValue);

/**
 * @brief Reads the bus voltage from the INA230.
 * @param device Pointer to the INA230 device handle.
 * @param voltageOut Pointer to store the read bus voltage in Volts.
 * @return OBC_ERR_CODE_SUCCESS if read succeeded, error code otherwise.
 */
obc_error_code_t ina230GetBusVoltage(const ina230_t *device, float *voltageOut);

/**
 * @brief Reads the shunt voltage from the INA230.
 * @param device Pointer to the INA230 device handle.
 * @param voltageOut Pointer to store the read shunt voltage in Volts.
 * @return OBC_ERR_CODE_SUCCESS if read succeeded, error code otherwise.
 */
obc_error_code_t ina230GetShuntVoltage(const ina230_t *device, float *voltageOut);

/**
 * @brief Reads the calculated current from the INA230.
 * @param device Pointer to the INA230 device handle.
 * @param currentOut Pointer to store the read current in Amps.
 * @return OBC_ERR_CODE_SUCCESS if read succeeded, error code otherwise.
 */
obc_error_code_t ina230GetCurrent(const ina230_t *device, float *currentOut);

/**
 * @brief Reads the calculated power from the INA230.
 * @param device Pointer to the INA230 device handle.
 * @param powerOut Pointer to store the read power in Watts.
 * @return OBC_ERR_CODE_SUCCESS if read succeeded, error code otherwise.
 */
obc_error_code_t ina230GetPower(const ina230_t *device, float *powerOut);

/**
 * @brief Configures the hardware alert pin to trigger on an overcurrent event.
 * @param device Pointer to the INA230 device handle.
 * @param maxCurrentAmps The threshold current in Amps that triggers the alert.
 * @return OBC_ERR_CODE_SUCCESS if configuration succeeded, error code otherwise.
 */
obc_error_code_t ina230SetOvercurrentAlert(const ina230_t *device, float maxCurrentAmps);

/**
 * @brief Polls the Mask/Enable register to see if the configured alert has tripped.
 *
 * The Alert Function Flag (bit 4) latches high when the alert configured by
 * ina230SetOvercurrentAlert() trips. Reading the Mask/Enable register clears the flag.
 *
 * @param device Pointer to the INA230 device handle.
 * @param hasAlerted Set to true if the alert function flag was set, false otherwise.
 * @return OBC_ERR_CODE_SUCCESS if read succeeded, error code otherwise.
 */
obc_error_code_t ina230CheckAlert(const ina230_t *device, bool *hasAlerted);

#ifdef __cplusplus
}
#endif
