#pragma once

#include <stdint.h>
#include <stdlib.h>

#include "obc_errors.h"

/**
 * @brief Set the GNC task period. Will be used for when we want to run GNC tasks more or less frequently (QEYnet
 * pointing, attitude adjustments)
 * @param periodMs period to set the GNC periodic task time to. Note that periodMS can be set to any value between
 * 0-50ms, but can only be increased by intervals of 50ms (e.g 100ms, 150ms, 200ms, ...)
 * @return OBC_ERR_CODE_SUCCESS on success, else an error code
 */
obc_error_code_t setGncTaskPeriod(uint16_t periodMs);
