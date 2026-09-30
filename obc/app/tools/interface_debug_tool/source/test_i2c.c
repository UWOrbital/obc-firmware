#include "test_i2c.h"

#include "obc_i2c_io.h"
#include "obc_print.h"

#define I2C_MUTEX_TIMEOUT portMAX_DELAY
#define I2C_TRANSFER_TIMEOUT pdMS_TO_TICKS(100)

void testI2C(void) {
  sciPrintf("Testing I2C...\r\n");
  uint8_t messageData[] = {0xff, 0x00, 0xff};
  if (i2cSendTo(0x00, 3, messageData, I2C_MUTEX_TIMEOUT, I2C_TRANSFER_TIMEOUT) != OBC_ERR_CODE_SUCCESS) {
    sciPrintf(
        "Failed sending message through I2C\r\n"
        "The failure might because no I2C slave device at address 0x00, "
        "no actual data bytes are sent since no acknowledgement from I2C slave device\r\n");
    return;
  }
  sciPrintf("Successfully sending message (0xff, 0x00, 0xff) through I2C...\r\n");
}
