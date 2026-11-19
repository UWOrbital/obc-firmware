/* Exercise the real logger task and codec, replacing only queues, clocks, and I/O. */
#include "logger.h"
#include "obc_logging_codec.h"
#include "obc_print.h"
#include "obc_time.h"
#include "obc_time_utils.h"
#include "os_queue.h"
#include "FreeRTOSConfig.h"
#include "redposix.h"
#if !LOG_USE_FILE_IDS
#include "obc_log_file_ids.h"
#endif

#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include <string.h>

void obcTaskInitLogger(void);
void obcTaskFunctionLogger(void *parameters);

static logger_event_t pending;
static int queued, task_sends, isr_sends, uart_writes, sd_writes, sd_closes;
static unsigned char output[512];
static size_t output_size;
static char output_path[32];
static jmp_buf drained;
static log_file_ref_t assertion_file;
static unsigned long assertion_line;
static const uint32_t task_time = 1735787045U; /* 2025-01-02 03:04:05 UTC */
static const uint32_t isr_time = 1735787046U;

QueueHandle_t xQueueCreateStatic(UBaseType_t count, UBaseType_t size, uint8_t *storage, StaticQueue_t *queue) {
  assert(count > 0 && size == sizeof(pending) && storage != NULL);
  return queue;
}
BaseType_t xQueueSend(QueueHandle_t queue, const void *event, TickType_t timeout) {
  (void)timeout;
  assert(queue != NULL && !queued);
  memcpy(&pending, event, sizeof(pending));
  queued = 1;
  task_sends++;
  return pdPASS;
}
BaseType_t xQueueSendFromISR(QueueHandle_t queue, const void *event, BaseType_t *awoken) {
  int previous = task_sends;
  BaseType_t result = xQueueSend(queue, event, 0);
  task_sends = previous;
  isr_sends++;
  *awoken = pdFALSE;
  return result;
}
BaseType_t xQueueReceive(QueueHandle_t queue, void *event, TickType_t timeout) {
  (void)queue;
  (void)timeout;
  if (!queued) longjmp(drained, 1);
  memcpy(event, &pending, sizeof(pending));
  queued = 0;
  return pdPASS;
}
static void capture(const void *data, size_t length) {
  assert(output_size == 0 && length < sizeof(output));
  memcpy(output, data, length);
  output_size = length;
  output[length] = '\0';
}
obc_error_code_t sciPrintText(unsigned char *data, uint32_t length, TickType_t timeout) {
  assert(timeout == portMAX_DELAY);
  capture(data, length);
  uart_writes++;
  return OBC_ERR_CODE_SUCCESS;
}
int32_t red_open(const char *path, uint32_t flags) {
  assert(flags == (RED_O_WRONLY | RED_O_APPEND | RED_O_CREAT));
  snprintf(output_path, sizeof(output_path), "%s", path);
  return 42;
}
int32_t red_write(int32_t descriptor, const void *data, uint32_t length) {
  assert(descriptor == 42);
  capture(data, length);
  sd_writes++;
  return (int32_t)length;
}
int32_t red_close(int32_t descriptor) {
  assert(descriptor == 42);
  sd_closes++;
  return 0;
}
uint32_t getCurrentUnixTime(void) { return task_time; }
uint32_t getCurrentUnixTimeInISR(void) { return isr_time; }
rtc_date_time_t getCurrentDateTime(void) {
  rtc_date_time_t result;
  assert(unixToDatetime(task_time, &result) == OBC_ERR_CODE_SUCCESS);
  return result;
}
rtc_date_time_t getCurrentDateTimeinISR(void) {
  rtc_date_time_t result;
  assert(unixToDatetime(isr_time, &result) == OBC_ERR_CODE_SUCCESS);
  return result;
}
void freertosConfigAssert(log_file_ref_t file, unsigned long line) {
  assertion_file = file;
  assertion_line = line;
}

static void reset_output(void) {
  assert(!queued);
  memset(output, 0, sizeof(output));
  output_size = 0;
  output_path[0] = '\0';
  task_sends = isr_sends = uart_writes = sd_writes = sd_closes = 0;
}
static void drain(void) {
  if (setjmp(drained) == 0) obcTaskFunctionLogger(NULL);
  assert(!queued);
}
static void check_file(log_file_ref_t actual, log_file_ref_t expected) {
#if LOG_USE_FILE_IDS
  assert(actual == expected);
#else
  assert(actual != NULL && strcmp(actual, expected) == 0);
#endif
}

static void check_record(log_file_ref_t file, uint16_t id, int from_isr, int message, log_output_format_t format,
                         log_output_location_t location) {
  reset_output();
  logSetOutputFormat(format);
  logSetOutputLocation(location);
  /* ISR's existing return-code behavior is unrelated; verify its actual queued event. */
  if (message) {
    if (from_isr)
      (void)logMsgFromISR(LOG_INFO, file, 321, "hello satellite");
    else
      assert(logMsg(LOG_INFO, file, 321, "hello satellite") == OBC_ERR_CODE_SUCCESS);
  } else {
    if (from_isr)
      (void)logErrorCodeFromISR(LOG_ERROR, file, 321, 0x12345678U);
    else
      assert(logErrorCode(LOG_ERROR, file, 321, 0x12345678U) == OBC_ERR_CODE_SUCCESS);
  }
  assert(queued && task_sends == !from_isr && isr_sends == from_isr);
  check_file(pending.file, file);
  drain();
  assert(output_size > 0);
  assert(uart_writes == (location == LOG_TO_UART));
  assert(sd_writes == (location == LOG_TO_SDCARD) && sd_closes == sd_writes);
  if (location == LOG_TO_SDCARD) assert(strcmp(output_path, format == LOG_FORMAT_BINARY ? "log.bin" : "log.log") == 0);
  if (format == LOG_FORMAT_BINARY) {
    binary_log_entry_t decoded;
    size_t consumed = 0;
    assert(output[2] == (id & 0xff) && output[3] == (id >> 8));
    assert(binaryLogDecode(output, output_size, &decoded, &consumed) == OBC_ERR_CODE_SUCCESS);
    assert(consumed == output_size && decoded.fileId == id && decoded.line == 321);
    assert(decoded.type == (message ? LOG_TYPE_MSG : LOG_TYPE_ERROR_CODE));
    if (message)
      assert(strcmp(decoded.msg, "hello satellite") == 0);
    else
      assert(decoded.errCode == 0x12345678U);
#if defined(LOG_DATE_TIME) || defined(LOG_UNIX)
    assert(decoded.hasTimestamp && decoded.timestamp == (from_isr ? isr_time : task_time));
#else
    assert(!decoded.hasTimestamp);
#endif
  } else {
    char expected[256];
#if LOG_USE_FILE_IDS
    snprintf(expected, sizeof(expected), "-> %u:321", (unsigned)id);
#else
    snprintf(expected, sizeof(expected), "-> %s:321", file);
#endif
    assert(strstr((char *)output, expected) != NULL);
    assert(strstr((char *)output, message ? " - hello satellite\r\n" : " - 305419896\r\n") != NULL);
#if defined(LOG_DATE_TIME)
    assert(strncmp((char *)output, from_isr ? "25-01-02_03-04-06" : "25-01-02_03-04-05", 17) == 0);
#elif defined(LOG_UNIX)
    assert(strncmp((char *)output, from_isr ? "1735787046 " : "1735787045 ", 11) == 0);
#else
    assert(strncmp((char *)output, message ? "INFO " : "ERROR", 5) == 0);
#endif
  }
}

int main(void) {
  obcTaskInitLogger();
  logSetLevel(LOG_TRACE);
#if LOG_USE_FILE_IDS
  const log_file_ref_t files[] = {0, 7, UINT16_MAX};
  const uint16_t ids[] = {0, 7, UINT16_MAX};
#else
  assert(LOG_FILE_ID_COUNT > 7);
  const log_file_ref_t files[] = {LOG_FILE_PATHS[0], LOG_FILE_PATHS[7], "unknown/source.c"};
  const uint16_t ids[] = {0, 7, UINT16_MAX};
#endif
  for (size_t file = 0; file < 3; file++)
    for (int isr = 0; isr < 2; isr++)
      for (int message = 0; message < 2; message++)
        for (int binary = 0; binary < 2; binary++)
          for (int sd = 0; sd < 2; sd++)
            check_record(files[file], ids[file], isr, message, binary ? LOG_FORMAT_BINARY : LOG_FORMAT_TEXT,
                         sd ? LOG_TO_SDCARD : LOG_TO_UART);

  reset_output();
  assert(logMsg(LOG_INFO, files[0], 1, NULL) == OBC_ERR_CODE_INVALID_ARG);
  assert(logMsgFromISR(LOG_INFO, files[0], 1, NULL) == OBC_ERR_CODE_INVALID_ARG);
#if !LOG_USE_FILE_IDS
  assert(logMsg(LOG_INFO, NULL, 1, "hello") == OBC_ERR_CODE_INVALID_ARG);
  assert(logMsgFromISR(LOG_INFO, NULL, 1, "hello") == OBC_ERR_CODE_INVALID_ARG);
  assert(logErrorCode(LOG_ERROR, NULL, 1, 1) == OBC_ERR_CODE_INVALID_ARG);
  assert(logErrorCodeFromISR(LOG_ERROR, NULL, 1, 1) == OBC_ERR_CODE_INVALID_ARG);
#endif
  assert(!queued);
  unsigned macro_line = __LINE__ + 1;
  assert(LOG_INFO("macro message") == OBC_ERR_CODE_SUCCESS);
  assert(pending.line == macro_line);
#if LOG_USE_FILE_IDS
  assert(pending.file == 7);
#else
  assert(strcmp(pending.file, "test/logging/test_logger.c") == 0);
#endif
  drain();
  unsigned assert_line = __LINE__ + 1;
  configASSERT(pdFALSE);
  assert(assertion_line == assert_line);
#if LOG_USE_FILE_IDS
  assert(assertion_file == 7);
#else
  assert(strcmp(assertion_file, "test/logging/test_logger.c") == 0);
#endif
  puts("Logger task/ISR, binary/text, UART/SD, source references and assertions passed.");
  return 0;
}
