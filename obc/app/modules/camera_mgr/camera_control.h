#pragma once
#include "arducam.h"

/**
 * @brief Sets the SPI bus and I2C mux for a specific camera
 *
 * @param camID Camera ID
 * @return Error code. OBC_ERR_CODE_SUCCESS if successful.
 */
obc_error_code_t selectCamera(camera_id_t camID);

/**
 * @brief Configures ov5642 sensor to defaults
 *
 * @return Error code. OBC_ERR_CODE_SUCCESS if successful.
 */
obc_error_code_t camConfigureSensor(void);
