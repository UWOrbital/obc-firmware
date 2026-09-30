#include "telemetry_fs_utils.h"

#include <redposix.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "obc_assert.h"
#include "obc_errors.h"
#include "obc_logging.h"
#include "obc_reliance_fs.h"
#include "telemetry_manager.h"

obc_error_code_t mkTelemetryDir(void) {
  obc_error_code_t errCode;

  RETURN_IF_ERROR_CODE(mkDir(TELEMETRY_FILE_DIRECTORY));

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t createTelemetryFile(uint32_t telemBatchId, int32_t *telemFileId) {
  obc_error_code_t errCode;

  if (telemFileId == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  unsigned char telemFilePathBuffer[TELEMETRY_FILE_PATH_MAX_LENGTH] = {'\0'};
  RETURN_IF_ERROR_CODE(
      constructTelemetryFilePath(telemBatchId, (char *)telemFilePathBuffer, TELEMETRY_FILE_PATH_MAX_LENGTH));

  RETURN_IF_ERROR_CODE(createFile((char *)telemFilePathBuffer, telemFileId));

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t openTelemetryFileRW(uint32_t telemBatchId, int32_t *telemFileId) {
  obc_error_code_t errCode;

  if (telemFileId == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  unsigned char telemFilePathBuffer[TELEMETRY_FILE_PATH_MAX_LENGTH] = {'\0'};
  RETURN_IF_ERROR_CODE(
      constructTelemetryFilePath(telemBatchId, (char *)telemFilePathBuffer, TELEMETRY_FILE_PATH_MAX_LENGTH));

  // TODO: If we overflowed the batch ID, we should delete the duplicate file
  // However, don't delete the file if an overflow hasn't occurred (like if a system reset occurred).

  int32_t telFile = 0;
  RETURN_IF_ERROR_CODE(openFile((const char *)telemFilePathBuffer, RED_O_RDWR | RED_O_APPEND, &telFile));

  *telemFileId = telFile;

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t openTelemetryFileRO(uint32_t telemBatchId, int32_t *telemFileId) {
  obc_error_code_t errCode;

  if (telemFileId == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  unsigned char telemFilePathBuffer[TELEMETRY_FILE_PATH_MAX_LENGTH] = {'\0'};
  RETURN_IF_ERROR_CODE(
      constructTelemetryFilePath(telemBatchId, (char *)telemFilePathBuffer, TELEMETRY_FILE_PATH_MAX_LENGTH));

  int32_t telFile = 0;
  RETURN_IF_ERROR_CODE(openFile((const char *)telemFilePathBuffer, RED_O_RDONLY, &telFile));

  *telemFileId = telFile;

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t closeTelemetryFile(int32_t telemFileId) {
  obc_error_code_t errCode;

  RETURN_IF_ERROR_CODE(closeFile(telemFileId));

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t createAndOpenTelemetryFileRW(uint32_t telemBatchId, int32_t *telemFileId) {
  obc_error_code_t errCode;

  if (telemFileId == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  RETURN_IF_ERROR_CODE(createTelemetryFile(telemBatchId, telemFileId));

  RETURN_IF_ERROR_CODE(openTelemetryFileRW(telemBatchId, telemFileId));

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t constructTelemetryFilePath(uint32_t telemBatchId, char *buff, size_t buffSize) {
  if (buff == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  if (buffSize < TELEMETRY_FILE_PATH_MAX_LENGTH) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  int ret = snprintf(buff, buffSize, "%s%s%lu%s", TELEMETRY_FILE_DIRECTORY, TELEMETRY_FILE_PREFIX, telemBatchId,
                     TELEMETRY_FILE_EXTENSION);
  if (ret < 0) {
    return OBC_ERR_CODE_INVALID_FILE_NAME;
  }

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t writeTelemetryToFile(int32_t telFileId, telemetry_data_t telemetryData) {
  // Assume file is open and valid
  obc_error_code_t errCode;

  RETURN_IF_ERROR_CODE(writeFile(telFileId, &telemetryData, sizeof(telemetry_data_t)));

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t readNextTelemetryFromFile(int32_t telemFileId, telemetry_data_t *telemData) {
  // Assume file is open and valid
  obc_error_code_t errCode;

  // TODO: Read the file in reverse and filter out excess state data
  // We're only sending X past states per downlink, so we should only need to read
  // the last X states

  size_t bytesRead = 0;
  RETURN_IF_ERROR_CODE(readFile(telemFileId, telemData, sizeof(telemetry_data_t), &bytesRead));

  if (bytesRead == 0) {
    return OBC_ERR_CODE_REACHED_EOF;
  }

  // Since we only write the telemetry data struct, the number of bytes in the file
  // should be a multiple of the size of the struct.
  if (bytesRead != sizeof(telemetry_data_t)) {
    return OBC_ERR_CODE_FAILED_FILE_READ;
  }

  return OBC_ERR_CODE_SUCCESS;
}
