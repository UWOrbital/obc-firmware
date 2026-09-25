#pragma once
#include <stdint.h>

#define MAGIC_NUM 0xDEAD

typedef struct __attribute__((packed)) {
  uint16_t magic_num;
  uint32_t build_num;
  uint64_t git_hash;
  uint32_t app_entry_func_addr;
  uint8_t release_ver_major;
  uint8_t release_ver_minor;
  uint8_t board_id;
  uint8_t occupied_slot;
  uint8_t reserved[6];
  uint32_t crc_addr;
} metadata_t;
