#include "obc_reliance_fs.h"

#include <redposix.h>

#include "obc_errors.h"
#include "obc_logging.h"

obc_error_code_t setupFileSystem(void) {
  int32_t ret;

  ret = red_init();
  if (ret != 0) {
    return OBC_ERR_CODE_FS_INIT_FAILED;
  }

  // TODO: FS formatting doesn't need to be done every time
  // since it wipes the microSD card. We should only do it
  // if the card is new or if the user requests it.
  ret = red_format("");
  if (ret != 0) {
    return OBC_ERR_CODE_FS_FORMAT_FAILED;
  }

  ret = red_mount("");
  if (ret != 0) {
    return OBC_ERR_CODE_FS_MOUNT_FAILED;
  }

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t mkDir(const char *dirPath) {
  int32_t ret = red_mkdir(dirPath);
  if (ret != 0) {
    if (red_errno == RED_EEXIST) {
      return OBC_ERR_CODE_SUCCESS;
    }

    LOG_ERROR_CODE(red_errno + RELIANCE_EDGE_ERROR_CODES_OFFSET);
    return OBC_ERR_CODE_MKDIR_FAILED;
  }

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t createFile(const char *filePath, int32_t *fileId) {
  obc_error_code_t errCode;

  if (fileId == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  // One of RED_O_RDONLY, RED_O_WRONLY, or RED_O_RDWR must be specified.
  // We're closing the file immediately anyways, so doesn't matter which one we use.
  int32_t file = red_open(filePath, RED_O_RDONLY | RED_O_CREAT);
  if (file < 0) {
    return OBC_ERR_CODE_FAILED_FILE_OPEN;
  }

  *fileId = file;

  RETURN_IF_ERROR_CODE(closeFile(file));

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t closeFile(int32_t fileId) {
  if (fileId < 0) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  int32_t ret = red_close(fileId);
  if (ret != 0) {
    LOG_ERROR_CODE(red_errno + RELIANCE_EDGE_ERROR_CODES_OFFSET);
    return OBC_ERR_CODE_FAILED_FILE_CLOSE;
  }

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t deleteFile(const char *filePath) {
  int32_t ret = red_unlink(filePath);
  if (ret != 0) {
    LOG_ERROR_CODE(red_errno + RELIANCE_EDGE_ERROR_CODES_OFFSET);
    return OBC_ERR_CODE_FAILED_FILE_DELETE;
  }

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t writeFile(int32_t fileId, const void *data, size_t dataLength) {
  if (fileId < 0) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  int32_t ret = red_write(fileId, data, dataLength);

  if (red_errno == RED_EFBIG) {
    return OBC_ERR_CODE_MAX_FILE_SIZE_REACHED;
  }

  if (ret < 0) {
    LOG_ERROR_CODE(red_errno + RELIANCE_EDGE_ERROR_CODES_OFFSET);
    return OBC_ERR_CODE_FAILED_FILE_WRITE;
  }

  if ((size_t)ret != dataLength) {
    LOG_ERROR_CODE(red_errno + RELIANCE_EDGE_ERROR_CODES_OFFSET);
    return OBC_ERR_CODE_FAILED_FILE_WRITE;
  }

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t readFile(int32_t fileId, void *buffer, size_t bufferSize, size_t *bytesRead) {
  if (fileId < 0) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  if (bytesRead == NULL || buffer == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  int32_t ret = red_read(fileId, buffer, bufferSize);
  if (ret < 0) {
    LOG_ERROR_CODE(red_errno + RELIANCE_EDGE_ERROR_CODES_OFFSET);
    return OBC_ERR_CODE_FAILED_FILE_READ;
  }

  *bytesRead = ret;

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t getFileSize(int32_t fileId, size_t *fileSize) {
  int32_t curr_pos = red_lseek(fileId, 0, RED_SEEK_CUR);

  if (curr_pos < 0) {
    LOG_ERROR_CODE(red_errno + RELIANCE_EDGE_ERROR_CODES_OFFSET);
    return OBC_ERR_CODE_FAILED_FILE_SEEK;
  }

  int32_t file_end = red_lseek(fileId, 0, RED_SEEK_END);

  if (file_end < 0) {
    LOG_ERROR_CODE(red_errno + RELIANCE_EDGE_ERROR_CODES_OFFSET);
    return OBC_ERR_CODE_FAILED_FILE_SEEK;
  }

  *fileSize = file_end != 1 ? file_end : 0;

  // Seek back to the original position
  int32_t ret = red_lseek(fileId, curr_pos, RED_SEEK_SET);

  if (ret < 0) {
    LOG_ERROR_CODE(red_errno + RELIANCE_EDGE_ERROR_CODES_OFFSET);
    return OBC_ERR_CODE_FAILED_FILE_SEEK;
  }

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t openFile(const char *filePath, uint32_t openMode, int32_t *fileId) {
  *fileId = red_open(filePath, openMode);
  if (*fileId < 0) {
    return OBC_ERR_CODE_FAILED_FILE_OPEN;
  }

  return OBC_ERR_CODE_SUCCESS;
}
