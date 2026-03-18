#pragma once

#include "stdint.h"

#define PREVIEW_CONFIG_LEN 583
#define JPEG_CONFIG_LEN 72
#define RES_320_240_CONFIG_LEN 27

/**
 * @struct sensor_config_t
 * @brief	Sensor config struct, reg is the address to write config to and val is the value to write
 */
typedef struct {
  uint16_t reg;
  uint8_t val;
} sensor_config_t;

/**
 * @brief Access camera preview configuration arrays
 * @return Pointer to config array
 */
sensor_config_t* getCamPreviewConfig(void);

/**
 * @brief Access camera capture configuration arrays
 * @return Pointer to config array
 */
sensor_config_t* getCamCaptureConfig(void);

/**
 * @brief Access camera resolution configuration arrays
 * @return Pointer to config array
 */
sensor_config_t* getCamResolutionConfig(void);
