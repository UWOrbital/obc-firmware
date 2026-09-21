#pragma once
#include "obc_reset_reason.h"  // Enum of reasons for resetting the system

/**
 * @brief reset the systems
 *
 * @param obc_reset_reason_t - the reason to reset system
 */
__attribute__((noreturn)) void resetSystem(obc_reset_reason_t reason);
