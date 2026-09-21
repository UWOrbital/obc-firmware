#pragma once

#include "obc_errors.h"
#include <stdint.h>

extern uint8_t appBootBFlag;
extern uint8_t appWriteBFlag;

/**
 * @brief Unpacks and runs a CmdMsg
 *
 * @param recvBuffer UART buffer to read command
 */
obc_error_code_t blRunCommand(uint8_t recvBuffer[]);

/**
 * @brief Chooses which app to boot into
 *
 * @param enableBootAppB 0 to select app A, 1 to select app B. Returns error otherwise.
 */
obc_error_code_t blEnableBootApp(uint8_t enableBootAppB);

/**
 * @brief Chooses which app to write to
 *
 * @param enableWriteAppB 0 to select app A, 1 to select app B. Returns error otherwise.
 */
obc_error_code_t blEnableWriteApp(uint8_t enableWriteAppB);

/**
 * @brief Checks the integrity of the app and jumps to it's reset vector
 */
obc_error_code_t blJumpToApp();
