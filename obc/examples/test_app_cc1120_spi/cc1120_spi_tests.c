#include "cc1120_spi_tests.h"

#include <string.h>

#include "cc1120_defs.h"
#include "cc1120_mcu.h"
#include "cc1120_spi.h"

uint8_t CC1120_REGS_DEFAULTS[CC1120_REGS_STD_SPACE_SIZE] = {
    CC1120_DEFAULTS_IOCFG3,         CC1120_DEFAULTS_IOCFG2,
    CC1120_DEFAULTS_IOCFG1,         CC1120_DEFAULTS_IOCFG0,
    CC1120_DEFAULTS_SYNC3,          CC1120_DEFAULTS_SYNC2,
    CC1120_DEFAULTS_SYNC1,          CC1120_DEFAULTS_SYNC0,
    CC1120_DEFAULTS_SYNC_CFG1,      CC1120_DEFAULTS_SYNC_CFG0,
    CC1120_DEFAULTS_DEVIATION_M,    CC1120_DEFAULTS_MODCFG_DEV_E,
    CC1120_DEFAULTS_DCFILT_CFG,     CC1120_DEFAULTS_PREAMBLE_CFG1,
    CC1120_DEFAULTS_PREAMBLE_CFG0,  CC1120_DEFAULTS_FREQ_IF_CFG,
    CC1120_DEFAULTS_IQIC,           CC1120_DEFAULTS_CHAN_BW,
    CC1120_DEFAULTS_MDMCFG1,        CC1120_DEFAULTS_MDMCFG0,
    CC1120_DEFAULTS_SYMBOL_RATE2,   CC1120_DEFAULTS_SYMBOL_RATE1,
    CC1120_DEFAULTS_SYMBOL_RATE0,   CC1120_DEFAULTS_AGC_REF,
    CC1120_DEFAULTS_AGC_CS_THR,     CC1120_DEFAULTS_AGC_GAIN_ADJUST,
    CC1120_DEFAULTS_AGC_CFG3,       CC1120_DEFAULTS_AGC_CFG2,
    CC1120_DEFAULTS_AGC_CFG1,       CC1120_DEFAULTS_AGC_CFG0,
    CC1120_DEFAULTS_FIFO_CFG,       CC1120_DEFAULTS_DEV_ADDR,
    CC1120_DEFAULTS_SETTLING_CFG,   CC1120_DEFAULTS_FS_CFG,
    CC1120_DEFAULTS_WOR_CFG1,       CC1120_DEFAULTS_WOR_CFG0,
    CC1120_DEFAULTS_WOR_EVENT0_MSB, CC1120_DEFAULTS_WOR_EVENT0_LSB,
    CC1120_DEFAULTS_PKT_CFG2,       CC1120_DEFAULTS_PKT_CFG1,
    CC1120_DEFAULTS_PKT_CFG0,       CC1120_DEFAULTS_RFEND_CFG1,
    CC1120_DEFAULTS_RFEND_CFG0,     CC1120_DEFAULTS_PA_CFG2,
    CC1120_DEFAULTS_PA_CFG1,        CC1120_DEFAULTS_PA_CFG0,
    CC1120_DEFAULTS_PKT_LEN};

/**
 * @brief E2E test for SPI read function.
 * Reads through all registers up to the extended register space,
 * and compares values to default values.
 * Burst reads all the values and compares them to the default values.
 * Burst reads FREQ registers in extended address space and compares them to defaults.
 *
 * @return OBC_ERROR_CODE_SUCCESS - If all registers are read correctly and have the right value.
 * @return An error code - If any register does not have the expected value,
 *                 or status byte is invalid.
 */
obc_error_code_t cc1120TestSpiRead(void) {
  obc_error_code_t errCode;
  uint8_t addr = 0x00U;
  uint8_t data;

  uint8_t burstData[CC1120_REGS_EXT_ADDR];
  errCode = cc1120ReadSpi(addr, burstData, CC1120_REGS_EXT_ADDR);

  LOG_IF_ERROR_CODE(errCode);
  RETURN_IF_ERROR_CODE(errCode);

  if (memcmp(CC1120_REGS_DEFAULTS, burstData, CC1120_REGS_EXT_ADDR)) {
    errCode = OBC_ERR_CODE_CC1120_TEST_FAILURE;
    LOG_ERROR_CODE(errCode);
    return errCode;
  }

  if (errCode == OBC_ERR_CODE_SUCCESS) {
    errCode = cc1120ReadExtAddrSpi(CC1120_REGS_EXT_MARCSTATE, &data, 1);

    LOG_IF_ERROR_CODE(errCode);
    RETURN_IF_ERROR_CODE(errCode);

    if (data != 0x41U) {
      errCode = OBC_ERR_CODE_CC1120_TEST_FAILURE;
      LOG_ERROR_CODE(errCode);
      return errCode;
    }
  }

  if (errCode == OBC_ERR_CODE_SUCCESS) {
    uint8_t extBurstData[3];
    uint8_t expected[3] = {0x00U, 0x00U, 0x00U};
    errCode = cc1120ReadExtAddrSpi(CC1120_REGS_EXT_FREQ2, extBurstData, 3);

    if (errCode != OBC_ERR_CODE_SUCCESS) {
      errCode = OBC_ERR_CODE_CC1120_TEST_FAILURE;
      LOG_ERROR_CODE(errCode);
      return errCode;
    }

    if (memcmp(extBurstData, expected, 3)) {
      errCode = OBC_ERR_CODE_CC1120_TEST_FAILURE;
      LOG_ERROR_CODE(errCode);
      return errCode;
    }
  }

  if (errCode == OBC_ERR_CODE_SUCCESS) LOG_INFO("CC1120 SPI read test passed.\n");

  return errCode;
}
