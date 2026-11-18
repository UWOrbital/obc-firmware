#include "bl_app_flag.h"

const app_flag_t bl_app_A_flag __attribute__((section(".app_A_flag"), used)) = {
    .app_state = VALIDATED,
    .boot_attempts = 0,
};

const app_flag_t bl_app_B_flag __attribute__((section(".app_B_flag"), used)) = {
    .app_state = VALIDATED,
    .boot_attempts = 0,
};
