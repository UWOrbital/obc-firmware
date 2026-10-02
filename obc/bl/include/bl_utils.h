#pragma once

#include "obc_errors.h"
#include "bl_app_flag.h"
#include <stdint.h>

/**
 * @brief Unpacks and runs a CmdMsg
 *
 * @param recvBuffer UART buffer to read command
 */
obc_error_code_t blRunCommand(uint8_t recvBuffer[]);

/**
 * @brief Edits App flag struct in flash
 *
 * @param replacement_app_flag App flag struct to be written to flash
 */
obc_error_code_t blEditAppFlag(app_flag_t replacement_app_flag);

/**
 * @brief Checks the integrity of the app and jumps to it's reset vector
 */
obc_error_code_t blJumpToApp();
