#include "bl_time.h"

#include <stdint.h>

static uint32_t initTime;

void blInitTick() {
  rtiInit();
  rtiStartCounter(rtiCOUNTER_BLOCK1);
  initTime = rtiGetCurrentTick(rtiCOMPARE1);
}

uint32_t blGetCurrentTick() { return (rtiGetCurrentTick(rtiCOMPARE1) - initTime) / 50; }
