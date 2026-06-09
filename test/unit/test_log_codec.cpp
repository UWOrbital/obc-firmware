#include "obc_logging_codec.h"
#include "obc_log_file_ids.h"
#include "obc_errors.h"

#include <gtest/gtest.h>

#include <cstring>

TEST(TestLogCodec, EncodeDecodeErrorCodeRoundTrip) {
  binary_log_entry_t entry = {};
  entry.type = LOG_TYPE_ERROR_CODE;
  entry.level = LOG_ERROR;
  entry.fileId = 12;
  entry.line = 456;
  entry.hasTimestamp = 1;
  entry.timestamp = 1735689600;  // 2025-01-01 00:00:00 UTC
  entry.errCode = OBC_ERR_CODE_MUTEX_TIMEOUT;

  uint8_t buf[BINARY_LOG_MAX_ENTRY_SIZE] = {0};
  size_t encodedLen = 0;
  ASSERT_EQ(binaryLogEncode(&entry, buf, sizeof(buf), &encodedLen), OBC_ERR_CODE_SUCCESS);
  // sync + flags + fileId + line + timestamp + errCode
  EXPECT_EQ(encodedLen, 14U);

  binary_log_entry_t decoded = {};
  size_t consumedLen = 0;
  ASSERT_EQ(binaryLogDecode(buf, encodedLen, &decoded, &consumedLen), OBC_ERR_CODE_SUCCESS);
  EXPECT_EQ(consumedLen, encodedLen);
  EXPECT_EQ(decoded.type, LOG_TYPE_ERROR_CODE);
  EXPECT_EQ(decoded.level, LOG_ERROR);
  EXPECT_EQ(decoded.fileId, entry.fileId);
  EXPECT_EQ(decoded.line, entry.line);
  EXPECT_EQ(decoded.hasTimestamp, 1U);
  EXPECT_EQ(decoded.timestamp, entry.timestamp);
  EXPECT_EQ(decoded.errCode, entry.errCode);
}

TEST(TestLogCodec, EncodeDecodeMsgRoundTrip) {
  binary_log_entry_t entry = {};
  entry.type = LOG_TYPE_MSG;
  entry.level = LOG_DEBUG;
  entry.fileId = 3;
  entry.line = 99;
  entry.hasTimestamp = 0;
  strcpy(entry.msg, "Starting init");

  uint8_t buf[BINARY_LOG_MAX_ENTRY_SIZE] = {0};
  size_t encodedLen = 0;
  ASSERT_EQ(binaryLogEncode(&entry, buf, sizeof(buf), &encodedLen), OBC_ERR_CODE_SUCCESS);
  // sync + flags + fileId + line + msgLen + msg
  EXPECT_EQ(encodedLen, 6U + 1U + strlen("Starting init"));

  binary_log_entry_t decoded = {};
  size_t consumedLen = 0;
  ASSERT_EQ(binaryLogDecode(buf, encodedLen, &decoded, &consumedLen), OBC_ERR_CODE_SUCCESS);
  EXPECT_EQ(consumedLen, encodedLen);
  EXPECT_EQ(decoded.type, LOG_TYPE_MSG);
  EXPECT_EQ(decoded.level, LOG_DEBUG);
  EXPECT_EQ(decoded.fileId, entry.fileId);
  EXPECT_EQ(decoded.line, entry.line);
  EXPECT_EQ(decoded.hasTimestamp, 0U);
  EXPECT_STREQ(decoded.msg, "Starting init");
}

// Golden vector shared with the ground-station pytest suite
// (backend/python_test/test_obc_utils/test_log_codec.py). If this test
// changes, the Python test must change too.
TEST(TestLogCodec, GoldenVectorErrorCode) {
  binary_log_entry_t entry = {};
  entry.type = LOG_TYPE_ERROR_CODE;
  entry.level = LOG_ERROR;
  entry.fileId = 7;
  entry.line = 123;
  entry.hasTimestamp = 1;
  entry.timestamp = 1735689600;  // 2025-01-01 00:00:00 UTC
  entry.errCode = 301;

  const uint8_t expected[] = {0xA8, 0x14, 0x07, 0x00, 0x7B, 0x00, 0x80, 0x85, 0x74, 0x67, 0x2D, 0x01, 0x00, 0x00};

  uint8_t buf[BINARY_LOG_MAX_ENTRY_SIZE] = {0};
  size_t encodedLen = 0;
  ASSERT_EQ(binaryLogEncode(&entry, buf, sizeof(buf), &encodedLen), OBC_ERR_CODE_SUCCESS);
  ASSERT_EQ(encodedLen, sizeof(expected));
  EXPECT_EQ(memcmp(buf, expected, sizeof(expected)), 0);
}

TEST(TestLogCodec, GoldenVectorMsg) {
  binary_log_entry_t entry = {};
  entry.type = LOG_TYPE_MSG;
  entry.level = LOG_DEBUG;
  entry.fileId = 2;
  entry.line = 10;
  entry.hasTimestamp = 0;
  strcpy(entry.msg, "Hi");

  const uint8_t expected[] = {0xA8, 0x09, 0x02, 0x00, 0x0A, 0x00, 0x02, 'H', 'i'};

  uint8_t buf[BINARY_LOG_MAX_ENTRY_SIZE] = {0};
  size_t encodedLen = 0;
  ASSERT_EQ(binaryLogEncode(&entry, buf, sizeof(buf), &encodedLen), OBC_ERR_CODE_SUCCESS);
  ASSERT_EQ(encodedLen, sizeof(expected));
  EXPECT_EQ(memcmp(buf, expected, sizeof(expected)), 0);
}

TEST(TestLogCodec, EncodeBufferTooSmall) {
  binary_log_entry_t entry = {};
  entry.type = LOG_TYPE_ERROR_CODE;
  entry.level = LOG_ERROR;
  entry.hasTimestamp = 1;

  uint8_t buf[8] = {0};  // needs 14
  size_t encodedLen = 0;
  EXPECT_EQ(binaryLogEncode(&entry, buf, sizeof(buf), &encodedLen), OBC_ERR_CODE_BUFF_TOO_SMALL);
}

TEST(TestLogCodec, EncodeInvalidArgs) {
  binary_log_entry_t entry = {};
  uint8_t buf[BINARY_LOG_MAX_ENTRY_SIZE] = {0};
  size_t encodedLen = 0;

  EXPECT_EQ(binaryLogEncode(nullptr, buf, sizeof(buf), &encodedLen), OBC_ERR_CODE_INVALID_ARG);
  EXPECT_EQ(binaryLogEncode(&entry, nullptr, sizeof(buf), &encodedLen), OBC_ERR_CODE_INVALID_ARG);
  EXPECT_EQ(binaryLogEncode(&entry, buf, sizeof(buf), nullptr), OBC_ERR_CODE_INVALID_ARG);

  entry.level = LOG_OFF;  // not a valid record level
  EXPECT_EQ(binaryLogEncode(&entry, buf, sizeof(buf), &encodedLen), OBC_ERR_CODE_INVALID_ARG);
}

TEST(TestLogCodec, DecodeRejectsBadSyncByte) {
  uint8_t buf[BINARY_LOG_MAX_ENTRY_SIZE] = {0};
  buf[0] = 0x55;

  binary_log_entry_t decoded = {};
  size_t consumedLen = 0;
  EXPECT_EQ(binaryLogDecode(buf, sizeof(buf), &decoded, &consumedLen), OBC_ERR_CODE_INVALID_ARG);
}

TEST(TestLogCodec, DecodeRejectsTruncatedBuffer) {
  binary_log_entry_t entry = {};
  entry.type = LOG_TYPE_ERROR_CODE;
  entry.level = LOG_WARN;
  entry.hasTimestamp = 1;

  uint8_t buf[BINARY_LOG_MAX_ENTRY_SIZE] = {0};
  size_t encodedLen = 0;
  ASSERT_EQ(binaryLogEncode(&entry, buf, sizeof(buf), &encodedLen), OBC_ERR_CODE_SUCCESS);

  binary_log_entry_t decoded = {};
  size_t consumedLen = 0;
  EXPECT_EQ(binaryLogDecode(buf, encodedLen - 1, &decoded, &consumedLen), OBC_ERR_CODE_FAILED_UNPACK);
  EXPECT_EQ(binaryLogDecode(buf, 3, &decoded, &consumedLen), OBC_ERR_CODE_FAILED_UNPACK);
}

TEST(TestLogCodec, FileIdLookupRoundTrip) {
  ASSERT_GT(LOG_FILE_ID_COUNT, 0U);

  for (uint16_t id = 0; id < LOG_FILE_ID_COUNT; id++) {
    const char *path = logFilePathFromId(id);
    ASSERT_NE(path, nullptr);
    EXPECT_EQ(logFileIdFromPath(path), id);
  }
}

TEST(TestLogCodec, FileIdLookupUnknown) {
  EXPECT_EQ(logFileIdFromPath("not/a/real/file.c"), BINARY_LOG_FILE_ID_UNKNOWN);
  EXPECT_EQ(logFileIdFromPath(nullptr), BINARY_LOG_FILE_ID_UNKNOWN);
  EXPECT_EQ(logFilePathFromId(BINARY_LOG_FILE_ID_UNKNOWN), nullptr);
  EXPECT_EQ(logFilePathFromId(LOG_FILE_ID_COUNT), nullptr);
}

TEST(TestLogCodec, MsgIsTruncatedToMaxLen) {
  binary_log_entry_t entry = {};
  entry.type = LOG_TYPE_MSG;
  entry.level = LOG_INFO;

  // Entry msg buffer is BINARY_LOG_MAX_MSG_LEN + 1; fill it entirely
  memset(entry.msg, 'a', BINARY_LOG_MAX_MSG_LEN);
  entry.msg[BINARY_LOG_MAX_MSG_LEN] = '\0';

  uint8_t buf[BINARY_LOG_MAX_ENTRY_SIZE] = {0};
  size_t encodedLen = 0;
  ASSERT_EQ(binaryLogEncode(&entry, buf, sizeof(buf), &encodedLen), OBC_ERR_CODE_SUCCESS);
  EXPECT_EQ(encodedLen, 6U + 1U + BINARY_LOG_MAX_MSG_LEN);

  binary_log_entry_t decoded = {};
  size_t consumedLen = 0;
  ASSERT_EQ(binaryLogDecode(buf, encodedLen, &decoded, &consumedLen), OBC_ERR_CODE_SUCCESS);
  EXPECT_EQ(strlen(decoded.msg), BINARY_LOG_MAX_MSG_LEN);
}
