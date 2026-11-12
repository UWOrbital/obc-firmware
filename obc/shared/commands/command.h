#pragma once

#include <stdint.h>
#include "obc_errors.h"
#include "obc_gs_command_data.h"

#define CMD_FORWARDING_START_ADDRESS 0xF0204000
#define CMD_FORWARDING_SECTION_SIZE 0x00004000

#define FCMD_FLAG_START_ADDRESS 0xF0204000
#define FCMD_FLAG_SECTION_SIZE 0x00004000

typedef obc_error_code_t (*cmd_callback_t)(cmd_msg_t *, uint8_t *, uint8_t *);

typedef struct {
  cmd_callback_t callback;
  cmd_policy_t policy;
  cmd_opt_t opts;  // Mask of command options
} cmd_info_t;

#define ENABLE_APP_MAGIC_NUM 0x4F4F4F4F

typedef struct __attribute__((packed)) {
  fcmd_msg_t fCmd;
  uint32_t flag;
} fcmd_flag_t;

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
obc_error_code_t editFCmdFlag(fcmd_flag_t replacementFCmd);

/**
 * @brief Casts cmd to fcmd type. Does not check if cmd data is of type fcmd. Erases data union of the fcmd buffer.
 *
 * @param cmd_msg_t* cmd_msg Pointer to the cmd message
 * @param fcmd_msg_t* fCmdBuffer Pointer to the buffer storing the fCmd
 * @return obc_error_code_t OBC_ERR_CODE_SUCCESS if successful, otherwise an error code
 */
obc_error_code_t castCmdToFCmd(const cmd_msg_t *cmd_msg, fcmd_msg_t *fCmdBuffer);

/**
 * @brief Casts fcmd to cmd type. Erases data union of the cmd msg buffer.
 *
 * @param fcmd_msg_t* fcmd_msg Pointer to the cmd message
 * @param cmd_msg_t* cmdBuffer Pointer to the buffer storing the fCmd
 * @return obc_error_code_t OBC_ERR_CODE_SUCCESS if successful, otherwise an error code
 */
obc_error_code_t castFCmdToCmd(const fcmd_msg_t *fcmd_msg, cmd_msg_t *cmdBuffer);
