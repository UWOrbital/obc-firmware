#include "obc_i2c_io.h"
#include "obc_errors.h"
#include "obc_assert.h"
#include "obc_logging.h"

#include <FreeRTOS.h>
#include <os_portmacro.h>
#include <os_semphr.h>
#include <os_task.h>

#include <i2c.h>
#include <sys_common.h>

// The I2C bus to use for the OBC
#define I2C_REG i2cREG1

// Max number of bytes you can send when calling i2cWriteReg
#define I2C_WRITE_REG_MAX_BYTES 32U

// The mutex timeout for the I2C bus
#define I2C_MUTEX_TIMEOUT portMAX_DELAY

// Timeout to wait for a transfer to complete
#define I2C_TRANSFER_TIMEOUT pdMS_TO_TICKS(100)

STATIC_ASSERT(I2C_REG == i2cREG1, "I2C_REG must be i2cREG1");

static SemaphoreHandle_t i2cMutex = NULL;
static StaticSemaphore_t i2cMutexBuffer;

// Semaphore to signal when a transfer is complete
static SemaphoreHandle_t i2cTransferComplete = NULL;
static StaticSemaphore_t i2cTransferCompleteBuffer;

void initI2CMutex(void) {
  if (i2cMutex == NULL) {
    i2cMutex = xSemaphoreCreateMutexStatic(&i2cMutexBuffer);
  }

  ASSERT(i2cMutex != NULL);

  if (i2cTransferComplete == NULL) {
    i2cTransferComplete = xSemaphoreCreateBinaryStatic(&i2cTransferCompleteBuffer);
  }

  ASSERT(i2cTransferComplete != NULL);
}

obc_error_code_t i2cSendTo(uint8_t sAddr, uint16_t size, uint8_t *buf, TickType_t mutexTimeoutTicks,
                           TickType_t transferTimeoutTicks) {
  obc_error_code_t errCode;

  ASSERT(i2cMutex != NULL);

  if (buf == NULL || size < 1) return OBC_ERR_CODE_INVALID_ARG;

  if (xSemaphoreTake(i2cMutex, mutexTimeoutTicks) != pdTRUE) {
    return OBC_ERR_CODE_MUTEX_TIMEOUT;
  }

  i2cSetSlaveAdd(I2C_REG, sAddr);
  i2cSetDirection(I2C_REG, I2C_TRANSMITTER);
  i2cSetCount(I2C_REG, size);
  i2cSetMode(I2C_REG, I2C_MASTER);
  i2cSetStop(I2C_REG);
  i2cSetStart(I2C_REG);

  i2cSend(I2C_REG, size, buf);

  if (xSemaphoreTake(i2cTransferComplete, transferTimeoutTicks) != pdTRUE) {
    errCode = OBC_ERR_CODE_I2C_TRANSFER_TIMEOUT;
    i2cSetStop(I2C_REG);
  } else {
    errCode = OBC_ERR_CODE_SUCCESS;
  }

  i2cClearSCD(I2C_REG);

  xSemaphoreGive(i2cMutex);  // Won't fail because the mutex is taken correctly
  return errCode;
}

obc_error_code_t i2cReceiveFrom(uint8_t sAddr, uint16_t size, uint8_t *buf, TickType_t mutexTimeoutTicks,
                                TickType_t transferTimeoutTicks) {
  obc_error_code_t errCode;

  ASSERT(i2cMutex != NULL);

  if (buf == NULL || size < 1) return OBC_ERR_CODE_INVALID_ARG;

  if (xSemaphoreTake(i2cMutex, mutexTimeoutTicks) != pdTRUE) {
    return OBC_ERR_CODE_MUTEX_TIMEOUT;
  }

  i2cSetSlaveAdd(I2C_REG, sAddr);
  i2cSetDirection(I2C_REG, I2C_RECEIVER);
  i2cSetCount(I2C_REG, size);
  i2cSetMode(I2C_REG, I2C_MASTER);
  i2cSetStop(I2C_REG);
  i2cSetStart(I2C_REG);

  i2cReceive(I2C_REG, size, buf);

  if (xSemaphoreTake(i2cTransferComplete, transferTimeoutTicks) != pdTRUE) {
    errCode = OBC_ERR_CODE_I2C_TRANSFER_TIMEOUT;
    i2cSetStop(I2C_REG);
  } else {
    errCode = OBC_ERR_CODE_SUCCESS;
  }

  /* Clear the Stop condition */
  i2cClearSCD(I2C_REG);
  xSemaphoreGive(i2cMutex);

  return errCode;
}

obc_error_code_t i2cReadReg(uint8_t sAddr, uint8_t reg, uint8_t *data, uint16_t numBytes,
                            TickType_t transferTimeoutTicks) {
  obc_error_code_t errCode;

  ASSERT(i2cMutex != NULL);

  if (data == NULL || numBytes < 1) return OBC_ERR_CODE_INVALID_ARG;

  RETURN_IF_ERROR_CODE(i2cSendTo(sAddr, 1, &reg, I2C_MUTEX_TIMEOUT, transferTimeoutTicks));

  RETURN_IF_ERROR_CODE(i2cReceiveFrom(sAddr, numBytes, data, I2C_MUTEX_TIMEOUT, transferTimeoutTicks));

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t i2cWriteReg(uint8_t sAddr, uint8_t reg, uint8_t *data, uint8_t numBytes) {
  obc_error_code_t errCode;

  ASSERT(i2cMutex != NULL);

  if (data == NULL || numBytes < 1 || numBytes > I2C_WRITE_REG_MAX_BYTES) return OBC_ERR_CODE_INVALID_ARG;

  uint8_t dataBuf[I2C_WRITE_REG_MAX_BYTES + 1];
  dataBuf[0] = reg;

  for (int i = 0; i < numBytes; i++) {
    dataBuf[i + 1] = data[i];
  }

  RETURN_IF_ERROR_CODE(i2cSendTo(sAddr, numBytes + 1, dataBuf, I2C_MUTEX_TIMEOUT, I2C_TRANSFER_TIMEOUT));
  return OBC_ERR_CODE_SUCCESS;
}

void i2cNotification(i2cBASE_t *i2c, uint32 flags) {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  if (flags & I2C_SCD_INT) {
    if (xSemaphoreGiveFromISR(i2cTransferComplete, &xHigherPriorityTaskWoken) != pdTRUE) {
      LOG_ERROR_FROM_ISR("Semaphore cannot be given or was already given.");
    }
  }

  if (flags & I2C_NACK_INT) {
    i2c->STR = (uint32)I2C_NACK_INT;
    i2cSetStop(i2c);
  }

  if (flags & I2C_AL_INT) {
    i2c->STR = (uint32)I2C_AL_INT;
    i2cSetStop(i2c);
  }

  if (flags & I2C_ARDY_INT) {
    i2cSetStop(i2c);
  }

  if (flags & I2C_AAS_INT) {
    i2cSetStop(i2c);
  }

  i2cClearSCD(i2c);

  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
