#include <gio.h>
#include <sci.h>

#include "obc_print.h"
#include "obc_sci_io.h"

#define UART_MUTEX_BLOCK_TIME portMAX_DELAY

int main(void) {
  // Initialize hardware.
  gioInit();
  sciInit();

  // Initialize the SCI mutex.
  initSciPrint();

  while (1) {
    // Send a string of text via SCI
    sciPrintText((unsigned char *)"Hello from SCI!\r\n", 20, UART_MUTEX_BLOCK_TIME);

    // Test sciPrintf
    sciPrintf("Testing sciPrintf: %d %d %s\r\n", 0, 1, "Hello");

    // Toggle the LED.
    gioToggleBit(gioPORTB, 1);

    // Simple delay.
    for (volatile int i = 0; i < 1000000; i++) {
      // Do nothing.
    }
  }

  return 0;
}
