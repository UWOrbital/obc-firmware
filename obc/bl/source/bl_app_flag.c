#include "bl_app_flag.h"

const app_flag_t app_metadata __attribute__((section(".app_flag"), used)) = {
    .app_status = 0xFFF03333,
    .boot_attempts = 0xFFF03333,
    .app_b_flag = 0x00000000,
    .crc_selector = ENABLE_APP_MAGIC_NUM,
};
