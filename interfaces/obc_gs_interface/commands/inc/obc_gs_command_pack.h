#pragma once

#include "obc_gs_command_data.h"
#include "obc_gs_errors.h"

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

obc_gs_error_code_t packCmdMsg(uint8_t* buffer, uint32_t* offset, const cmd_msg_t* cmdMsg, uint8_t* numPacked);

#ifdef __cplusplus
}
#endif
