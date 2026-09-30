#include "obc_spi_io.h"

#include <FreeRTOS.h>
#include <gio.h>
#include <os_semphr.h>
#include <os_task.h>
#include <spi.h>
#include <stdint.h>

#include "obc_errors.h"
#include "obc_logging.h"

// This includes SPI2 which isn't available on the RM46 PGE package
#define NUM_SPI_PORTS 5

// SPIFLG Errors
#define SPI_FLAG_ERR_MASK 0xFFU   // All errors are shown in the lowest byte of SPIFLG
#define SPI_FLAG_SUCCESS 0x00U    // No errors
#define SPI_FLAG_DLENERR 0x01U    // Data length error
#define SPI_FLAG_TIMEOUT 0x02U    // Timeout error
#define SPI_FLAG_PARERR 0x04U     // Parity error
#define SPI_FLAG_DESYNC 0x08U     // Desynchronization error
#define SPI_FLAG_BITERR 0x10U     // Bit error
#define SPI_FLAG_RXOVRNINT 0x40U  // Receive overrun interrupt flag

#define SPI_BLOCKING_TIMEOUT pdMS_TO_TICKS(1000)

#define CS_ASSERTED 0
#define CS_DEASSERTED 1

/**
 * @brief Log Spi Tx/Rx Errors.
 * @param spiErr Spi error flag.
 */
static void spiLogErrors(uint32_t spiErr);

// TODO: Deprecate this once codebase is refactored to allow non-SPI pins to be used as chip selects
static spiBASE_t *portToReg(gioPORT_t *port);

static SemaphoreHandle_t spiMutexes[NUM_SPI_PORTS];
static StaticSemaphore_t spiMutexBuffers[NUM_SPI_PORTS];

void initSpiMutex(void) {
  for (int i = 0; i < NUM_SPI_PORTS; i++) {
    spiMutexes[i] = xSemaphoreCreateRecursiveMutexStatic(&spiMutexBuffers[i]);

    // configASSERT used since ASSERT may log to microSD card (a SPI device)
    configASSERT(spiMutexes[i]);
  }
}

obc_error_code_t spiTakeBusMutex(spiBASE_t *spi) {
  obc_error_code_t errCode;

  SemaphoreHandle_t spiMutex;
  RETURN_IF_ERROR_CODE(getSpiMutex(spi, &spiMutex));

  if (xSemaphoreTakeRecursive(spiMutex, SPI_BLOCKING_TIMEOUT) == pdTRUE) {
    return OBC_ERR_CODE_SUCCESS;
  }

  return OBC_ERR_CODE_MUTEX_TIMEOUT;
}

obc_error_code_t spiReleaseBusMutex(spiBASE_t *spi) {
  obc_error_code_t errCode;

  SemaphoreHandle_t spiMutex;
  RETURN_IF_ERROR_CODE(getSpiMutex(spi, &spiMutex));

  if (!isSpiBusOwner(spiMutex)) {
    return OBC_ERR_CODE_NOT_MUTEX_OWNER;
  }

  if (xSemaphoreGiveRecursive(spiMutex) != pdTRUE) {
    return OBC_ERR_CODE_UNKNOWN;
  }

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t assertChipSelect(gioPORT_t *csPort, uint8_t csPin) {
  obc_error_code_t errCode;

  // We currently assume csPort is a SPI port
  // An additional arg (spiReg) is needed before we deprecate portToReg
  spiBASE_t *spiReg = portToReg(csPort);
  if (spiReg == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  RETURN_IF_ERROR_CODE(spiTakeBusMutex(spiReg));
  gioSetBit(csPort, csPin, CS_ASSERTED);

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t deassertChipSelect(gioPORT_t *spiPort, uint8_t csNum) {
  obc_error_code_t errCode;

  // We currently assume csPort is a SPI port
  // An additional arg (spiReg) is needed before we deprecate portToReg
  spiBASE_t *spiReg = portToReg(spiPort);
  if (spiReg == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  SemaphoreHandle_t spiMutex;
  RETURN_IF_ERROR_CODE(getSpiMutex(spiReg, &spiMutex));

  // Since the CS pin must be deasserted before releasing the mutex, we need to do this check
  // before spiReleaseBusMutex
  if (!isSpiBusOwner(spiMutex)) {
    return OBC_ERR_CODE_NOT_MUTEX_OWNER;
  }

  gioSetBit(spiPort, csNum, CS_DEASSERTED);
  RETURN_IF_ERROR_CODE(spiReleaseBusMutex(spiReg));

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t spiTransmitByte(spiBASE_t *spiReg, spiDAT1_t *spiDataFormat, uint8_t outb) {
  obc_error_code_t errCode;

  RETURN_IF_ERROR_CODE(spiTransmitBytes(spiReg, spiDataFormat, &outb, 1));
  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t spiReceiveByte(spiBASE_t *spiReg, spiDAT1_t *spiDataFormat, uint8_t *inb) {
  obc_error_code_t errCode;

  RETURN_IF_ERROR_CODE(spiReceiveBytes(spiReg, spiDataFormat, inb, 1));
  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t spiTransmitAndReceiveByte(spiBASE_t *spiReg, spiDAT1_t *spiDataFormat, uint8_t outb, uint8_t *inb) {
  obc_error_code_t errCode;

  RETURN_IF_ERROR_CODE(spiTransmitAndReceiveBytes(spiReg, spiDataFormat, &outb, inb, 1));
  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t spiTransmitBytes(spiBASE_t *spiReg, spiDAT1_t *spiDataFormat, uint8_t *outBytes, size_t numBytes) {
  obc_error_code_t errCode;

  if (spiReg == NULL || outBytes == NULL || spiDataFormat == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  SemaphoreHandle_t spiMutex;
  RETURN_IF_ERROR_CODE(getSpiMutex(spiReg, &spiMutex));

  if (!isSpiBusOwner(spiMutex)) {
    return OBC_ERR_CODE_NOT_MUTEX_OWNER;
  }

  uint16_t spiWordOut;
  for (size_t i = 0; i < numBytes; i++) {
    // The SPI HAL functions take 16-bit arguments, but we're using 8-bit word size
    spiWordOut = (uint16_t)outBytes[i];

    uint32_t spiErr = spiTransmitData(spiReg, spiDataFormat, 1, &spiWordOut) & SPI_FLAG_ERR_MASK;

    if (spiErr != SPI_FLAG_SUCCESS) {
      spiLogErrors(spiErr);
      return OBC_ERR_CODE_SPI_FAILURE;
    }
  }

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t spiReceiveBytes(spiBASE_t *spiReg, spiDAT1_t *spiDataFormat, uint8_t *inBytes, size_t numBytes) {
  obc_error_code_t errCode;

  if (spiReg == NULL || spiDataFormat == NULL || inBytes == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  SemaphoreHandle_t spiMutex;
  RETURN_IF_ERROR_CODE(getSpiMutex(spiReg, &spiMutex));

  if (!isSpiBusOwner(spiMutex)) {
    return OBC_ERR_CODE_NOT_MUTEX_OWNER;
  }

  uint16_t spiWordIn;
  for (size_t i = 0; i < numBytes; i++) {
    // The SPI HAL functions take 16-bit arguments, but we're using 8-bit word size
    spiWordIn = (uint16_t)inBytes[i];

    uint32_t spiErr = spiReceiveData(spiReg, spiDataFormat, 1, &spiWordIn) & SPI_FLAG_ERR_MASK;

    if (spiErr != SPI_FLAG_SUCCESS) {
      spiLogErrors(spiErr);
      return OBC_ERR_CODE_SPI_FAILURE;
    }

    inBytes[i] = (uint8_t)(spiWordIn & 0xFFU);
  }

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t spiTransmitAndReceiveBytes(spiBASE_t *spiReg, spiDAT1_t *spiDataFormat, uint8_t *outBytes,
                                            uint8_t *inBytes, size_t numBytes) {
  obc_error_code_t errCode;

  if (spiReg == NULL || inBytes == NULL || outBytes == NULL || spiDataFormat == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  SemaphoreHandle_t spiMutex;
  RETURN_IF_ERROR_CODE(getSpiMutex(spiReg, &spiMutex));

  if (!isSpiBusOwner(spiMutex)) {
    return OBC_ERR_CODE_NOT_MUTEX_OWNER;
  }

  uint16_t spiWordOut;
  uint16_t spiWordIn;
  for (size_t i = 0; i < numBytes; i++) {
    // The SPI HAL functions take 16-bit arguments, but we're using 8-bit word size
    spiWordOut = (uint16_t)outBytes[i];

    uint32_t spiErr = spiTransmitAndReceiveData(spiReg, spiDataFormat, 1, &spiWordOut, &spiWordIn) & SPI_FLAG_ERR_MASK;

    if (spiErr != SPI_FLAG_SUCCESS) {
      spiLogErrors(spiErr);
      return OBC_ERR_CODE_SPI_FAILURE;
    }

    inBytes[i] = (uint8_t)(spiWordIn & 0xFFU);
  }

  return OBC_ERR_CODE_SUCCESS;
}

static void spiLogErrors(uint32_t spiErr) {
  if (spiErr & SPI_FLAG_DLENERR) LOG_ERROR_CODE(OBC_ERR_CODE_SPI_DATA_LENGTH_ERROR);
  if (spiErr & SPI_FLAG_TIMEOUT) LOG_ERROR_CODE(OBC_ERR_CODE_SPI_TIMEOUT);
  if (spiErr & SPI_FLAG_PARERR) LOG_ERROR_CODE(OBC_ERR_CODE_SPI_PARITY_ERROR);
  if (spiErr & SPI_FLAG_DESYNC) LOG_ERROR_CODE(OBC_ERR_CODE_SPI_DESYNC_ERROR);
  if (spiErr & SPI_FLAG_BITERR) LOG_ERROR_CODE(OBC_ERR_CODE_SPI_BIT_ERROR);
  if (spiErr & SPI_FLAG_RXOVRNINT) LOG_ERROR_CODE(OBC_ERR_CODE_SPI_RX_OVERRUN);
}

bool isSpiBusOwner(SemaphoreHandle_t spiMutex) {
  TaskHandle_t owner = xSemaphoreGetMutexHolder(spiMutex);
  TaskHandle_t currentTask = xTaskGetCurrentTaskHandle();

  return currentTask == owner;
}

obc_error_code_t getSpiMutex(spiBASE_t *spi, SemaphoreHandle_t *mutex) {
  if (spi == NULL || mutex == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  switch ((uint32)spi) {
    case (uint32)spiREG1:
      *mutex = spiMutexes[0];
      break;
    case (uint32)spiREG2:
      *mutex = spiMutexes[1];
      break;
    case (uint32)spiREG3:
      *mutex = spiMutexes[2];
      break;
    case (uint32)spiREG4:
      *mutex = spiMutexes[3];
      break;
    case (uint32)spiREG5:
      *mutex = spiMutexes[4];
      break;
    default:
      return OBC_ERR_CODE_INVALID_ARG;
  }

  return OBC_ERR_CODE_SUCCESS;
}

// TODO: Deprecate this once codebase is refactored to allow non-SPI pins to be used as chip selects
static spiBASE_t *portToReg(gioPORT_t *port) {
  // Map spi Ports to spi Registers
  switch ((uint32)port) {
    case (uint32)spiPORT1:
      return spiREG1;
    case (uint32)spiPORT2:
      return spiREG2;
    case (uint32)spiPORT3:
      return spiREG3;
    case (uint32)spiPORT4:
      return spiREG4;
    case (uint32)spiPORT5:
      return spiREG5;
  }

  return NULL;
}
