#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#include "obc_errors.h"
#include "obc_logging.h"

static log_level_t logLevel;
static log_output_location_t outputLocation;

void initLogger(void) {
  logLevel = LOG_DEFAULT_LEVEL;
  outputLocation = LOG_DEFAULT_OUTPUT_LOCATION;
}

void logSetLevel(log_level_t newLogLevel) { logLevel = newLogLevel; }

void logSetOutputLocation(log_output_location_t newOutputLocation) { outputLocation = newOutputLocation; }

obc_error_code_t logErrorCode(log_level_t msgLevel, const char *file, uint32_t line, uint32_t errCode) {
  if (msgLevel < logLevel) return OBC_ERR_CODE_LOG_MSG_SILENCED;

  if (file == NULL) return OBC_ERR_CODE_INVALID_ARG;

  return OBC_ERR_CODE_SUCCESS;
}
