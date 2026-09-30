#include "obc_digital_watchdog.h"

#include <reg_rti.h>
#include <rti.h>
#include <stdint.h>
#include <system.h>

#include "obc_assert.h"

// Watchdog is fed by writing these two values to the WDKEY register
#define RESET_DWD_CMD1 0xE51AUL
#define RESET_DWD_CMD2 0xA35CUL

#define DWD_CTRL_ENABLE 0xA98559DAUL

// Preload value is used to set the timeout period
#define MIN_PRELOAD_VAL 0
#define MAX_PRELOAD_VAL 0xFFF

// Assuming RTI frequency of 73.333 MHz, this
// corresponds to a timeout of ~0.45 seconds
// based on the formula: timeout = (PRELOAD_VAL + 1) * 2^13 / RTI_FREQ
#define PRELOAD_VAL 0xFBB

// This check does not explicitly check for 73.333 MHz, but it will fail if the RTI frequency changed too much
STATIC_ASSERT((uint32_t)RTI_FREQ == 73, "RTI frequency is not 73.333 MHz");
STATIC_ASSERT(PRELOAD_VAL >= MIN_PRELOAD_VAL && PRELOAD_VAL <= MAX_PRELOAD_VAL, "Preload value is out of range");

void feedDigitalWatchdog(void) {
  rtiREG1->WDKEY = RESET_DWD_CMD1;
  rtiREG1->WDKEY = RESET_DWD_CMD2;
}

void initDigitalWatchdog(void) {
  rtiREG1->DWDPRLD = PRELOAD_VAL;
  rtiREG1->WWDSIZECTRL = Size_100_Percent;
  rtiREG1->WWDRXNCTRL = Generate_Reset;
  rtiREG1->DWDCTRL = DWD_CTRL_ENABLE;
}
