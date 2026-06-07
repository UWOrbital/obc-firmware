#pragma once

#include <FreeRTOS.h>
#include <os_queue.h>
#include "obc_errors.h"

extern QueueHandle_t rtcTempQueueHandle;

/**
 * @brief Get the temperature from the mailbox temperature queue of the ds3232
 * @param temp The memory address that stores the temperature in the mailbox queue
 * @return The error code.
 */
obc_error_code_t readRTCTemp(float *data);
