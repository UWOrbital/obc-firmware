#include "obc_metadata.h"

#include "obc_errors.h"

extern void _c_int00(void);
extern uint32_t __crc_addr;

const metadata_t app_metadata __attribute__((section(".metadata"), used)) = {
    .magic_num = MAGIC_NUM,
    .build_num = 1,
    .git_hash = 0,
    .app_entry_func_addr = (uint32_t)&_c_int00,
    .release_ver_major = 0,
    .release_ver_minor = 1,
    .board_id = BOARD_ID,
    .reserved = {0},
    .crc_addr = (uint32_t)&__crc_addr,
};
