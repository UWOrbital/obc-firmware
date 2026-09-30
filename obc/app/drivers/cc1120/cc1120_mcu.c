#include "cc1120_mcu.h"

#include "obc_spi_io.h"

static const spiDAT1_t spiConfig = {.CS_HOLD = false, .WDEL = false, .DFSEL = CC1120_SPI_FMT, .CSNR = SPI_CS_NONE};

/**
 * @brief Simultaneously sends and receives a byte over CC1120 SPI interface
 *
 * @param outb - Data to transfer
 * @param inb - Buffer to store the received byte.
 * @return error code - An error code from obc_errors.h
 */
obc_error_code_t mcuCC1120SpiTransfer(uint8_t outb, uint8_t *inb) {
  obc_error_code_t errCode;
  RETURN_IF_ERROR_CODE(spiTransmitAndReceiveByte(CC1120_SPI_REG, (spiDAT1_t *)&spiConfig, outb, inb));
  return OBC_ERR_CODE_SUCCESS;
}

/**
 * @brief Calls the correct CS assert function based on the MCU selected.
 *
 * @return error code - An error code from obc_errors.h
 */
obc_error_code_t mcuCC1120CSAssert(void) {
  obc_error_code_t errCode;
  RETURN_IF_ERROR_CODE(assertChipSelect(CC1120_SPI_PORT, CC1120_SPI_CS));
  return OBC_ERR_CODE_SUCCESS;
}

/**
 * @brief Calls the correct CS deassert function based on the MCU selected.
 *
 * @return error code - An error code from obc_errors.h
 */
obc_error_code_t mcuCC1120CSDeassert(void) {
  obc_error_code_t errCode;
  RETURN_IF_ERROR_CODE(deassertChipSelect(CC1120_SPI_PORT, CC1120_SPI_CS));
  return OBC_ERR_CODE_SUCCESS;
}
