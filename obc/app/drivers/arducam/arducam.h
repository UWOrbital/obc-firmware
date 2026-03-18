#pragma once

#include "ov5642_config.h"
#include "ov5642.h"

#include <stdint.h>
#include <stdbool.h>

#include "obc_errors.h"
#include "obc_logging.h"

/**
 * @enum	camera_t
 * @brief	Primary or secondary camera.
 *
 * Enum containing camera identifiers and camera count.
 */
typedef enum {
  PRIMARY,
  SECONDARY,
  CAMERA_COUNT,
} camera_id_t;

/**
 * @brief Read Arducam's arduchip firmware verion.
 *        Interpreted as Bits[7:4] . Bits[3:0]
 * @param cameraID Camera ID of camera
 * @param version 1 byte buffer to store value read.
 * @return Error code. OBC_ERR_CODE_SUCCESS if successful.
 */
obc_error_code_t arducamReadFWVersion(camera_id_t cameraID, uint8_t* version);

/**
 * @brief Read Arducam's Test Register
 *
 * @param cameraID Camera ID of camera
 * @param buffer 1 byte buffer to store value read.
 * @return Error code. OBC_ERR_CODE_SUCCESS if successful.
 */
obc_error_code_t arducamReadTestReg(camera_id_t cameraID, uint8_t* buffer);

/**
 * @brief Write a value to Arducam's Test Register
 *
 * @param cameraID Camera ID of camera
 * @param value 1 byte value to be written.
 * @return Error code. OBC_ERR_CODE_SUCCESS if successful.
 */
obc_error_code_t arducamWriteTestReg(camera_id_t cameraID, uint8_t value);

/**
 * @brief Read Arducam's Sensor Power Control Register
 *        Bit[0]: Reset Sensor?
 *        Bit[1]: Standby Sensor 0 = out of standby, 1 = in standby;
 *        Bit[2]: Power Down Sensor 0 = disable power, 1 = enable power;
 *        Note: After Power Down, Sensor will need to be reinitialized
 *
 * @param cameraID Camera ID of camera
 * @param buffer 1 byte buffer to store value read.
 * @return Error code. OBC_ERR_CODE_SUCCESS if successful.
 * @return obc_error_code_t
 */
obc_error_code_t arducamReadSensorPowerControlReg(camera_id_t cameraID, uint8_t* buffer);

/**
 * @brief Read the size of the Write FIFO
 *
 * @param cameraID Camera ID of camera
 * @param fifoSize uint32_t buffer to store value
 * @return Error code. OBC_ERR_CODE_SUCCESS if successful.
 */
obc_error_code_t arducamReadFIFOSize(camera_id_t cameraID, uint32_t* fifoSize);

/**
 * @brief Initialize selected camera.
 *
 * @param cameraID Camera ID of camera
 * @return Error code. OBC_ERR_CODE_SUCCESS if successful.
 */
obc_error_code_t initCamera(camera_id_t cameraID);

/**
 * @brief Returns status of capture.
 *
 * @param cameraID Camera ID of camera
 * @return Error code. OBC_ERR_CODE_CAMERA_CAPTURE_COMPLETE if capture is complete,
 * OBC_ERR_CODE_CAMERA_CAPTURE_INCOMPLETE if capture is INCOMPLETE. Any error codes otherwise.
 */
obc_error_code_t isCaptureDone(camera_id_t cameraID);

/**
 * @brief Starts image capture on selected camera.
 *
 * @param cameraID Camera ID of camera
 * @return Error code. OBC_ERR_CODE_SUCCESS if successful.
 */
obc_error_code_t startImageCapture(camera_id_t cameraID);

/**
 * @brief Reads bufferLen number of bytes from camera FIFO.
 *
 * @param cameraID Camera ID of camera
 * @param buffer Pointer to buffer.
 * @param bufferLen Length of buffer in bytes.
 * @param bytesRead Number of bytes read.
 * @return obc_error_code_t
 */
obc_error_code_t readImage(camera_id_t cameraID, uint8_t* buffer, size_t bufferLen, size_t* bytesRead);

/**
 * @brief Power up camera sensor our of Standby.
 *
 * @param cameraID Camera ID of camera
 * @return Error code. OBC_ERR_CODE_SUCCESS if successful.
 */
obc_error_code_t wakeupCamera(camera_id_t cameraID);

/**
 * @brief Power down camera sensor into standby. Sensor configuration will NOT be lost.
 *
 * @param cameraID Camera ID of camera
 * @return Error code. OBC_ERR_CODE_SUCCESS if successful.
 */
obc_error_code_t standbyCamera(camera_id_t cameraID);

/**
 * @brief Power on camera sensor. Sensor will have to be reconfigured.
 *
 * @param cameraID Camera ID of camera
 * @return Error code. OBC_ERR_CODE_SUCCESS if successful.
 */
obc_error_code_t powerOnCamera(camera_id_t cameraID);

/**
 * @brief Power off camera sensor. Sensor configuration will be lost.
 *
 * @param cameraID Camera ID of camera
 * @return Error code. OBC_ERR_CODE_SUCCESS if successful.
 */
obc_error_code_t powerOffCamera(camera_id_t cameraID);

/**
 * @brief Reset camera sensor configuration.
 *
 * @param cameraID Camera ID of camera
 * @return obc_error_code_t
 */
obc_error_code_t resetCamera(camera_id_t cameraID);
