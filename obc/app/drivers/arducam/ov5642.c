#include "ov5642.h"

#include "ov5642_config.h"
#include "obc_logging.h"
#include "obc_i2c_io.h"

#include <os_semphr.h>

// Camera Img Sensor (I2C) defines
#define CAM_I2C_ADDR 0x3C
#define I2C_MUTEX_TIMEOUT portMAX_DELAY
#define I2C_TRANSFER_TIMEOUT pdMS_TO_TICKS(100)
#define OV5642_REG_MUTEX_TIMEOUT portMAX_DELAY

#define CHIP_ID_HIGH_BYTE_REG 0x300A
#define CHIP_ID_LOW_BYTE_REG 0x300B
#define SYSTEM_CONTROL00_REG 0x3008
#define TIMING_CONTROL_18_REG 0x3818
#define TIMING_HS_HIGH_REG 0x3800
#define TIMING_HS_LOW_REG 0x3801
#define COMPRESSION_CTRL07_REG 0x4407
#define ISP_CONTROL_00_REG 0x5000
#define LENC_BRV_SCALE_HIGH_REG 0x5888
#define LENC_BRV_SCALE_LOW_REG 0x5889

#define SOFTWARE_RESET_MODE 0x80

// Need a mutex because some registers are multipurpose and require reading and writing to be atomic
static SemaphoreHandle_t ov5642RegMutex = NULL;

obc_error_code_t initOV5642(void) {
  ov5642RegMutex = xSemaphoreCreateMutex();

  return OBC_ERR_CODE_SUCCESS;
}

/**
 * @brief Read 8 bits from a 16 bit register over I2C
 * @param regID Register address to write to
 * @param regDat Data to send
 * @return Error code indicating if the write was successful
 */
static obc_error_code_t camWriteSensorReg16_8(uint32_t regID, uint8_t regDat);

/**
 * @brief Write 8 bits to a 16 bit register over I2C
 * @param regID Register address to read from
 * @param regDat Buffer to store received data
 * @return Error code indicating if the read was successful
 */
static obc_error_code_t camReadSensorReg16_8(uint32_t regID, uint8_t* regDat);

/**
 * @brief Write to a list of registers over I2C
 * @param reglist List of registers and data to write
 * @return Error code indicating if the writes were successful
 */
static obc_error_code_t camWriteSensorRegList16_8(const sensor_config_t reglist[], size_t reglistLen);

static obc_error_code_t camWriteSensorReg16_8(uint32_t regID, uint8_t regDat) {
  uint8_t reg_tx_data[3] = {(regID >> 8), (regID & 0x00FF), regDat};
  return i2cSendTo(CAM_I2C_ADDR, 3, reg_tx_data, I2C_MUTEX_TIMEOUT, I2C_TRANSFER_TIMEOUT);
}

static obc_error_code_t camReadSensorReg16_8(uint32_t regID, uint8_t* regDat) {
  obc_error_code_t errCode;
  uint8_t reg_id_tx_data[2] = {(regID >> 8), (regID & 0x00FF)};
  RETURN_IF_ERROR_CODE(i2cSendTo(CAM_I2C_ADDR, 2, reg_id_tx_data, I2C_MUTEX_TIMEOUT, I2C_TRANSFER_TIMEOUT));
  RETURN_IF_ERROR_CODE(i2cReceiveFrom(CAM_I2C_ADDR, 1, regDat, I2C_MUTEX_TIMEOUT, I2C_TRANSFER_TIMEOUT));
  return errCode;
}

static obc_error_code_t camWriteSensorRegList16_8(const sensor_config_t reglist[], size_t reglistLen) {
  obc_error_code_t errCode;

  for (size_t i = 0; i < reglistLen; i++) {
    RETURN_IF_ERROR_CODE(camWriteSensorReg16_8(reglist[i].reg, reglist[i].val));
  }

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t applyCamPreviewConfig(void) {
  if (xSemaphoreTake(ov5642RegMutex, OV5642_REG_MUTEX_TIMEOUT) != pdTRUE) {
    return OBC_ERR_CODE_MUTEX_TIMEOUT;
  }

  obc_error_code_t errCode;
  RETURN_AND_GIVE_IF_ERROR_CODE(camWriteSensorRegList16_8(getCamPreviewConfig(), PREVIEW_CONFIG_LEN), ov5642RegMutex);

  xSemaphoreGive(ov5642RegMutex);
  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t applyCamCaptureConfig(void) {
  if (xSemaphoreTake(ov5642RegMutex, OV5642_REG_MUTEX_TIMEOUT) != pdTRUE) {
    return OBC_ERR_CODE_MUTEX_TIMEOUT;
  }

  obc_error_code_t errCode;
  RETURN_AND_GIVE_IF_ERROR_CODE(camWriteSensorRegList16_8(getCamCaptureConfig(), JPEG_CONFIG_LEN), ov5642RegMutex);

  xSemaphoreGive(ov5642RegMutex);
  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t applyCamResolutionConfig(void) {
  if (xSemaphoreTake(ov5642RegMutex, OV5642_REG_MUTEX_TIMEOUT) != pdTRUE) {
    return OBC_ERR_CODE_MUTEX_TIMEOUT;
  }

  obc_error_code_t errCode;
  RETURN_AND_GIVE_IF_ERROR_CODE(camWriteSensorRegList16_8(getCamResolutionConfig(), RES_320_240_CONFIG_LEN),
                                ov5642RegMutex);

  xSemaphoreGive(ov5642RegMutex);
  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t ov5642GetChipID(uint16_t* buffer) {
  if (xSemaphoreTake(ov5642RegMutex, OV5642_REG_MUTEX_TIMEOUT) != pdTRUE) {
    return OBC_ERR_CODE_MUTEX_TIMEOUT;
  }

  obc_error_code_t errCode;
  uint8_t cam_id[2] = {0};
  RETURN_AND_GIVE_IF_ERROR_CODE(camReadSensorReg16_8(CHIP_ID_HIGH_BYTE_REG, &cam_id[0]), ov5642RegMutex);
  RETURN_AND_GIVE_IF_ERROR_CODE(camReadSensorReg16_8(CHIP_ID_LOW_BYTE_REG, &cam_id[1]), ov5642RegMutex);

  *buffer = ((uint16_t)cam_id[0] << 8) | cam_id[1];

  xSemaphoreGive(ov5642RegMutex);
  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t ov5642Reset(void) {
  if (xSemaphoreTake(ov5642RegMutex, OV5642_REG_MUTEX_TIMEOUT) != pdTRUE) {
    return OBC_ERR_CODE_MUTEX_TIMEOUT;
  }

  obc_error_code_t errCode;
  RETURN_AND_GIVE_IF_ERROR_CODE(camWriteSensorReg16_8(SYSTEM_CONTROL00_REG, SOFTWARE_RESET_MODE), ov5642RegMutex);

  xSemaphoreGive(ov5642RegMutex);
  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t ov5642SetMirror(bool enabled) {
  if (xSemaphoreTake(ov5642RegMutex, OV5642_REG_MUTEX_TIMEOUT) != pdTRUE) {
    return OBC_ERR_CODE_MUTEX_TIMEOUT;
  }

  obc_error_code_t errCode;
  uint8_t timingControl18;
  RETURN_AND_GIVE_IF_ERROR_CODE(camReadSensorReg16_8(TIMING_CONTROL_18_REG, &timingControl18), ov5642RegMutex);
  timingControl18 &= 0xBF;  // clear mirror bit
  timingControl18 |= (enabled << 6);
  RETURN_AND_GIVE_IF_ERROR_CODE(camWriteSensorReg16_8(TIMING_CONTROL_18_REG, timingControl18), ov5642RegMutex);

  xSemaphoreGive(ov5642RegMutex);
  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t ov5642SetVerticalFlip(bool enabled) {
  if (xSemaphoreTake(ov5642RegMutex, OV5642_REG_MUTEX_TIMEOUT) != pdTRUE) {
    return OBC_ERR_CODE_MUTEX_TIMEOUT;
  }

  obc_error_code_t errCode;

  uint8_t timingControl18;
  RETURN_AND_GIVE_IF_ERROR_CODE(camReadSensorReg16_8(TIMING_CONTROL_18_REG, &timingControl18), ov5642RegMutex);
  timingControl18 &= 0xDF;  // clear vertical flip bit
  timingControl18 |= (enabled << 5);
  RETURN_AND_GIVE_IF_ERROR_CODE(camWriteSensorReg16_8(TIMING_CONTROL_18_REG, timingControl18), ov5642RegMutex);

  xSemaphoreGive(ov5642RegMutex);
  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t ov5642SetHorizontalStart(uint16_t horizontalStart) {
  if (horizontalStart > 0xFFF) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  if (xSemaphoreTake(ov5642RegMutex, OV5642_REG_MUTEX_TIMEOUT) != pdTRUE) {
    return OBC_ERR_CODE_MUTEX_TIMEOUT;
  }

  obc_error_code_t errCode;
  RETURN_AND_GIVE_IF_ERROR_CODE(camWriteSensorReg16_8(TIMING_HS_HIGH_REG, horizontalStart >> 8), ov5642RegMutex);
  RETURN_AND_GIVE_IF_ERROR_CODE(camWriteSensorReg16_8(TIMING_HS_LOW_REG, horizontalStart), ov5642RegMutex);

  xSemaphoreGive(ov5642RegMutex);
  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t ov5642SetQuantizationScale(uint8_t quantizationScale) {
  if (quantizationScale > 0x3F) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  if (xSemaphoreTake(ov5642RegMutex, OV5642_REG_MUTEX_TIMEOUT) != pdTRUE) {
    return OBC_ERR_CODE_MUTEX_TIMEOUT;
  }

  obc_error_code_t errCode;
  uint8_t compressionControl7;
  RETURN_AND_GIVE_IF_ERROR_CODE(camReadSensorReg16_8(COMPRESSION_CTRL07_REG, &compressionControl7), ov5642RegMutex);
  compressionControl7 &= 0xC0;  // clear quantization scale bits
  compressionControl7 |= quantizationScale;
  RETURN_AND_GIVE_IF_ERROR_CODE(camWriteSensorReg16_8(COMPRESSION_CTRL07_REG, compressionControl7), ov5642RegMutex);

  xSemaphoreGive(ov5642RegMutex);
  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t ov5642SetLencCorrection(bool enabled) {
  if (xSemaphoreTake(ov5642RegMutex, OV5642_REG_MUTEX_TIMEOUT) != pdTRUE) {
    return OBC_ERR_CODE_MUTEX_TIMEOUT;
  }

  obc_error_code_t errCode;
  uint8_t ispControl00;
  RETURN_AND_GIVE_IF_ERROR_CODE(camReadSensorReg16_8(ISP_CONTROL_00_REG, &ispControl00), ov5642RegMutex);
  ispControl00 &= 0x7F;  // clear lenc correction bit
  ispControl00 |= enabled << 7;
  RETURN_AND_GIVE_IF_ERROR_CODE(camWriteSensorReg16_8(ISP_CONTROL_00_REG, ispControl00), ov5642RegMutex);

  xSemaphoreGive(ov5642RegMutex);
  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t ov5642SetLencBrvScale(uint16_t lencBrvScale) {
  if (lencBrvScale > 0x1FF) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  if (xSemaphoreTake(ov5642RegMutex, OV5642_REG_MUTEX_TIMEOUT) != pdTRUE) {
    return OBC_ERR_CODE_MUTEX_TIMEOUT;
  }

  obc_error_code_t errCode;
  RETURN_AND_GIVE_IF_ERROR_CODE(camWriteSensorReg16_8(TIMING_HS_HIGH_REG, lencBrvScale >> 8), ov5642RegMutex);
  RETURN_AND_GIVE_IF_ERROR_CODE(camWriteSensorReg16_8(TIMING_HS_LOW_REG, lencBrvScale), ov5642RegMutex);

  xSemaphoreGive(ov5642RegMutex);
  return OBC_ERR_CODE_SUCCESS;
}
