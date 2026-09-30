#include "mpu6050.h"

#include <sci.h>
#include <stdio.h>

#include "obc_i2c_io.h"
#include "obc_sci_io.h"

#define UART_MUTEX_BLOCK_TIME portMAX_DELAY
#define I2C_TRANSFER_TIMEOUT pdMS_TO_TICKS(100)

void wakeupMPU6050(void) {
  uint8_t data = 0x00;
  sciPrintText((unsigned char *)"Trying to wake up MPU6050\r\n", 28, UART_MUTEX_BLOCK_TIME, scilinREG);
  i2cWriteReg(MPU6050_DEFAULT_ADDRESS, MPU6050_REG_PWR_MGMT_1, &data, 1);
  sciPrintText((unsigned char *)"MPU6050 Woken Up\r\n", 19, UART_MUTEX_BLOCK_TIME, scilinREG);
}

uint8_t readAccelDataMPU6050(double *accX, double *accY, double *accZ) {
  uint8_t data[6];
  if (i2cReadReg(MPU6050_DEFAULT_ADDRESS, MPU6050_REG_ACCEL_XOUT_H, data, 6) == 0) {
    sciPrintText((unsigned char *)"Failed to read acceleration data\r\n", 35, UART_MUTEX_BLOCK_TIME, scilinREG);
    return 0;
  }

  *accX = (int16_t)((data[0] << 8) | data[1]) * (9.81 / 16384.0);
  *accY = (int16_t)((data[2] << 8) | data[3]) * (9.81 / 16384.0);
  *accZ = (int16_t)((data[4] << 8) | data[5]) * (9.81 / 16384.0);

  char buf[31] = {};
  sprintf(buf, "X: %0.02f Y: %0.02f Z: %0.02f\r\n", *accX, *accY, *accZ);
  sciPrintText((unsigned char *)buf, sizeof(buf), UART_MUTEX_BLOCK_TIME, scilinREG);

  return 1;
}
