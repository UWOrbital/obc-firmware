#include "fcmd_msg.h"

// Struct to store cmd message and enabled status for app->bl forwarded command messages
const fcmd_msg_t forwarded_cmd __attribute__((section(".fcmd_msg"), used)) = {
    .fCmd = {0},  // Do some shit
    .fEnabled = 0,
};
