#pragma once

#include "obc_errors.h"
#include "obc_gs_command_data.h"
#include "bl_app_flag.h"
#include <stdint.h>

/**
 * @brief Unpacks a CmdMsg
 *
 * @param recvBuffer UART buffer to read command
 * @param unpackedCmdMsg Pointer to cmd_msg_t variable to store unpacked cmd msg
 */
obc_error_code_t blUnpackCommand(uint8_t recvBuffer[], cmd_msg_t* unpackedCmdMsg);

/**
 * @brief Runs a CmdMsg
 *
 * @param unpackedCmdMsg CmdMsg to run
 */
obc_error_code_t blRunCommand(cmd_msg_t* unpackedCmdMsg);

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

/**
 * @brief Reads the bytes at flash memory and returns the forwarded struct, afterwards clearing it.
 */
obc_error_code_t blReadFCmd();
