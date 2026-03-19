
#pragma once

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "obc_gs_errors.h"
#include "obc_gs_commands_response.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Unpack a buffer of chars to a struct of type cmd_unpacked_response_t representing the response of a command.
 * @param buffer A buffer of bytes from which to read the packed data
 * @param unpackedResponsePtr Pointer to the response which needs to be set from values obtained by unpacking the
 * buffer.
 * @return Error code indicating if the write was successful. OBC_GS_ERR_CODE_UNSUPPORTED_CMD if the unpacked cmd was
 * not valid.
 */
obc_gs_error_code_t unpackCmdResponse(uint8_t* buffer, cmd_response_header_t* response, uint8_t* responseDataBuffer);

#ifdef __cplusplus
}
#endif
