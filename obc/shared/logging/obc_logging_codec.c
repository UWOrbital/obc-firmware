#include "obc_logging_codec.h"
#include "obc_log_file_ids.h"

#include <string.h>

// Wire format layout is documented in obc_logging_codec.h. This file implements
// pack/unpack of individual records and file-path ID lookup against the
// generated table in obc_log_file_ids.c.

/**
 * @brief Return the length of a null-terminated string, capped at maxLen.
 *
 * Used when encoding message logs so we never read past the msg buffer or
 * emit more than BINARY_LOG_MAX_MSG_LEN bytes on the wire.
 */
static size_t boundedStrLen(const char *str, size_t maxLen) {
  size_t len = 0;
  while (len < maxLen && str[len] != '\0') {
    len++;
  }
  return len;
}

// Little-endian pack/unpack helpers. The ground-station Python decoder uses the
// same byte order, so these must not be changed without updating both repos.

static void packUint16LE(uint8_t *buf, uint16_t val) {
  buf[0] = (uint8_t)(val & 0xFFU);
  buf[1] = (uint8_t)((val >> 8) & 0xFFU);
}

static void packUint32LE(uint8_t *buf, uint32_t val) {
  buf[0] = (uint8_t)(val & 0xFFU);
  buf[1] = (uint8_t)((val >> 8) & 0xFFU);
  buf[2] = (uint8_t)((val >> 16) & 0xFFU);
  buf[3] = (uint8_t)((val >> 24) & 0xFFU);
}

static uint16_t unpackUint16LE(const uint8_t *buf) { return (uint16_t)(buf[0] | ((uint16_t)buf[1] << 8)); }

static uint32_t unpackUint32LE(const uint8_t *buf) {
  return (uint32_t)buf[0] | ((uint32_t)buf[1] << 8) | ((uint32_t)buf[2] << 16) | ((uint32_t)buf[3] << 24);
}

obc_error_code_t binaryLogEncode(const binary_log_entry_t *entry, uint8_t *buf, size_t bufLen, size_t *encodedLen) {
  if (entry == NULL || buf == NULL || encodedLen == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  // LOG_OFF is a filter level, not a valid record level
  if (entry->level > LOG_FATAL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  size_t msgLen = 0;
  if (entry->type == LOG_TYPE_MSG) {
    msgLen = boundedStrLen(entry->msg, BINARY_LOG_MAX_MSG_LEN);
  }

  // Record size depends on which optional/trailing fields are present
  size_t totalLen = BINARY_LOG_FIXED_HEADER_SIZE;
  if (entry->hasTimestamp) {
    totalLen += BINARY_LOG_TIMESTAMP_SIZE;
  }
  if (entry->type == LOG_TYPE_ERROR_CODE) {
    totalLen += BINARY_LOG_ERROR_CODE_SIZE;
  } else {
    totalLen += 1U + msgLen;  // 1-byte length prefix + message body
  }

  if (bufLen < totalLen) {
    return OBC_ERR_CODE_BUFF_TOO_SMALL;
  }

  size_t offset = 0;

  // Fixed header: sync byte + flags + file ID + line number
  buf[offset++] = BINARY_LOG_SYNC_BYTE;

  uint8_t flags = (uint8_t)(entry->level & BINARY_LOG_FLAG_LEVEL_MASK);
  if (entry->type == LOG_TYPE_MSG) {
    flags |= BINARY_LOG_FLAG_TYPE_MSG;
  }
  if (entry->hasTimestamp) {
    flags |= BINARY_LOG_FLAG_HAS_TIMESTAMP;
  }
  buf[offset++] = flags;

  packUint16LE(&buf[offset], entry->fileId);
  offset += 2;
  packUint16LE(&buf[offset], entry->line);
  offset += 2;

  // Optional timestamp (unix seconds, same representation as LOG_UNIX text logs)
  if (entry->hasTimestamp) {
    packUint32LE(&buf[offset], entry->timestamp);
    offset += 4;
  }

  // Trailing payload: either a raw error code or a length-prefixed message
  if (entry->type == LOG_TYPE_ERROR_CODE) {
    packUint32LE(&buf[offset], entry->errCode);
    offset += 4;
  } else {
    buf[offset++] = (uint8_t)msgLen;
    memcpy(&buf[offset], entry->msg, msgLen);
    offset += msgLen;
  }

  *encodedLen = offset;
  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t binaryLogDecode(const uint8_t *buf, size_t bufLen, binary_log_entry_t *entry, size_t *consumedLen) {
  if (buf == NULL || entry == NULL || consumedLen == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  if (bufLen < BINARY_LOG_FIXED_HEADER_SIZE) {
    return OBC_ERR_CODE_FAILED_UNPACK;
  }

  // Caller must position buf at a sync byte; stream decoders scan for 0xA8 first
  if (buf[0] != BINARY_LOG_SYNC_BYTE) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  memset(entry, 0, sizeof(*entry));

  // Parse fixed header
  const uint8_t flags = buf[1];
  const uint8_t level = flags & BINARY_LOG_FLAG_LEVEL_MASK;
  if (level > LOG_FATAL) {
    return OBC_ERR_CODE_FAILED_UNPACK;
  }

  entry->level = (log_level_t)level;
  entry->type = (flags & BINARY_LOG_FLAG_TYPE_MSG) ? LOG_TYPE_MSG : LOG_TYPE_ERROR_CODE;
  entry->hasTimestamp = (flags & BINARY_LOG_FLAG_HAS_TIMESTAMP) ? 1U : 0U;
  entry->fileId = unpackUint16LE(&buf[2]);
  entry->line = unpackUint16LE(&buf[4]);

  size_t offset = BINARY_LOG_FIXED_HEADER_SIZE;

  if (entry->hasTimestamp) {
    if (bufLen < offset + BINARY_LOG_TIMESTAMP_SIZE) {
      return OBC_ERR_CODE_FAILED_UNPACK;
    }
    entry->timestamp = unpackUint32LE(&buf[offset]);
    offset += BINARY_LOG_TIMESTAMP_SIZE;
  }

  if (entry->type == LOG_TYPE_ERROR_CODE) {
    if (bufLen < offset + BINARY_LOG_ERROR_CODE_SIZE) {
      return OBC_ERR_CODE_FAILED_UNPACK;
    }
    entry->errCode = unpackUint32LE(&buf[offset]);
    offset += BINARY_LOG_ERROR_CODE_SIZE;
  } else {
    if (bufLen < offset + 1U) {
      return OBC_ERR_CODE_FAILED_UNPACK;
    }
    const uint8_t msgLen = buf[offset++];
    // Reject corrupt length fields before copying into the fixed-size msg buffer
    if (msgLen > BINARY_LOG_MAX_MSG_LEN || bufLen < offset + msgLen) {
      return OBC_ERR_CODE_FAILED_UNPACK;
    }
    memcpy(entry->msg, &buf[offset], msgLen);
    entry->msg[msgLen] = '\0';
    offset += msgLen;
  }

  *consumedLen = offset;
  return OBC_ERR_CODE_SUCCESS;
}

uint16_t logFileIdFromPath(const char *path) {
  if (path == NULL) {
    return BINARY_LOG_FILE_ID_UNKNOWN;
  }

  // LOG_FILE_PATHS is sorted alphabetically by gen_log_file_ids.py, so binary
  // search is valid. Paths come from __FILE_FROM_REPO_ROOT__ at compile time.
  size_t low = 0;
  size_t high = LOG_FILE_ID_COUNT;
  while (low < high) {
    const size_t mid = low + (high - low) / 2;
    const int cmp = strcmp(LOG_FILE_PATHS[mid], path);
    if (cmp == 0) {
      return (uint16_t)mid;
    } else if (cmp < 0) {
      low = mid + 1;
    } else {
      high = mid;
    }
  }

  // e.g. FreeRTOS assert paths that bypass __FILE_FROM_REPO_ROOT__
  return BINARY_LOG_FILE_ID_UNKNOWN;
}

const char *logFilePathFromId(uint16_t fileId) {
  if (fileId >= LOG_FILE_ID_COUNT) {
    return NULL;
  }
  return LOG_FILE_PATHS[fileId];
}
