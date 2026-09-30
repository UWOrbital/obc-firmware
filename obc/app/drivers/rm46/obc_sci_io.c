#include "obc_sci_io.h"

#include <FreeRTOS.h>
#include <FreeRTOSConfig.h>
#include <os_portmacro.h>
#include <os_semphr.h>
#include <os_task.h>
#include <sci.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#include "obc_assert.h"
#include "obc_board_config.h"
#include "obc_errors.h"

static SemaphoreHandle_t sciReadMutex = NULL;
static SemaphoreHandle_t sciWriteMutex = NULL;
static StaticSemaphore_t sciReadMutexBuffer;
static StaticSemaphore_t sciWriteMutexBuffer;
static SemaphoreHandle_t sciLinReadMutex = NULL;
static SemaphoreHandle_t sciLinWriteMutex = NULL;
static StaticSemaphore_t sciLinReadMutexBuffer;
static StaticSemaphore_t sciLinWriteMutexBuffer;

// Semaphore to signal when an async transfer is complete
static SemaphoreHandle_t sciTransferComplete = NULL;
static StaticSemaphore_t sciTransferCompleteBuffer;
static SemaphoreHandle_t sciLinTransferComplete = NULL;
static StaticSemaphore_t sciLinTransferCompleteBuffer;

STATIC_ASSERT((UART_PRINT_REG == sciREG) || (UART_PRINT_REG == scilinREG),
              "UART_PRINT_REG must be sciREG or scilinREG");
STATIC_ASSERT((UART_READ_REG == sciREG) || (UART_READ_REG == scilinREG), "UART_READ_REG must be sciREG or scilinREG");

void initSciMutex(void) {
  if (sciReadMutex == NULL) {
    sciReadMutex = xSemaphoreCreateMutexStatic(&sciReadMutexBuffer);
  }
  configASSERT(sciReadMutex);

  if (sciWriteMutex == NULL) {
    sciWriteMutex = xSemaphoreCreateMutexStatic(&sciWriteMutexBuffer);
  }
  configASSERT(sciWriteMutex);

  if (sciLinReadMutex == NULL) {
    sciLinReadMutex = xSemaphoreCreateMutexStatic(&sciLinReadMutexBuffer);
  }
  configASSERT(sciLinReadMutex);

  if (sciLinWriteMutex == NULL) {
    sciLinWriteMutex = xSemaphoreCreateMutexStatic(&sciLinWriteMutexBuffer);
  }
  configASSERT(sciLinWriteMutex);

  if (sciTransferComplete == NULL) {
    sciTransferComplete = xSemaphoreCreateBinaryStatic(&sciTransferCompleteBuffer);
  }
  configASSERT(sciTransferComplete);

  if (sciLinTransferComplete == NULL) {
    sciLinTransferComplete = xSemaphoreCreateBinaryStatic(&sciLinTransferCompleteBuffer);
  }
  configASSERT(sciLinTransferComplete);
}

obc_error_code_t sciReadBytes(uint8_t *buf, size_t numBytes, TickType_t uartMutexTimeoutTicks,
                              size_t transferCompleteTimeoutTicks, sciBASE_t *sciReg) {
  obc_error_code_t errCode;

  if (!(sciReg == scilinREG || sciReg == sciREG)) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  if (buf == NULL || numBytes < 1) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  SemaphoreHandle_t mutex = NULL;
  SemaphoreHandle_t transferCompleteSemaphore = NULL;

  if (sciReg == sciREG) {
    mutex = sciReadMutex;
    transferCompleteSemaphore = sciTransferComplete;
  } else {
    mutex = sciLinReadMutex;
    transferCompleteSemaphore = sciLinTransferComplete;
  }

  configASSERT(mutex != NULL);
  configASSERT(transferCompleteSemaphore != NULL);

  if (xSemaphoreTake(mutex, uartMutexTimeoutTicks) != pdTRUE) {
    return OBC_ERR_CODE_MUTEX_TIMEOUT;
  }

  // Start asynchronous transfer
  sciReceive(sciReg, numBytes, buf);

  // Wait for transfer to complete
  if (xSemaphoreTake(transferCompleteSemaphore, transferCompleteTimeoutTicks) != pdTRUE) {
    errCode = OBC_ERR_CODE_SEMAPHORE_TIMEOUT;
  } else {
    errCode = OBC_ERR_CODE_SUCCESS;
  }

  xSemaphoreGive(mutex);
  return errCode;
}

obc_error_code_t sciSendBytes(uint8_t *buf, size_t numBytes, TickType_t uartMutexTimeoutTicks, sciBASE_t *sciReg) {
  if (!(sciReg == scilinREG || sciReg == sciREG)) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  if (buf == NULL || numBytes < 1) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  SemaphoreHandle_t mutex = (sciReg == sciREG) ? sciWriteMutex : sciLinWriteMutex;
  configASSERT(mutex != NULL);

  if (xSemaphoreTake(mutex, uartMutexTimeoutTicks) != pdTRUE) {
    return OBC_ERR_CODE_MUTEX_TIMEOUT;
  }

  sciSend(sciReg, numBytes, buf);

  xSemaphoreGive(mutex);
  return OBC_ERR_CODE_SUCCESS;
}

void sciNotification(sciBASE_t *sci, uint32 flags) {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  if (sci == sciREG) {
    switch (flags) {
      case SCI_RX_INT:
        xSemaphoreGiveFromISR(sciTransferComplete, &xHigherPriorityTaskWoken);
        break;
    }
  } else if (sci == scilinREG) {
    switch (flags) {
      case SCI_RX_INT:
        xSemaphoreGiveFromISR(sciLinTransferComplete, &xHigherPriorityTaskWoken);
        break;
    }
  } else {
    return;
  }
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
