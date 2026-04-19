#pragma once

#include "obc_errors.h"

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Initializes mutex for ov5642
 *
 * @return Error code. OBC_ERR_CODE_SUCCESS if successful.
 */
obc_error_code_t initOV5642(void);

/**
 * @brief Applies camera preview configuration array
 *
 * @return Error code. OBC_ERR_CODE_SUCCESS if successful.
 */
obc_error_code_t applyCamPreviewConfig(void);

/**
 * @brief Applies camera capture configuration array
 *
 * @return Error code. OBC_ERR_CODE_SUCCESS if successful.
 */
obc_error_code_t applyCamCaptureConfig(void);

/**
 * @brief Applies camera resolution configuration array
 *
 * @return Error code. OBC_ERR_CODE_SUCCESS if successful.
 */
obc_error_code_t applyCamResolutionConfig(void);

/**
 * @brief Packs ChipID into provided buffer
 *
 * @param buffer The buffer to pack the value into.
 * @return Error code. OBC_ERR_CODE_SUCCESS if successful.
 */
obc_error_code_t ov5642GetChipID(uint16_t* buffer);

/**
 * @brief Resets ov5642.
 *
 * @return Error code. OBC_ERR_CODE_SUCCESS if successful.
 */
obc_error_code_t ov5642Reset(void);

/**
 * @brief Sets the OV5642 Mirror registry settings.
 *
 * @param enabled Whether or not to mirror camera output
 * @return Error code. OBC_ERR_CODE_SUCCESS if successful.
 */
obc_error_code_t ov5642SetMirror(bool enabled);

/**
 * @brief Sets the OV5642 Vertical Flip registry settings.
 *
 * @param enabled Whether or not to flip camera output
 * @return Error code. OBC_ERR_CODE_SUCCESS if successful.
 */
obc_error_code_t ov5642SetVerticalFlip(bool enabled);

/**
 * @brief Sets the OV5642 Horizontal Start registry settings.
 *
 * @param horizontalStart Pixel value of horizontal start for windowing/cropping
 * @return Error code. OBC_ERR_CODE_SUCCESS if successful.
 */
obc_error_code_t ov5642SetHorizontalStart(uint16_t horizontalStart);

/**
 * @brief Sets the OV5642 Quantization Scale registry settings.
 *
 * @param quantizationScale New value of Quantization Scale
 * @return Error code. OBC_ERR_CODE_SUCCESS if successful.
 */
obc_error_code_t ov5642SetQuantizationScale(uint8_t quantizationScale);

/**
 * @brief Sets the OV5642 Lenc Correction registry settings.
 *
 * @param enabled Whether or not to enabled Lenc Correction
 * @return Error code. OBC_ERR_CODE_SUCCESS if successful.
 */
obc_error_code_t ov5642SetLencCorrection(bool enabled);

/**
 * @brief Sets the OV5642 Lenc Brv Scale registry settings.
 *
 * @param lencBrvScale Reciprocal of vertical step at B/R channel
 * @return Error code. OBC_ERR_CODE_SUCCESS if successful.
 */
obc_error_code_t ov5642SetLencBrvScale(uint16_t lencBrvScale);
