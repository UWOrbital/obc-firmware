#include "logger.h"

#include <FreeRTOS.h>
#include <FreeRTOSConfig.h>
#include <os_queue.h>
#include <redposix.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys_common.h>

#include "obc_errors.h"
#include "obc_logging.h"
#include "obc_print.h"
#include "obc_time.h"

#define LOG_FILE_NAME "log.log"

#define MAX_MSG_SIZE 128U
#define MAX_FNAME_LINENUM_SIZE 150U
// Extra 10 for the small extra pieces in "%s - %s\r\n"
#define MAX_LOG_SIZE (MAX_MSG_SIZE + MAX_FNAME_LINENUM_SIZE + 10U)

#define UART_MUTEX_BLOCK_TIME portMAX_DELAY

#if defined(LOG_DATE_TIME)
// When logging over the air to ground station, we should timestamp using UNIX and have a tool to convert unix to date
// time on the GS side for all the logs but for development it's easier to log as date time
#define GET_TIMESTAMP getCurrentDateTime()
#define GET_TIMESTAMP_FROM_ISR getCurrentDateTimeinISR()
#elif defined(LOG_UNIX)
#define GET_TIMESTAMP getCurrentUnixTime()
#define GET_TIMESTAMP_FROM_ISR getCurrentUnixTimeInISR()
#endif

static const char *LEVEL_STRINGS[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};

static log_level_t logLevel;
static log_output_location_t outputLocation;

#define LOGGER_QUEUE_LENGTH 10U
#define LOGGER_QUEUE_ITEM_SIZE sizeof(logger_event_t)
#define LOGGER_QUEUE_RX_WAIT_PERIOD portMAX_DELAY
#define LOGGER_QUEUE_TX_WAIT_PERIOD 0

static QueueHandle_t loggerQueueHandle = NULL;
static StaticQueue_t loggerQueue;
static uint8_t loggerQueueStack[LOGGER_QUEUE_LENGTH * LOGGER_QUEUE_ITEM_SIZE];

/**
 * @brief Sends an event to the logger queue
 *
 * @param event Pointer to the event to send
 * @return obc_error_code_t OBC_ERR_CODE_SUCCESS if the packet was sent to the queue
 */
static obc_error_code_t sendToLoggerQueue(logger_event_t *event, size_t blockTimeTicks);

/**
 * @brief Sends an event to the logger queue from an ISR
 *
 * @param event Pointer to the event to send
 * @return obc_error_code_t OBC_ERR_CODE_SUCCESS if the packet was sent to the queue
 */
static obc_error_code_t sendToLoggerQueueFromISR(logger_event_t *event);

void logSetLevel(log_level_t newLogLevel) { logLevel = newLogLevel; }

void obcTaskInitLogger(void) {
  ASSERT((loggerQueueStack != NULL) && (&loggerQueue != NULL));
  if (loggerQueueHandle == NULL) {
    loggerQueueHandle = xQueueCreateStatic(LOGGER_QUEUE_LENGTH, LOGGER_QUEUE_ITEM_SIZE, loggerQueueStack, &loggerQueue);
  }

  outputLocation = LOG_DEFAULT_OUTPUT_LOCATION;
  logLevel = LOG_DEFAULT_LEVEL;
}

void obcTaskFunctionLogger(void *pvParameters) {
  char *fname = LOG_FILE_NAME;
  while (1) {
    logger_event_t queueMsg;
    if (xQueueReceive(loggerQueueHandle, &queueMsg, LOGGER_QUEUE_RX_WAIT_PERIOD) != pdPASS) {
      continue;
    }
    if (queueMsg.logEntry.logLevel > LOG_FATAL) {
      LOG_ERROR_CODE(OBC_ERR_CODE_UNSUPPORTED_EVENT);
      continue;
    }
    if (queueMsg.file == NULL) {
      LOG_ERROR_CODE(OBC_ERR_CODE_UNSUPPORTED_EVENT);
      continue;
    }

    // File & line number
    char infobuf[MAX_FNAME_LINENUM_SIZE] = {0};
    int ret = 0;

#if defined(LOG_DATE_TIME)
    const rtc_date_time_t *currDate = &queueMsg.timestamp;
    ret = snprintf(infobuf, MAX_FNAME_LINENUM_SIZE, "%02u-%02u-%02u_%02u-%02u-%02u %-5s -> %s:%lu", currDate->date.year,
                   currDate->date.month, currDate->date.date, currDate->time.hours, currDate->time.minutes,
                   currDate->time.seconds, LEVEL_STRINGS[queueMsg.logEntry.logLevel], queueMsg.file, queueMsg.line);
#elif defined(LOG_UNIX)
    ret = snprintf(infobuf, MAX_FNAME_LINENUM_SIZE, "%u %-5s -> %s:%lu", queueMsg.timestamp,
                   LEVEL_STRINGS[queueMsg.logEntry.logLevel], queueMsg.file, queueMsg.line);
#else
    ret = snprintf(infobuf, MAX_FNAME_LINENUM_SIZE, "%-5s -> %s:%lu", LEVEL_STRINGS[queueMsg.logEntry.logLevel],
                   queueMsg.file, queueMsg.line);
#endif

    if (ret < 0) {
      LOG_ERROR_CODE(OBC_ERR_CODE_INVALID_ARG);
      continue;
    }
    if ((uint32_t)ret >= MAX_FNAME_LINENUM_SIZE) {
      LOG_ERROR_CODE(OBC_ERR_CODE_BUFF_TOO_SMALL);
      continue;
    }
    char logBuf[MAX_LOG_SIZE] = {0};
    int logBufLen = 0;
    switch (queueMsg.logEntry.logType) {
      case LOG_TYPE_ERROR_CODE:
        // Prepare entire output
        logBufLen = snprintf(logBuf, MAX_LOG_SIZE, "%s - %lu\r\n", infobuf, queueMsg.errCode);
        if (logBufLen < 0) {
          LOG_ERROR_CODE(OBC_ERR_CODE_INVALID_ARG);
          continue;
        }
        if ((uint32_t)logBufLen >= MAX_LOG_SIZE) {
          LOG_ERROR_CODE(OBC_ERR_CODE_BUFF_TOO_SMALL);
          continue;
        }
        break;
      case LOG_TYPE_MSG:
        // if it isnt an error log, it has a string to be logged
        // Prepare entire output
        logBufLen = snprintf(logBuf, MAX_LOG_SIZE, "%s - %s\r\n", infobuf, queueMsg.msg);
        if (logBufLen < 0) {
          LOG_ERROR_CODE(OBC_ERR_CODE_INVALID_ARG);
          continue;
        }
        if ((uint32_t)logBufLen >= MAX_LOG_SIZE) {
          LOG_ERROR_CODE(OBC_ERR_CODE_BUFF_TOO_SMALL);
          continue;
        }
        break;
      default:
        LOG_ERROR_CODE(OBC_ERR_CODE_UNSUPPORTED_EVENT);
        continue;
    }
    if (outputLocation == LOG_TO_SDCARD) {
      int32_t fdescriptor = red_open(fname, RED_O_WRONLY | RED_O_APPEND | RED_O_CREAT);
      if (fdescriptor == -1) {
        continue;
      }
      if (red_write(fdescriptor, logBuf, logBufLen) == -1) {
        continue;
      }
      if (red_close(fdescriptor) == -1) {
        continue;
      }
    } else {
      sciPrintText((unsigned char *)logBuf, logBufLen, UART_MUTEX_BLOCK_TIME);
    }
  }
}

static obc_error_code_t sendToLoggerQueue(logger_event_t *event, size_t blockTimeTicks) {
  if (loggerQueueHandle == NULL) {
    return OBC_ERR_CODE_INVALID_STATE;
  }

  if (event == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  if (xQueueSend(loggerQueueHandle, (void *)event, blockTimeTicks) == pdPASS) {
    return OBC_ERR_CODE_SUCCESS;
  }

  return OBC_ERR_CODE_QUEUE_FULL;
}

static obc_error_code_t sendToLoggerQueueFromISR(logger_event_t *event) {
  if (loggerQueueHandle == NULL) {
    return OBC_ERR_CODE_INVALID_STATE;
  }

  if (event == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }
  BaseType_t xHigherPriorityTaskAwoken = pdFALSE;

  xQueueSendFromISR(loggerQueueHandle, (void *)event, &xHigherPriorityTaskAwoken);

  portYIELD_FROM_ISR(xHigherPriorityTaskAwoken);

  return OBC_ERR_CODE_QUEUE_FULL;
}

void logSetOutputLocation(log_output_location_t newOutputLocation) { outputLocation = newOutputLocation; }

obc_error_code_t logErrorCode(log_level_t msgLevel, const char *file, uint32_t line, uint32_t errCode) {
  if (msgLevel < logLevel) {
    return OBC_ERR_CODE_LOG_MSG_SILENCED;
  }

  if (file == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  logger_event_t logEvent = {.logEntry = {.logType = LOG_TYPE_ERROR_CODE, .logLevel = msgLevel},
                             .file = file,
                             .line = line,
                             .errCode = errCode};

#if defined(LOG_DATE_TIME) || defined(LOG_UNIX)
  logEvent.timestamp = GET_TIMESTAMP;
#endif

  // send the event to the logger queue and don't try to log any error that occurs
  return sendToLoggerQueue(&logEvent, LOGGER_QUEUE_TX_WAIT_PERIOD);
}

obc_error_code_t logMsg(log_level_t msgLevel, const char *file, uint32_t line, const char *msg) {
  if (msgLevel < logLevel) {
    return OBC_ERR_CODE_LOG_MSG_SILENCED;
  }

  if (file == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  if (msg == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  logger_event_t logEvent = {
      .logEntry = {.logType = LOG_TYPE_MSG, .logLevel = msgLevel}, .file = file, .line = line, .msg = msg};

#if defined(LOG_DATE_TIME) || defined(LOG_UNIX)
  logEvent.timestamp = GET_TIMESTAMP;
#endif

  return sendToLoggerQueue(&logEvent, LOGGER_QUEUE_TX_WAIT_PERIOD);
}

obc_error_code_t logErrorCodeFromISR(log_level_t msgLevel, const char *file, uint32_t line, uint32_t errCode) {
  if (msgLevel < logLevel) {
    return OBC_ERR_CODE_LOG_MSG_SILENCED;
  }

  if (file == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  logger_event_t logEvent = {.logEntry = {.logType = LOG_TYPE_ERROR_CODE, .logLevel = msgLevel},
                             .file = file,
                             .line = line,
                             .errCode = errCode};

#if defined(LOG_DATE_TIME) || defined(LOG_UNIX)
  logEvent.timestamp = GET_TIMESTAMP_FROM_ISR;
#endif

  // send the event to the logger queue and don't try to log any error that occurs
  return sendToLoggerQueueFromISR(&logEvent);
}

obc_error_code_t logMsgFromISR(log_level_t msgLevel, const char *file, uint32_t line, const char *msg) {
  if (msgLevel < logLevel) {
    return OBC_ERR_CODE_LOG_MSG_SILENCED;
  }

  if (file == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  if (msg == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  logger_event_t logEvent = {
      .logEntry = {.logType = LOG_TYPE_MSG, .logLevel = msgLevel}, .file = file, .line = line, .msg = msg};

#if defined(LOG_DATE_TIME) || defined(LOG_UNIX)
  logEvent.timestamp = GET_TIMESTAMP_FROM_ISR;
#endif

  return sendToLoggerQueueFromISR(&logEvent);
}
