#pragma once

#include <stdint.h>
#include "obc_errors.h"
#include "obc_gs_command_data.h"
#include "fcmd_msg.h"

typedef obc_error_code_t (*cmd_callback_t)(cmd_msg_t *, uint8_t *, uint8_t *);

typedef struct {
  cmd_callback_t callback;
  cmd_policy_t policy;
  cmd_opt_t opts;  // Mask of command options
} cmd_info_t;

/**
 * @brief Verifies a command and assigns it's information to the currCmdInfo parameter.
 *
 * @param cmd Pointer to the command message
 * @param currCmdInfo Pointer to the command information
 * @return obc_error_code_t OBC_ERR_CODE_SUCCESS if successful, otherwise an error code
 */
obc_error_code_t verifyCommand(cmd_msg_t *cmd, cmd_info_t *currCmdInfo);

/**
 * @brief Processes non time tagged commands
 *
 * @param cmd Pointer to the command message
 * @param currCmdInfo Pointer to the command information
 * @return obc_error_code_t OBC_ERR_CODE_SUCCESS if successful, otherwise an error code
 */
obc_error_code_t processNonTimeTaggedCommand(cmd_msg_t *cmd, cmd_info_t *currCmdInfo, uint8_t *responseData,
                                             uint8_t *responseDataLen);

/**
 * @brief Edits the fcmd struct
 *
 * @param replacementFCmd Pointer to the command message
 * @return obc_error_code_t OBC_ERR_CODE_SUCCESS if successful, otherwise an error code
 */
obc_error_code_t editFCmdMsg(fcmd_msg_t replacementFCmd);
