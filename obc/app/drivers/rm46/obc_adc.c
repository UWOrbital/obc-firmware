#include "obc_adc.h"

#include <FreeRTOS.h>
#include <os_portmacro.h>
#include <os_semphr.h>
#include <os_task.h>

#include "adc.h"
#include "obc_errors.h"
#include "obc_logging.h"
#include "reg_adc.h"

static SemaphoreHandle_t adcConversionMutex = NULL;
static StaticSemaphore_t adcConversionMutexBuffer;

// Should be configured to max FIFO size for all groups, so that an adcData_t array of a proper size can be made
#define MAX_GROUP_SIZE 16U

// Assumes all groups have same resolution for simplicity. Could make this more flexible/an enum.
#define RESOLUTION 12U

// VCCAD is 3.3V
#define REF_VOLTAGE_HIGH 3.3f
#define REF_VOLTAGE_LOW 0.0f

// Arbitrary
#define ADC_MAX_WAIT_CYCLES 1000

#define ADC_NUM_GROUPS 3

typedef enum {
  ADC1_GROUP0_NUM_CHANNELS = 16U,
  ADC1_GROUP1_NUM_CHANNELS = 16U,
  ADC1_GROUP2_NUM_CHANNELS = 16U,
  ADC2_GROUP0_NUM_CHANNELS = 16U,
  ADC2_GROUP1_NUM_CHANNELS = 16U,
  ADC2_GROUP2_NUM_CHANNELS = 16U
} adc_group_channels_t;

#define ADC_INDEX(module, group) ((module) * ADC_NUM_GROUPS + (group))

const uint32_t adcGroupLengths[6] = {ADC1_GROUP0_NUM_CHANNELS, ADC1_GROUP1_NUM_CHANNELS, ADC1_GROUP2_NUM_CHANNELS,
                                     ADC2_GROUP0_NUM_CHANNELS, ADC2_GROUP1_NUM_CHANNELS, ADC2_GROUP2_NUM_CHANNELS};

void initADC(void) {
  adcInit();
  if (adcConversionMutex == NULL) {
    adcConversionMutex = xSemaphoreCreateMutexStatic(&adcConversionMutexBuffer);
  }
  ASSERT(adcConversionMutex != NULL);
}

static obc_error_code_t adcGetGroupReadings(adc_module_t adc, adc_group_t group, adcData_t *data,
                                            TickType_t blockTime) {
  if (data == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  if (xSemaphoreTake(adcConversionMutex, blockTime) != pdTRUE) {
    return OBC_ERR_CODE_MUTEX_TIMEOUT;
  }

  adcBASE_t *adcReg = (adc == ADC_MODULE_1) ? adcREG1 : adcREG2;

  adcStartConversion(adcReg, group);

  uint16_t totalAttempts = 0;
  while (!adcIsConversionComplete(adcReg, group) && totalAttempts < ADC_MAX_WAIT_CYCLES) {
    totalAttempts++;
  }

  if (totalAttempts >= ADC_MAX_WAIT_CYCLES) {
    xSemaphoreGive(adcConversionMutex);
    return OBC_ERR_CODE_ADC_FAILURE;
  }

  adcStopConversion(adcReg, group);

  adcGetData(adcReg, group, data);

  xSemaphoreGive(adcConversionMutex);

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t adcDigitalToAnalog(uint16_t reading, float *buffer) {
  if (buffer == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }
  *buffer = (float)(reading) * (REF_VOLTAGE_HIGH - REF_VOLTAGE_LOW) / ((float)(1 << RESOLUTION) + REF_VOLTAGE_LOW);
  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t adcGetSingleData(adc_module_t adc, adc_channel_t channel, adc_group_t group, uint16_t *reading,
                                  TickType_t blockTime) {
  if (reading == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  if (adcConversionMutex == NULL) {
    return OBC_ERR_CODE_INVALID_STATE;
  }

  adcData_t adcData[MAX_GROUP_SIZE];

  obc_error_code_t errCode;
  RETURN_IF_ERROR_CODE(adcGetGroupReadings(adc, group, adcData, blockTime));

  uint8_t index = ADC_INDEX(adc, group);
  uint32_t groupSize = adcGroupLengths[index];

  for (uint32_t i = 0; i < groupSize; i++) {
    if (adcData[i].id == channel) {
      *reading = adcData[i].value;
      return OBC_ERR_CODE_SUCCESS;
    }
  }

  return OBC_ERR_CODE_ADC_INVALID_CHANNEL;
}

obc_error_code_t adcGetGroupData(adc_module_t adc, adc_group_t group, uint16_t *readings, TickType_t blockTime) {
  if (readings == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  if (adcConversionMutex == NULL) {
    return OBC_ERR_CODE_INVALID_STATE;
  }

  adcData_t adcData[MAX_GROUP_SIZE];

  obc_error_code_t errCode;
  RETURN_IF_ERROR_CODE(adcGetGroupReadings(adc, group, adcData, blockTime));

  uint8_t index = ADC_INDEX(adc, group);
  uint32_t groupSize = adcGroupLengths[index];

  for (uint32_t i = 0; i < groupSize; i++) {
    readings[i] = adcData[i].value;
  }

  return OBC_ERR_CODE_SUCCESS;
}
