#include "obc_logging_codec.h"
#include "obc_log_file_ids.h"

#include <string.h>

// Wire format layout is documented in obc_logging_codec.h. This file implements
// pack/unpack of individual records and file-path ID lookup against the
// generated table in obc_log_file_ids.c.

// Little-endian pack/unpack helpers. The ground-station Python decoder uses the
// same byte order, so these must not be changed without updating both repos.

static obc_error_code_t packUint16LE(uint8_t *buf, uint16_t val) {
  if (buf == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  buf[0] = (uint8_t)(val & 0xFFU);
  buf[1] = (uint8_t)((val >> 8) & 0xFFU);
  return OBC_ERR_CODE_SUCCESS;
}

static obc_error_code_t packUint32LE(uint8_t *buf, uint32_t val) {
  if (buf == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  buf[0] = (uint8_t)(val & 0xFFU);
  buf[1] = (uint8_t)((val >> 8) & 0xFFU);
  buf[2] = (uint8_t)((val >> 16) & 0xFFU);
  buf[3] = (uint8_t)((val >> 24) & 0xFFU);
  return OBC_ERR_CODE_SUCCESS;
}

static obc_error_code_t unpackUint16LE(const uint8_t *buf, uint16_t *val) {
  if (buf == NULL || val == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  *val = (uint16_t)(buf[0] | ((uint16_t)buf[1] << 8));
  return OBC_ERR_CODE_SUCCESS;
}

static obc_error_code_t unpackUint32LE(const uint8_t *buf, uint32_t *val) {
  if (buf == NULL || val == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  *val = (uint32_t)buf[0] | ((uint32_t)buf[1] << 8) | ((uint32_t)buf[2] << 16) | ((uint32_t)buf[3] << 24);
  return OBC_ERR_CODE_SUCCESS;
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
    while (msgLen < BINARY_LOG_MAX_MSG_LEN && entry->msg[msgLen] != '\0') {
      msgLen++;
    }
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

  obc_error_code_t errCode = packUint16LE(&buf[offset], entry->fileId);
  if (errCode != OBC_ERR_CODE_SUCCESS) {
    return errCode;
  }
  offset += 2;
  errCode = packUint16LE(&buf[offset], entry->line);
  if (errCode != OBC_ERR_CODE_SUCCESS) {
    return errCode;
  }
  offset += 2;

  // Optional timestamp (unix seconds, same representation as LOG_UNIX text logs)
  if (entry->hasTimestamp) {
    errCode = packUint32LE(&buf[offset], entry->timestamp);
    if (errCode != OBC_ERR_CODE_SUCCESS) {
      return errCode;
    }
    offset += 4;
  }

  // Trailing payload: either a raw error code or a length-prefixed message
  if (entry->type == LOG_TYPE_ERROR_CODE) {
    errCode = packUint32LE(&buf[offset], entry->errCode);
    if (errCode != OBC_ERR_CODE_SUCCESS) {
      return errCode;
    }
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
  obc_error_code_t errCode = unpackUint16LE(&buf[2], &entry->fileId);
  if (errCode != OBC_ERR_CODE_SUCCESS) {
    return errCode;
  }
  errCode = unpackUint16LE(&buf[4], &entry->line);
  if (errCode != OBC_ERR_CODE_SUCCESS) {
    return errCode;
  }

  size_t offset = BINARY_LOG_FIXED_HEADER_SIZE;

  if (entry->hasTimestamp) {
    if (bufLen < offset + BINARY_LOG_TIMESTAMP_SIZE) {
      return OBC_ERR_CODE_FAILED_UNPACK;
    }
    errCode = unpackUint32LE(&buf[offset], &entry->timestamp);
    if (errCode != OBC_ERR_CODE_SUCCESS) {
      return errCode;
    }
    offset += BINARY_LOG_TIMESTAMP_SIZE;
  }

  if (entry->type == LOG_TYPE_ERROR_CODE) {
    if (bufLen < offset + BINARY_LOG_ERROR_CODE_SIZE) {
      return OBC_ERR_CODE_FAILED_UNPACK;
    }
    errCode = unpackUint32LE(&buf[offset], &entry->errCode);
    if (errCode != OBC_ERR_CODE_SUCCESS) {
      return errCode;
    }
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

obc_error_code_t logFileIdFromPath(const char *path, uint16_t *fileId) {
  if (path == NULL || fileId == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  // LOG_FILE_PATHS is sorted alphabetically by gen_log_file_ids.py, so binary
  // search is valid. Paths come from __FILE_FROM_REPO_ROOT__ at compile time.
  size_t low = 0;
  size_t high = LOG_FILE_ID_COUNT;
  while (low < high) {
    const size_t mid = low + (high - low) / 2;
    const int cmp = strcmp(LOG_FILE_PATHS[mid], path);
    if (cmp == 0) {
      *fileId = (uint16_t)mid;
      return OBC_ERR_CODE_SUCCESS;
    } else if (cmp < 0) {
      low = mid + 1;
    } else {
      high = mid;
    }
  }

  // e.g. FreeRTOS assert paths that bypass __FILE_FROM_REPO_ROOT__
  *fileId = BINARY_LOG_FILE_ID_UNKNOWN;
  return OBC_ERR_CODE_SUCCESS;
}

const char *logFilePathFromId(uint16_t fileId) {
  if (fileId >= LOG_FILE_ID_COUNT) {
    return NULL;
  }
  return LOG_FILE_PATHS[fileId];
}
