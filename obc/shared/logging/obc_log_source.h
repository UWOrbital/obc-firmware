#pragma once

#include <stdint.h>

#ifndef LOG_USE_FILE_IDS
#define LOG_USE_FILE_IDS 0
#endif

// This header is also used by FreeRTOSConfig.h, without pulling in the logger.
#if LOG_USE_FILE_IDS
typedef uint16_t log_file_ref_t;

// CMake supplies a different constant for each source file from the generated
// mapping. An unmapped logging caller fails to compile instead of losing its ID.
#define OBC_LOG_FILE ((log_file_ref_t)OBC_LOG_FILE_ID)
#else
#include <string.h>

typedef const char *log_file_ref_t;

#ifdef SOURCE_PATH
#define OBC_LOG_FILE \
  (strstr(__FILE__, SOURCE_PATH) ? strstr(__FILE__, SOURCE_PATH) + sizeof(SOURCE_PATH) - 1 : __FILE__)
#else
#define OBC_LOG_FILE __FILE__
#endif
#endif
