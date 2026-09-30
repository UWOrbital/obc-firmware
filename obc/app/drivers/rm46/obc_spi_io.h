#pragma once

#include <FreeRTOS.h>
#include <gio.h>
#include <os_semphr.h>
#include <spi.h>
#include <stdint.h>

#include "obc_errors.h"

#define DEASSERT_RETURN_IF_ERROR_CODE(_spiPort, _csNum, _ret) \
  do {                                                        \
    errCode = _ret;                                           \
    if (errCode != OBC_ERR_CODE_SUCCESS) {                    \
      LOG_ERROR_CODE(errCode);                                \
      deassertChipSelect(_spiPort, _csNum);                   \
      return errCode;                                         \
    }                                                         \
  } while (0)

/**
 * @brief Initialize mutexes protecting SPI ports.
 */
void initSpiMutex(void);

/**
 * @brief Deselect chip select; set the chip select pin high.
 * @param spiPort The SPI port to use.
 * @param csNum The chip select pin to use.
 * @return Error code. OBC_ERR_CODE_SUCCESS if successful.
 */
obc_error_code_t deassertChipSelect(gioPORT_t *spiPort, uint8_t csNum);

/**
 * @brief Select chip select; set the chip select pin low.
 * @param spiPort The SPI port to use.
 * @param csNum The chip select pin to use.
 * @return Error code. OBC_ERR_CODE_SUCCESS if successful.
 */
obc_error_code_t assertChipSelect(gioPORT_t *spiPort, uint8_t csNum);

/**
 * @brief Take the mutex for the specified SPI port and chip select pin.
 *
 * @param spiReg The SPI register to use.
 *
 * @note This will not assert the CS pin. This function is intended to be used
 * to send data to a device that requires the CS pin to be deasserted.
 *
 * @return obc_error_code_t OBC_ERR_CODE_SUCCESS if successful, error code otherwise.
 */
obc_error_code_t spiTakeBusMutex(spiBASE_t *spiReg);

/**
 * @brief Release the mutex for the specified SPI port and chip select pin.
 *
 * @param spiReg The SPI register to use.
 *
 * @note This will not deassert the CS pin. This function is intended to be used
 * to send data to a device that requires the CS pin to be deasserted.
 *
 * @return obc_error_code_t OBC_ERR_CODE_SUCCESS if successful, error code otherwise.
 */
obc_error_code_t spiReleaseBusMutex(spiBASE_t *spiReg);

/**
 * @brief Send a byte via SPI.
 * @param spiReg The SPI register to use.
 * @param spiDataFormat The SPI data format options.
 * @param outb The byte to send.
 * @return Error code. OBC_ERR_CODE_SUCCESS if successful.
 */
obc_error_code_t spiTransmitByte(spiBASE_t *spiReg, spiDAT1_t *spiDataFormat, uint8_t outb);

/**
 * @brief Send multiple bytes via SPI.
 * @param spiReg The SPI register to use.
 * @param spiDataFormat The SPI data format options.
 * @param outBytes The bytes to send.
 * @param numBytes The number of bytes to send.
 * @return Error code. OBC_ERR_CODE_SUCCESS if successful.
 */
obc_error_code_t spiTransmitBytes(spiBASE_t *spiReg, spiDAT1_t *spiDataFormat, uint8_t *outBytes, size_t numBytes);

/**
 * @brief Receive a byte via SPI.
 * @param spiReg The SPI register to use.
 * @param spiDataFormat The SPI data format options.
 * @param inb Buffer to store the received byte.
 * @return Error code. OBC_ERR_CODE_SUCCESS if successful.
 */
obc_error_code_t spiReceiveByte(spiBASE_t *spiReg, spiDAT1_t *spiDataFormat, uint8_t *inb);

/**
 * @brief Receive multiple bytes via SPI.
 * @param spiReg The SPI register to use.
 * @param spiDataFormat The SPI data format options.
 * @param inBytes Buffer to store the received byte.
 * @param numBytes The number of bytes to receive
 * @return Error code. OBC_ERR_CODE_SUCCESS if successful.
 */
obc_error_code_t spiReceiveBytes(spiBASE_t *spiReg, spiDAT1_t *spiDataFormat, uint8_t *inBytes, size_t numBytes);

/**
 * @brief Send and receive a byte via SPI.
 * @param spiReg The SPI register to use.
 * @param spiDataFormat The SPI data format options.
 * @param outb The byte to send.
 * @param inb Buffer to store the received byte.
 * @return Error code. OBC_ERR_CODE_SUCCESS if successful.
 */
obc_error_code_t spiTransmitAndReceiveByte(spiBASE_t *spiReg, spiDAT1_t *spiDataFormat, uint8_t outb, uint8_t *inb);

/**
 * @brief Send and receive multiple bytes via SPI.
 * @param spiReg The SPI register to use.
 * @param spiDataFormat The SPI data format options.
 * @param outBytes The byte to send.
 * @param inBytes Buffer to store the received byte.
 * @param numBytes The number of bytes to send and receive
 * @return Error code. OBC_ERR_CODE_SUCCESS if successful.
 */
obc_error_code_t spiTransmitAndReceiveBytes(spiBASE_t *spiReg, spiDAT1_t *spiDataFormat, uint8_t *outBytes,
                                            uint8_t *inBytes, size_t numBytes);

/**
 * @brief Check if the current task owns the SPI bus mutex.
 *
 * @param spiMutex The SPI bus mutex to check.
 * @return true if the current task owns the SPI bus mutex; false otherwise.
 */
bool isSpiBusOwner(SemaphoreHandle_t spiMutex);

/**
 * @brief Get the Mutex for the specified SPI port and chip select pin.
 *
 * @param spi The SPI reg to use.
 * @param mutex The mutex for the specified SPI port and chip select pin.
 * @return obc_error_code_t OBC_ERR_CODE_SUCCESS if successful, error code otherwise.
 */
obc_error_code_t getSpiMutex(spiBASE_t *spi, SemaphoreHandle_t *mutex);
