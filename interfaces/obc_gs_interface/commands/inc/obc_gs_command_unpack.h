#pragma once

#include "obc_gs_command_data.h"
#include "obc_gs_errors.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

obc_gs_error_code_t unpackCmdMsg(const uint8_t* buffer, uint32_t* offset, cmd_msg_t* cmdMsg);

#ifdef __cplusplus
}
#endif
