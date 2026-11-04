#pragma once
#include <stddef.h>
#include <stdint.h>
typedef int BaseType_t;
typedef uint32_t TickType_t;
typedef unsigned UBaseType_t;
#define pdFALSE 0
#define pdPASS 1
#define portMAX_DELAY UINT32_MAX
#define portYIELD_FROM_ISR(value) ((void)(value))
