#pragma once

#include <stddef.h>
#include <stdint.h>

#include "obc_errors.h"
#include "telemetry_manager.h"

/* Telemetry file path config */
#define TELEMETRY_FILE_DIRECTORY "/telemetry/"
#define TELEMETRY_FILE_PREFIX "t_"
#define TELEMETRY_FILE_EXTENSION ".tlm"
#define TELEMETRY_FILE_NAME_MAX_LENGTH 10  // uint32_t max length
#define TELEMETRY_FILE_PATH_MAX_LENGTH                                                                  \
  sizeof(TELEMETRY_FILE_DIRECTORY) + sizeof(TELEMETRY_FILE_PREFIX) + sizeof(TELEMETRY_FILE_EXTENSION) + \
      TELEMETRY_FILE_NAME_MAX_LENGTH - 3 + 1  // -3 for the 3 %s in the format string, +1 for the null terminator

/**
 * @brief Create the telemetry directory.
 *
 * @return obc_error_code_t OBC_ERR_CODE_SUCCESS if successful, otherwise error code
 */
obc_error_code_t mkTelemetryDir(void);

/**
 * @brief Create a new telemetry file.
 *
 * @param telemBatchId The telemetry batch ID; used to create the file name
 * @param telemFileId Buffer to store the telemetry file descriptor
 * @return obc_error_code_t OBC_ERR_CODE_SUCCESS if successful, otherwise error code
 */
obc_error_code_t createTelemetryFile(uint32_t telemBatchId, int32_t *telemFileId);

/**
 * @brief Open a new telemetry file in read/write mode.
 *
 * @param telemBatchId The telemetry batch ID; used to create the file name
 * @param telemFileId Buffer to store the telemetry file descriptor
 * @return obc_error_code_t OBC_ERR_CODE_SUCCESS if successful, otherwise error code
 */
obc_error_code_t openTelemetryFileRW(uint32_t telemBatchId, int32_t *telemFileId);

/**
 * @brief Open a telemetry file in read-only mode.
 *
 * @param telemBatchId The telemetry batch ID; used to create the file name
 * @param telemFileId Buffer to store the telemetry file descriptor
 * @return obc_error_code_t OBC_ERR_CODE_SUCCESS if successful, otherwise error code
 */
obc_error_code_t openTelemetryFileRO(uint32_t telemBatchId, int32_t *telemFileId);

/**
 * @brief Close a telemetry file.
 *
 * @param telemFileId File descriptor given by Reliance Edge
 * @return obc_error_code_t OBC_ERR_CODE_SUCCESS if successful, otherwise error code
 */
obc_error_code_t closeTelemetryFile(int32_t telemFileId);

/**
 * @brief Construct the telemetry file path for the given telemetry batch ID
 *
 * @param telemBatchId The telemetry batch ID
 * @param buff Buffer to store the file path in (should be at least TELEMETRY_FILE_PATH_MAX_LENGTH bytes)
 * @param buffSize Size of the buffer (>= TELEMETRY_FILE_PATH_MAX_LENGTH)
 * @return obc_error_code_t OBC_ERR_CODE_SUCCESS if the file name was successfully obtained, error code otherwise
 */
obc_error_code_t constructTelemetryFilePath(uint32_t telemBatchId, char *buff, size_t buffSize);

/**
 * @brief Write telemetry data to file.
 *
 * @param telFileId File descriptor given by Reliance Edge
 * @param telemetryData Telemetry data to write to file
 * @return obc_error_code_t OBC_ERR_CODE_SUCCESS if successful, otherwise error code
 * @note File must already be opened for writing
 */
obc_error_code_t writeTelemetryToFile(int32_t telFileId, telemetry_data_t telemetryData);

/**
 * @brief Get the next telemetry data point from the given telemetry file
 *
 * @param telemFileId The telemetry file descriptor
 * @param telemData Buffer to store the telemetry data point in
 * @return obc_error_code_t OBC_ERR_CODE_SUCCESS if successful, error code otherwise
 * @note File must already be opened for reading
 */
obc_error_code_t readNextTelemetryFromFile(int32_t telemFileId, telemetry_data_t *telemData);

/**
 * @brief Create and open a new telemetry file in read/write mode.
 *
 * @param telemBatchId The telemetry batch ID; used to create the file name
 * @param telemFileId Buffer to store the telemetry file descriptor
 * @return obc_error_code_t
 */
obc_error_code_t createAndOpenTelemetryFileRW(uint32_t telemBatchId, int32_t *telemFileId);
