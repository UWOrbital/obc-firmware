#pragma once
#include <stdint.h>
#define RED_O_WRONLY 1
#define RED_O_APPEND 2
#define RED_O_CREAT 4
int32_t red_open(const char *path, uint32_t flags);
int32_t red_write(int32_t descriptor, const void *data, uint32_t length);
int32_t red_close(int32_t descriptor);
