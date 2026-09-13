#pragma once

#include "obc_errors.h"
#include <stdint.h>

/**
 * @brief Unpacks and runs a CmdMsg
 *
 * @param recvBuffer UART buffer to read command
 */
obc_error_code_t blRunCommand(uint8_t recvBuffer[]);

/**
 * @brief Checks the integrity of the app and jumps to it's reset vector
 */
obc_error_code_t blJumpToApp();
