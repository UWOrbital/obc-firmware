#pragma once

#include <stddef.h>
#include <stdint.h>

#include "obc_errors.h"

/**
 * @brief Setup the file system.
 *
 * @return obc_error_code_t OBC_ERR_CODE_SUCCESS if successful, otherwise an error code.
 */
obc_error_code_t setupFileSystem(void);

/**
 * @brief Create a directory.
 *
 * @param dirPath Path to the directory
 * @return obc_error_code_t OBC_ERR_CODE_SUCCESS if successful, otherwise error code
 */
obc_error_code_t mkDir(const char *dirPath);

/**
 * @brief Create a file.
 *
 * @param filePath Path to the file
 * @param fileId Buffer to store the file descriptor
 * @return obc_error_code_t OBC_ERR_CODE_SUCCESS if successful, otherwise error code
 */
obc_error_code_t createFile(const char *filePath, int32_t *fileId);

/**
 * @brief Close a file.
 *
 * @param fileId File descriptor given by Reliance Edge
 * @return obc_error_code_t OBC_ERR_CODE_SUCCESS if successful, otherwise error code
 */
obc_error_code_t closeFile(int32_t fileId);

/**
 * @brief Write to a file.
 *
 * @param fileId File descriptor given by Reliance Edge
 * @param data Buffer to write
 * @param dataLen Length of the buffer
 * @return obc_error_code_t OBC_ERR_CODE_SUCCESS if successful, otherwise error code
 */
obc_error_code_t writeFile(int32_t fileId, const void *data, size_t dataLen);

/**
 * @brief Read from a file.
 *
 * @param fileId File descriptor given by Reliance Edge
 * @param buffer Buffer to store the read data
 * @param bufferSize Size of the buffer
 * @param bytesRead Buffer to store the number of bytes read
 * @return obc_error_code_t OBC_ERR_CODE_SUCCESS if successful, otherwise error code
 */
obc_error_code_t readFile(int32_t fileId, void *buffer, size_t bufferSize, size_t *bytesRead);

/**
 * @brief Get the size of a file.
 *
 * @param fileId File descriptor given by Reliance Edge
 * @param fileSize Buffer to store the file size
 * @return obc_error_code_t OBC_ERR_CODE_SUCCESS if successful, otherwise error code
 */
obc_error_code_t getFileSize(int32_t fileId, size_t *fileSize);

/**
 * @brief Open a file.
 *
 * @param filePath Path to the file
 * @param openMode Mode to open the file in
 * @param fileId Buffer to store the file descriptor
 * @return obc_error_code_t OBC_ERR_CODE_SUCCESS if successful, otherwise error code
 */
obc_error_code_t openFile(const char *filePath, uint32_t openMode, int32_t *fileId);
