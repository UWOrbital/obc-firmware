#include "obc_print.h"

#include <sci.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#include "obc_assert.h"
#include "obc_board_config.h"
#include "obc_errors.h"
#include "obc_logging.h"

#define OBC_UART_BAUD_RATE 115200
#define UART_MUTEX_BLOCK_TIME portMAX_DELAY
#define MAX_PRINTF_SIZE 128U

uint32_t validBaudRates[] = {9600, 19200, 38400, 57600, 115200};

static obc_error_code_t isValidBaudRate(uint32_t baudRate);

static obc_error_code_t isValidBaudRate(uint32_t baudRate) {
  // Calculate the number of valid baud rates
  int numValidBaudRates = sizeof(validBaudRates) / sizeof(validBaudRates[0]);

  for (int i = 0; i < numValidBaudRates; i++) {
    if (baudRate == validBaudRates[i]) {
      // Baud rate is valid
      return OBC_ERR_CODE_SUCCESS;
    }
  }
  return OBC_ERR_CODE_INVALID_ARG;
}

void initSciPrint() {
  initSciMutex();
  sciPrintSetBaudrate(OBC_UART_BAUD_RATE);
}

obc_error_code_t sciPrintText(unsigned char *text, uint32_t length, TickType_t uartMutexTimeoutTicks) {
  if (text == NULL || length == 0) return OBC_ERR_CODE_INVALID_ARG;

  obc_error_code_t errCode;
  RETURN_IF_ERROR_CODE(sciSendBytes(text, length, uartMutexTimeoutTicks, UART_PRINT_REG));
  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t sciPrintf(const char *s, ...) {
  if (s == NULL) return OBC_ERR_CODE_INVALID_ARG;

  char buf[MAX_PRINTF_SIZE] = {0};

  va_list args;
  va_start(args, s);
  int n = vsnprintf(buf, MAX_PRINTF_SIZE, s, args);
  va_end(args);

  if (n < 0) return OBC_ERR_CODE_INVALID_ARG;

  // n == MAX_PRINTF_SIZE invalid because null character isn't included in count
  if ((uint32_t)n >= MAX_PRINTF_SIZE) return OBC_ERR_CODE_INVALID_ARG;

  return sciPrintText((unsigned char *)buf, (n + 1), UART_MUTEX_BLOCK_TIME);
}

obc_error_code_t sciPrintSetBaudrate(uint32_t baudRate) {
  // Check if the baudRate is a valid param
  obc_error_code_t errCode;
  RETURN_IF_ERROR_CODE(isValidBaudRate(baudRate));
  sciSetBaudrate(UART_PRINT_REG, baudRate);
  return OBC_ERR_CODE_SUCCESS;
}

void uartAssertFailed(char *file, int line, char *expr) {
  if (file == NULL || line < 0 || expr == NULL)
    return;  // Only called by assert, so we can assume that the arguments are valid

  sciPrintf("ASSERTION FAILED: %s, file %s, line %d\r\n", expr, file, line);
}
