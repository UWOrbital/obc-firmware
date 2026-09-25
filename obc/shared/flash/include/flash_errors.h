#pragma once

#define RELIANCE_EDGE_ERROR_CODES_OFFSET 1000U
#define DIGITAL_WATCHDOG_ERROR_CODE_OFFSET 900U

typedef enum {
  /* Common Errors 0 - 99 */
  FLASH_ERR_CODE_SUCCESS = 0,
  FLASH_ERR_CODE_UNKNOWN = 1,
  FLASH_ERR_CODE_INVALID_ARG = 2,
} flash_error_code_t;
