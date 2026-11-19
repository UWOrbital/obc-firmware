#include "obc_errors.h"
#include "obc_logging.h"

void logSetLevel(log_level_t newLogLevel) {}

obc_error_code_t logErrorCode(log_level_t msgLevel, log_file_ref_t file, uint32_t line, uint32_t errCode) {
  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t logMsg(log_level_t msgLevel, log_file_ref_t file, uint32_t line, const char *msg) {
  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t logErrorCodeFromISR(log_level_t msgLevel, log_file_ref_t file, uint32_t line, uint32_t errCode) {
  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t logMsgFromISR(log_level_t msgLevel, log_file_ref_t file, uint32_t line, const char *msg) {
  return OBC_ERR_CODE_SUCCESS;
}
