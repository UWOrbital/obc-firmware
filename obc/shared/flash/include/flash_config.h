#pragma once

#include <stdint.h>

/* THIS FILE IS ONLY USED BY bl_flash.c */

typedef struct {
  void *start;
  uint32_t length;  // number of 32-bit words
  uint32_t bankNumber;
  uint32_t sectorNumber;
  uint32_t flashBaseAddress;
} sector_info_t;

typedef struct {
  void *start;
  uint32_t length;  // number of 32-bit words
  uint32_t bankNumber;
  uint32_t flashBaseAddress;
  uint32_t numOfSectors;
} bank_info_t;

#define NUM_FLASH_SECTORS 16
const sector_info_t flashSectors[NUM_FLASH_SECTORS] = {
    {(void *)0x00000000, 0x04000, 0, 0, 0xfff87000},  {(void *)0x00004000, 0x04000, 0, 1, 0xfff87000},
    {(void *)0x00008000, 0x04000, 0, 2, 0xfff87000},  {(void *)0x0000C000, 0x04000, 0, 3, 0xfff87000},
    {(void *)0x00010000, 0x04000, 0, 4, 0xfff87000},  {(void *)0x00014000, 0x04000, 0, 5, 0xfff87000},
    {(void *)0x00018000, 0x08000, 0, 6, 0xfff87000},  {(void *)0x00020000, 0x20000, 0, 7, 0xfff87000},
    {(void *)0x00040000, 0x20000, 0, 8, 0xfff87000},  {(void *)0x00060000, 0x20000, 0, 9, 0xfff87000},
    {(void *)0x00080000, 0x20000, 0, 10, 0xfff87000}, {(void *)0x000A0000, 0x20000, 0, 11, 0xfff87000},
    {(void *)0x000C0000, 0x20000, 0, 12, 0xfff87000}, {(void *)0x000E0000, 0x20000, 0, 13, 0xfff87000},
    {(void *)0x00100000, 0x20000, 0, 14, 0xfff87000}, {(void *)0x00120000, 0x20000, 0, 15, 0xfff87000}};

#define NUM_FLASH_BANKS 1
const bank_info_t flashBanks[NUM_FLASH_BANKS] = {
    {(void *)0x00000000, 0x140000, 0, 0xfff87000, 16},
};
