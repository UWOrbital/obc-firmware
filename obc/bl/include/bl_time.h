#pragma once
#include <sci.h>

#include "rti.h"

/* Initalizes the timer */
void blInitTick();

/* Gets the number of ticks since the timer was initialized */
uint32_t blGetCurrentTick();
