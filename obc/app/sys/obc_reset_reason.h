#pragma once

/**
 * @brief Reasons for resetting the system.
 */
typedef enum {
  RESET_REASON_TESTING,               // For testing purposes
  RESET_REASON_CMD_EXEC_OBC_RESET,    // Reset due to command execution
  RESET_REASON_FS_FAILURE,            // File system operation failed
  RESET_REASON_STACK_CHECK_FAIL,      // Stack canary check failed
  RESET_REASON_FREERTOS_ASSERT_FAIL,  // config assert failed
  RESET_REASON_UNKNOWN = 0xFF,        // Unknown reason
} obc_reset_reason_t;
