#pragma once

#include "obc_errors.h"
#include "obc_gs_command_data.h"
#include "obc_gs_commands_response.h"

/**
 * @brief Sends a command to the command queue
 *
 * @param cmd Pointer to the command message
 * @return obc_error_code_t OBC_ERR_CODE_SUCCESS if successful, otherwise an error code
 */
obc_error_code_t sendToCommandQueue(cmd_msg_t *cmd);

/**
 * @brief Downlinks a command response by sending it to the downlink data queue
 *
 * @param cmdResHeader Pointer to the command response that stores the data to downlink
 * @param cmd Pointer to the command that just executed
 * @param errCode The error code of the message that just executed from the processTimeTagged/processNonTimeTagged
 * commands
 * @param responseData A pointer to an array that has data contained in the command response
 * @param responseDataLen A pointer to the length of data conatined in responseData (i.e. what number of bytes are
 * actual data rather than 0s)
 * @param sendBuffer A pointer to the array that will contained the packed command response to downlink
 * @return obc_error_code_t OBC_ERR_CODE_SUCCESS if successful, otherwise an error code
 */
obc_error_code_t downlinkCmdResponse(cmd_response_header_t *cmdResHeader, cmd_msg_t *cmd, obc_error_code_t errCode,
                                     uint8_t *responseData, uint8_t *responseDataLen, uint8_t *sendBuffer);
