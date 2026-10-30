#include "obc_gs_command_data.h"

// Struct to store cmd message and enabled status for app->bl forwarded command messages
const fcmd_msg_t forwarded_cmd __attribute__((section(".fcmd_msg"), used)) = {0};
