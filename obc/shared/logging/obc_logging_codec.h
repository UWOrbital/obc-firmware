#pragma once

#include "obc_errors.h"
#include "obc_logging.h"

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Compact binary log record format (all multi-byte fields little-endian):
 *
 * | Offset | Size | Field                                               |
 * |--------|------|-----------------------------------------------------|
 * | 0      | 1    | Sync byte (BINARY_LOG_SYNC_BYTE)                    |
 * | 1      | 1    | Flags: bits 0-2 log level, bit 3 log type           |
 * |        |      | (0 = error code, 1 = msg), bit 4 has-timestamp      |
 * | 2      | 2    | File ID (see obc_log_file_ids.h / log_file_ids.json)|
 * | 4      | 2    | Line number                                         |
 * | 6      | 4    | Unix timestamp (only present if has-timestamp set)  |
 * | ...    | 4    | Error code (only if log type is error code)         |
 * | ...    | 1+N  | Msg length N + N msg bytes (only if log type is msg)|
 *
 * A typical error-code record is 14 bytes vs ~60+ bytes of formatted text.
 * The ground station decodes records back into the standard text log format
 * using the same file ID mapping (log_file_ids.json).
 */

#define BINARY_LOG_SYNC_BYTE 0xA8U

#define BINARY_LOG_FLAG_LEVEL_MASK 0x07U
#define BINARY_LOG_FLAG_TYPE_MSG 0x08U
#define BINARY_LOG_FLAG_HAS_TIMESTAMP 0x10U

#define BINARY_LOG_MAX_MSG_LEN 128U
#define BINARY_LOG_FIXED_HEADER_SIZE 6U
#define BINARY_LOG_TIMESTAMP_SIZE 4U
#define BINARY_LOG_ERROR_CODE_SIZE 4U
#define BINARY_LOG_MAX_ENTRY_SIZE \
  (BINARY_LOG_FIXED_HEADER_SIZE + BINARY_LOG_TIMESTAMP_SIZE + 1U + BINARY_LOG_MAX_MSG_LEN)

// File ID used when the file path is not in the generated file ID table
#define BINARY_LOG_FILE_ID_UNKNOWN 0xFFFFU

/**
 * @struct binary_log_entry_t
 * @brief In-memory representation of a single binary log record.
 */
typedef struct {
  log_type_t type;
  log_level_t level;
  uint16_t fileId;
  uint16_t line;
  uint8_t hasTimestamp;
  uint32_t timestamp;                    // Unix seconds, valid if hasTimestamp is non-zero
  uint32_t errCode;                      // Valid if type == LOG_TYPE_ERROR_CODE
  char msg[BINARY_LOG_MAX_MSG_LEN + 1];  // Valid if type == LOG_TYPE_MSG (null-terminated)
} binary_log_entry_t;

/**
 * @brief Encode a log entry into the compact binary format.
 *
 * @param entry       Entry to encode
 * @param buf         Output buffer
 * @param bufLen      Size of the output buffer
 * @param encodedLen  Set to the number of bytes written on success
 * @return obc_error_code_t OBC_ERR_CODE_INVALID_ARG     if any pointer is NULL or the entry is malformed
 *                          OBC_ERR_CODE_BUFF_TOO_SMALL  if buf cannot hold the encoded record
 *                          OBC_ERR_CODE_SUCCESS         otherwise
 */
obc_error_code_t binaryLogEncode(const binary_log_entry_t *entry, uint8_t *buf, size_t bufLen, size_t *encodedLen);

/**
 * @brief Decode a compact binary log record back into a log entry.
 *
 * @param buf          Buffer starting at a record's sync byte
 * @param bufLen       Number of valid bytes in buf
 * @param entry        Decoded entry output
 * @param consumedLen  Set to the number of bytes consumed by the record on success
 * @return obc_error_code_t OBC_ERR_CODE_INVALID_ARG    if any pointer is NULL or the sync byte is wrong
 *                          OBC_ERR_CODE_FAILED_UNPACK  if the buffer is truncated or the record is malformed
 *                          OBC_ERR_CODE_SUCCESS        otherwise
 */
obc_error_code_t binaryLogDecode(const uint8_t *buf, size_t bufLen, binary_log_entry_t *entry, size_t *consumedLen);

/**
 * @brief Look up the file ID for a repo-relative file path (as produced by __FILE_FROM_REPO_ROOT__).
 *
 * @param path Repo-relative file path
 * @return uint16_t The file ID, or BINARY_LOG_FILE_ID_UNKNOWN if the path is not in the table
 */
uint16_t logFileIdFromPath(const char *path);

/**
 * @brief Look up the repo-relative file path for a file ID.
 *
 * @param fileId The file ID
 * @return const char* The file path, or NULL if the ID is not in the table
 */
const char *logFilePathFromId(uint16_t fileId);

#ifdef __cplusplus
}
#endif
