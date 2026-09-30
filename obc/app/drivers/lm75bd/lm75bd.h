#pragma once

#include <stdint.h>

#include "obc_errors.h"

/* LM75BD I2C Device Address */
#define LM75BD_OBC_I2C_ADDR 0x4FU /* (0x9U << 3) | 0x7 */

/* LM75BD Configuration Values */
#define LM75BD_DEV_OP_MODE_NORMAL 0x00U
#define LM75BD_DEV_OP_MODE_SHUTDOWN 0x01U

#define LM75BD_OS_POL_ACTIVE_LOW 0x00U
#define LM75BD_OS_POL_ACTIVE_HIGH 0x01U

#define LM75BD_OS_OP_MODE_COMP 0x00U
#define LM75BD_OS_OP_MODE_INT 0x01U

/**
 * @struct Configuration struct for LM75BD,118 temperature sensor
 *
 * @param devAddr I2C address of the LM75BD
 * @param osFaultQueueSize Number of consecutive OS faults until OS output is activated (1, 2, 4, or 6)
 * @param osPolarity OS output polarity, 0 = active low, 1 = active high
 * @param osOperationMode OS output operation mode, 0 = comparator, 1 = interrupt
 * @param devOperationMode Device operation mode, 0 = normal, 1 = shutdown
 * @param overTempThreshold Overtemperature shutdown threshold, in degrees Celsius
 * @param hysteresisThreshold Hysteresis threshold, in degrees Celsius
 */
typedef struct {
  uint8_t devAddr;
  uint8_t osFaultQueueSize;
  uint8_t osPolarity;
  uint8_t osOperationMode;
  uint8_t devOperationMode;
  float overTempThresholdCelsius;
  float hysteresisThresholdCelsius;
} lm75bd_config_t;

/**
 * @brief Initialize the LM75BD
 *
 * @param config Configuration struct for LM75BD
 * @return OBC_ERR_CODE_SUCCESS if successful, error code otherwise
 */
obc_error_code_t lm75bdInit(lm75bd_config_t *config);

/**
 * @brief Read the temperature from the LM75BD
 *
 * @param temp Pointer to float to store the temperature in degrees Celsius
 * @return OBC_ERR_CODE_SUCCESS if successful, error code otherwise
 */
obc_error_code_t readTempLM75BD(uint8_t devAddr, float *temp);

/**
 * @brief Read the configuration register from the LM75BD
 *
 * @param config Configuration struct for LM75BD
 * @return OBC_ERR_CODE_SUCCESS if successful, error code otherwise
 */
obc_error_code_t readConfigLM75BD(lm75bd_config_t *config);

/**
 * @brief Write to the configuration register from the LM75BD
 *
 * @param devAddr I2C address of the LM75BD
 * @param osFaultQueueSize Number of consecutive OS faults until OS output is activated (1, 2, 4, or 6)
 * @param osPolarity OS output polarity, 0 = active low, 1 = active high
 * @param osOperationMode OS output operation mode, 0 = comparator, 1 = interrupt
 * @param devOperationMode Device operation mode, 0 = normal, 1 = shutdown
 * @return OBC_ERR_CODE_SUCCESS if successful, error code otherwise
 */
obc_error_code_t writeConfigLM75BD(uint8_t devAddr, uint8_t osFaultQueueSize, uint8_t osPolarity,
                                   uint8_t osOperationMode, uint8_t devOperationMode);

/**
 * @brief Get the hysteresis threshold from the LM75BD
 *
 * @param devAddr I2C address of the LM75BD
 * @param hysteresisThresholdCelsius Hysteresis threshold, in degrees Celsius
 * @return OBC_ERR_CODE_SUCCESS if successful, error code otherwise
 */
obc_error_code_t readThystLM75BD(uint8_t devAddr, float *hysteresisThresholdCelsius);

/**
 * @brief Set the hysteresis threshold for the LM75BD
 *
 * @param devAddr I2C address of the LM75BD
 * @param hysteresisThresholdCelsius Hysteresis threshold, in degrees Celsius (<= 127.5)
 * @return OBC_ERR_CODE_SUCCESS if successful, error code otherwise
 */
obc_error_code_t writeThystLM75BD(uint8_t devAddr, float hysteresisThresholdCelsius);

/**
 * @brief Get the overtemperature shutdown threshold from the LM75BD
 *
 * @param devAddr I2C address of the LM75BD
 * @param overTempThresholdCelsius Overtemperature shutdown threshold, in degrees Celsius
 * @return OBC_ERR_CODE_SUCCESS if successful, error code otherwise
 */
obc_error_code_t readTosLM75BD(uint8_t devAddr, float *overTempThresholdCelsius);

/**
 * @brief Set the overtemperature shutdown threshold for the LM75BD
 *
 * @param devAddr I2C address of the LM75BD
 * @param overTempThresholdCelsius Overtemperature shutdown threshold, in degrees Celsius (<= 127.5)
 * @return OBC_ERR_CODE_SUCCESS if successful, error code otherwise
 */
obc_error_code_t writeTosLM75BD(uint8_t devAddr, float overTempThresholdCelsius);

/**
 * @brief Handle an OS interrupt from the LM75BD
 *
 * @param devAddr I2C address of the LM75BD
 */
void osHandlerLM75BD(uint8_t devAddr);
