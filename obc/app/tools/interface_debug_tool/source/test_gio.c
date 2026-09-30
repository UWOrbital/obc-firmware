#include "test_gio.h"

#include "gio.h"
#include "obc_print.h"

void testGIO(void) {
  sciPrintf("Testing GIO...\r\n");
  /* GIOA has 8 pins */
  for (int i = 0; i < 8; i++) {
    gioSetBit(gioPORTA, i, 1);
  }

  /* GIOB has 4 pins */
  for (int i = 0; i < 4; i++) {
    gioSetBit(gioPORTB, i, 1);
  }

  sciPrintf("Set bit 0 to 7 on portA and bit 0 to 3 on portB to high\r\n");
}
