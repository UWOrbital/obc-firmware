#pragma once

#include "obc_errors.h"
#include "obc_logging.h"
#include "ds3232_mz.h"

#include <stdio.h>

// launchpad doesn't have RTC so we should not try to add timestamps
#ifdef RM46_LAUNCHPAD
#undef LOG_DATE_TIME
#undef LOG_UNIX
#endif

typedef struct {
  log_entry_t logEntry;
  const char *file;
  uint32_t line;
#if defined(LOG_DATE_TIME)
  rtc_date_time_t timestamp;
#elif defined(LOG_UNIX)
  uint32_t timestamp;
#endif
  union {
    uint32_t errCode;
    const char *msg;
  };
} logger_event_t;

/**
 * @brief Set the output location
 *
 * @param newOutputLocation The new output location
 */
void logSetOutputLocation(log_output_location_t newOutputLocation);

/**
 * @brief Set the output format (text or compact binary)
 *
 * @param newOutputFormat The new output format
 */
void logSetOutputFormat(log_output_format_t newOutputFormat);
