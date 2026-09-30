#pragma once

#include <FreeRTOS.h>
#include <os_portmacro.h>
#include <os_semphr.h>
#include <os_task.h>
#include <stdint.h>

#include "adc.h"
#include "obc_errors.h"
#include "obc_logging.h"
#include "reg_adc.h"

typedef enum { ADC_MODULE_1 = 0U, ADC_MODULE_2 = 1U } adc_module_t;

typedef enum { ADC_GROUP_EVENT = 0U, ADC_GROUP_1 = 1U, ADC_GROUP_2 = 2U } adc_group_t;

typedef enum {
  ADC_CHANNEL_0 = 0U,
  ADC_CHANNEL_1,
  ADC_CHANNEL_2,
  ADC_CHANNEL_3,
  ADC_CHANNEL_4,
  ADC_CHANNEL_5,
  ADC_CHANNEL_6,
  ADC_CHANNEL_7,
  ADC_CHANNEL_8,
  ADC_CHANNEL_9,
  ADC_CHANNEL_10,
  ADC_CHANNEL_11,
  ADC_CHANNEL_12,
  ADC_CHANNEL_13,
  ADC_CHANNEL_14,
  ADC_CHANNEL_15,
  ADC_CHANNEL_16,
  ADC_CHANNEL_17,
  ADC_CHANNEL_18,
  ADC_CHANNEL_19,
  ADC_CHANNEL_20,
  ADC_CHANNEL_21,
  ADC_CHANNEL_22,
  ADC_CHANNEL_23
} adc_channel_t;

/**
 * @brief Initialize the ADC bus mutex
 */
void initADC(void);

/**
 * @note Get data from a single ADC channel. ADC conversion mode channel ID should be ENABLED for all groups this is
 * called for.
 * @param adc Pointer to adc module
 * @param channel Channel number
 * @param group Group number (0-2)
 * @param reading The float to receive the analog reading
 * @param blockTime The HAL ticks to wait for mutex
 * @return obc_error_code_t
 */
obc_error_code_t adcGetSingleData(adc_module_t adc, adc_channel_t channel, adc_group_t group, uint16_t *reading,
                                  TickType_t blockTime);

/**
 * @note Get data from an entire group, must pass in an array of floats long enough to hold values for the whole group
 * @param adc Pointer to adc module
 * @param group Group number (0-2)
 * @param readings Array of floats equal to number of channels in the group
 * @param blockTime The HAL ticks to wait for mutex
 * @return obc_error_code_t
 */
obc_error_code_t adcGetGroupData(adc_module_t adc, adc_group_t group, uint16_t *readings, TickType_t blockTime);

/**
 * @note Convert digital value from ADC into analog value (e.g. for voltage)
 * @param reading Digital value from ADC
 * @param buffer Pointer to buffer to store the converted analog value
 * @return obc_error_code_t
 */
obc_error_code_t adcDigitalToAnalog(uint16_t reading, float *buffer);
