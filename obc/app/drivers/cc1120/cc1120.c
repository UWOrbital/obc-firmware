#include "cc1120.h"
#include "cc1120_defs.h"
#include "cc1120_mcu.h"
#include "obc_logging.h"
#include "obc_board_config.h"

#define READ_BIT 1 << 7
#define BURST_BIT 1 << 6

#define CHIP_READY_MASK 1 << 7
#define CHIP_READY 0
#define CHIP_STATE 0b1110000

#define CC1120_RND_CONFIG 0x80

static const register_setting_t cc1120SettingsStd[] = {
    // Set GPIO 0 to RXFIFO_THR_PKT
    {CC1120_REGS_IOCFG0, 0x01U},
    // Set GPIO 1 to HighZ
    {CC1120_REGS_IOCFG1, 0x30U},
    // Set GPIO 2 to PKT_SYNC_RXTX
    {CC1120_REGS_IOCFG2, 0x06U},
    // Set GPIO 3 to TXFIFO_THR
    {CC1120_REGS_IOCFG3, 0x02U},
    // Set the sync word as 16 bits and allow for < 2 bit error on sync word
    {CC1120_REGS_SYNC_CFG0, 0x09U},
    // Set sync word qualifier value threshold similar to the one talked about for preamble in section 6.8
    {CC1120_REGS_SYNC_CFG1, 0x08U},
    // Set first 8 bits of the sync word to 0x55U (arbitrary value)
    {CC1120_REGS_SYNC0, 0x55U},
    // Set next 8 bits of the sync word to 0x57U (arbitrary value)
    {CC1120_REGS_SYNC1, 0x57U},
    // Set cc1120 to switch to FSTXON state after a packet is received
    {CC1120_REGS_RFEND_CFG1, 0x1F},
    {CC1120_REGS_DEVIATION_M, 0x3AU},
    {CC1120_REGS_MODCFG_DEV_E, 0x0AU},
    {CC1120_REGS_DCFILT_CFG, 0x1CU},
    // Set the preamble as 4 bytes of 10101010
    {CC1120_REGS_PREAMBLE_CFG1, 0x18U},
    // enable preamble and set the error threshold
    {CC1120_REGS_PREAMBLE_CFG0, 0x2AU},
    {CC1120_REGS_IQIC, 0xC6U},
    {CC1120_REGS_CHAN_BW, 0x08U},
    {CC1120_REGS_MDMCFG0, 0x05U},
    {CC1120_REGS_SYMBOL_RATE2, 0x73U},
    {CC1120_REGS_AGC_REF, 0x20U},
    {CC1120_REGS_AGC_CS_THR, 0x19U},
    {CC1120_REGS_AGC_CFG2, 0x20U},
    {CC1120_REGS_AGC_CFG1, 0xA9U},
    {CC1120_REGS_AGC_CFG0, 0xCFU},
    {CC1120_REGS_FIFO_CFG, TXRX_INTERRUPT_THRESHOLD},
    {CC1120_REGS_FS_CFG, 0x14U},
    {CC1120_REGS_PKT_CFG0, 0x00U},
    {CC1120_REGS_PA_CFG0, 0x7DU},
    {CC1120_REGS_PKT_LEN, 0x0CU}};

static const register_setting_t cc1120SettingsExt[] = {{CC1120_REGS_EXT_IF_MIX_CFG, 0x00U},
                                                       {CC1120_REGS_EXT_FREQOFF_CFG, 0x34U},
                                                       {CC1120_REGS_EXT_FREQ2, 0x6CU},
                                                       {CC1120_REGS_EXT_FREQ1, 0x7AU},
                                                       {CC1120_REGS_EXT_FREQ0, 0xE1U},
                                                       {CC1120_REGS_EXT_FS_DIG1, 0x00U},
                                                       {CC1120_REGS_EXT_FS_DIG0, 0x5FU},
                                                       {CC1120_REGS_EXT_FS_CAL1, 0x40U},
                                                       {CC1120_REGS_EXT_FS_CAL0, 0x0EU},
                                                       {CC1120_REGS_EXT_FS_DIVTWO, 0x03U},
                                                       {CC1120_REGS_EXT_FS_DSM0, 0x33U},
                                                       {CC1120_REGS_EXT_FS_DVC0, 0x17U},
                                                       {CC1120_REGS_EXT_FS_PFD, 0x50U},
                                                       {CC1120_REGS_EXT_FS_PRE, 0x6EU},
                                                       {CC1120_REGS_EXT_FS_REG_DIV_CML, 0x14U},
                                                       {CC1120_REGS_EXT_FS_SPARE, 0xACU},
                                                       {CC1120_REGS_EXT_FS_VCO0, 0xB4U},
                                                       {CC1120_REGS_EXT_XOSC5, 0x0EU},
                                                       {CC1120_REGS_EXT_XOSC1, 0x03U},
                                                       {CC1120_REGS_EXT_TOC_CFG, 0x89U},
                                                       {CC1120_REGS_EXT_RNDGEN, CC1120_RND_CONFIG}};

/**
 * @brief - Reads from consecutive registers from the CC1120.
 *
 * @param addr - The address of the first register to read.
 * @param data - The array to store the read data, or a pointer to a single uint8_t if len=1.
 * @param len - The number of registers to read.
 * @return OBC_ERR_CODE_SUCCESS - If the read was successful.
 * @return An error code - If the register is not valid, or the errCode byte is invalid.
 */
obc_error_code_t cc1120ReadSpi(uint8_t addr, uint8_t data[], uint8_t len) {
  obc_error_code_t errCode;

  if (data == NULL) return OBC_ERR_CODE_INVALID_ARG;

  if (addr >= CC1120_REGS_EXT_ADDR) return OBC_ERR_CODE_INVALID_ARG;

  if (len < 1) return OBC_ERR_CODE_INVALID_ARG;

  uint8_t header = (len > 1) ? (READ_BIT | BURST_BIT | addr) : (READ_BIT | addr);

  RETURN_IF_ERROR_CODE(mcuCC1120CSAssert());
  CC1120_DEASSERT_RETURN_IF_ERROR_CODE(cc1120SendByteReceiveStatus(header));

  for (uint8_t i = 0; i < len; i++) CC1120_DEASSERT_RETURN_IF_ERROR_CODE(mcuCC1120SpiTransfer(0x00, &data[i]));

  RETURN_IF_ERROR_CODE(mcuCC1120CSDeassert());

  return OBC_ERR_CODE_SUCCESS;
}

/**
 * @brief - Reads from consecutive extended address space registers on the CC1120
 *
 * @param addr - The address of the first register to read.
 * @param data - The array to store the read data, or a pointer to a single uint8_t if len=1.
 * @param len - The number of registers to read.
 * @return OBC_ERR_CODE_SUCCESS - If the read was successful.
 * @return An error code - If the register is not valid, or the errCode byte is invalid.
 */
obc_error_code_t cc1120ReadExtAddrSpi(uint8_t addr, uint8_t data[], uint8_t len) {
  obc_error_code_t errCode;

  if (data == NULL) return OBC_ERR_CODE_INVALID_ARG;

  if ((addr >= CC1120_REGS_EXT_RESERVE_SPACE_1_START && addr <= CC1120_REGS_EXT_RESERVE_SPACE_1_END))
    return OBC_ERR_CODE_INVALID_ARG;

  if ((addr >= CC1120_REGS_EXT_RESERVE_SPACE_2_START && addr <= CC1120_REGS_EXT_RESERVE_SPACE_2_END))
    return OBC_ERR_CODE_INVALID_ARG;

  if (addr > CC1120_REGS_EXT_FIFO_NUM_RXBYTES) return OBC_ERR_CODE_INVALID_ARG;

  if (len < 1) return OBC_ERR_CODE_INVALID_ARG;

  uint8_t header = (len > 1) ? (READ_BIT | BURST_BIT | CC1120_REGS_EXT_ADDR) : (READ_BIT | CC1120_REGS_EXT_ADDR);

  RETURN_IF_ERROR_CODE(mcuCC1120CSAssert());
  CC1120_DEASSERT_RETURN_IF_ERROR_CODE(cc1120SendByteReceiveStatus(header));

  uint8_t recvData;
  CC1120_DEASSERT_RETURN_IF_ERROR_CODE(mcuCC1120SpiTransfer(addr, &recvData));
  if (recvData != 0x00) {  // When sending the extended address, SO will return all zeros. See section 3.2.
    errCode = OBC_ERR_CODE_CC1120_READ_EXT_ADDR_SPI_FAILED;
    LOG_ERROR_CODE(errCode);
    return errCode;
  }

  for (uint8_t i = 0; i < len; i++) CC1120_DEASSERT_RETURN_IF_ERROR_CODE(mcuCC1120SpiTransfer(0x00, &data[i]));

  RETURN_IF_ERROR_CODE(mcuCC1120CSDeassert());

  return OBC_ERR_CODE_SUCCESS;
}

/**
 * @brief - Writes to consecutive registers on the CC1120.
 *
 * @param addr - The address of the first register to write to.
 * @param data - The array of data to write to the registers, or a pointer to a single uint8_t if len=1.
 * @param len - The number of registers to write.
 * @return OBC_ERR_CODE_SUCCESS - If the write was successful.
 * @return An error code - If the register is not valid, or the errCode byte is invalid.
 */
obc_error_code_t cc1120WriteSpi(uint8_t addr, const uint8_t data[], uint8_t len) {
  obc_error_code_t errCode;

  if (data == NULL) return OBC_ERR_CODE_INVALID_ARG;

  if (addr >= CC1120_REGS_EXT_ADDR) return OBC_ERR_CODE_INVALID_ARG;

  if (len < 1) return OBC_ERR_CODE_INVALID_ARG;

  uint8_t header = (len > 1) ? (BURST_BIT | addr) : addr;

  RETURN_IF_ERROR_CODE(mcuCC1120CSAssert());
  CC1120_DEASSERT_RETURN_IF_ERROR_CODE(cc1120SendByteReceiveStatus(header));

  for (uint8_t i = 0; i < len; i++) CC1120_DEASSERT_RETURN_IF_ERROR_CODE(cc1120SendByteReceiveStatus(data[i]));

  RETURN_IF_ERROR_CODE(mcuCC1120CSDeassert());

  return OBC_ERR_CODE_SUCCESS;
}

/**
 * @brief - Writes to consecutive extended address space registers on the CC1120.
 *
 * @param addr - The address of the first register to write to.
 * @param data - The array of data to write to the registers, or a pointer to a single uint8_t if len=1.
 * @param len - The number of registers to write.
 * @return OBC_ERR_CODE_SUCCESS - If the write was successful.
 * @return An error code - If the register is not valid, or the errCode byte is invalid.
 */
obc_error_code_t cc1120WriteExtAddrSpi(uint8_t addr, const uint8_t data[], uint8_t len) {
  obc_error_code_t errCode;

  if (data == NULL) return OBC_ERR_CODE_INVALID_ARG;

  if ((addr >= CC1120_REGS_EXT_RESERVE_SPACE_1_START && addr <= CC1120_REGS_EXT_RESERVE_SPACE_1_END))
    return OBC_ERR_CODE_INVALID_ARG;

  if ((addr >= CC1120_REGS_EXT_RESERVE_SPACE_2_START && addr <= CC1120_REGS_EXT_RESERVE_SPACE_2_END))
    return OBC_ERR_CODE_INVALID_ARG;

  if (addr > CC1120_REGS_EXT_FIFO_NUM_RXBYTES) return OBC_ERR_CODE_INVALID_ARG;

  if (len < 1) return OBC_ERR_CODE_INVALID_ARG;

  uint8_t header = (len > 1) ? (BURST_BIT | CC1120_REGS_EXT_ADDR) : CC1120_REGS_EXT_ADDR;

  RETURN_IF_ERROR_CODE(mcuCC1120CSAssert());
  CC1120_DEASSERT_RETURN_IF_ERROR_CODE(cc1120SendByteReceiveStatus(header));

  uint8_t recvData;
  CC1120_DEASSERT_RETURN_IF_ERROR_CODE(mcuCC1120SpiTransfer(addr, &recvData));
  if (recvData != 0x00) {  // When sending the extended address, SO will return all zeros. See section 3.2.
    errCode = OBC_ERR_CODE_CC1120_WRITE_EXT_ADDR_SPI_FAILED;
    LOG_ERROR_CODE(errCode);
    return errCode;
  }

  for (uint8_t i = 0; i < len; i++) CC1120_DEASSERT_RETURN_IF_ERROR_CODE(cc1120SendByteReceiveStatus(data[i]));

  RETURN_IF_ERROR_CODE(mcuCC1120CSDeassert());

  return OBC_ERR_CODE_SUCCESS;
}

/**
 * @brief Calls a strobe command on the CC1120.
 *
 * @param addr - The address of the strobe command.
 * @return OBC_ERR_CODE_SUCCESS - If the strobe command was successful.
 * @return An error code - If the register is not valid, or the errCode byte is invalid.
 */
obc_error_code_t cc1120StrobeSpi(uint8_t addr) {
  obc_error_code_t errCode;

  if (addr < CC1120_STROBE_SRES || addr > CC1120_STROBE_SNOP) return OBC_ERR_CODE_INVALID_ARG;

  RETURN_IF_ERROR_CODE(mcuCC1120CSAssert());
  CC1120_DEASSERT_RETURN_IF_ERROR_CODE(cc1120SendByteReceiveStatus(addr));

  RETURN_IF_ERROR_CODE(mcuCC1120CSDeassert());

  return OBC_ERR_CODE_SUCCESS;
}

/**
 * @brief - Reads consecutive registers from the RX FIFO memory.
 *
 * @param data - The array to store the read data, or a pointer to a single uint8_t if len=1.
 * @param len - The number of registers to read.
 * @return OBC_ERR_CODE_SUCCESS - If the read was successful.
 * @return An error code - If the errCode byte is invalid.
 */
obc_error_code_t cc1120ReadFifo(uint8_t data[], uint8_t len) {
  obc_error_code_t errCode;

  if (data == NULL) return OBC_ERR_CODE_INVALID_ARG;

  if (len < 1 || len > CC1120_TX_FIFO_SIZE) return OBC_ERR_CODE_INVALID_ARG;

  uint8_t header =
      (len > 1) ? (READ_BIT | BURST_BIT | CC1120_REGS_FIFO_ACCESS_STD) : (READ_BIT | CC1120_REGS_FIFO_ACCESS_STD);

  RETURN_IF_ERROR_CODE(mcuCC1120CSAssert());
  CC1120_DEASSERT_RETURN_IF_ERROR_CODE(cc1120SendByteReceiveStatus(header));

  for (uint8_t i = 0; i < len; i++) CC1120_DEASSERT_RETURN_IF_ERROR_CODE(mcuCC1120SpiTransfer(0x00, &data[i]));

  RETURN_IF_ERROR_CODE(mcuCC1120CSDeassert());

  return OBC_ERR_CODE_SUCCESS;
}

/**
 * @brief - Writes consecutive registers to the TX FIFO memory.
 *
 * @param data - The array of data to write to the registers, or a pointer to a single uint8_t if len=1.
 * @param len - The number of registers to write.
 * @return OBC_ERR_CODE_SUCCESS - If the write was successful.
 * @return An error code - If the errCode byte is invalid.
 */
obc_error_code_t cc1120WriteFifo(uint8_t data[], uint8_t len) {
  obc_error_code_t errCode;

  if (data == NULL) return OBC_ERR_CODE_INVALID_ARG;

  if (len < 1 || len > CC1120_TX_FIFO_SIZE) return OBC_ERR_CODE_INVALID_ARG;

  uint8_t header = (len > 1) ? (BURST_BIT | CC1120_REGS_FIFO_ACCESS_STD) : CC1120_REGS_FIFO_ACCESS_STD;

  RETURN_IF_ERROR_CODE(mcuCC1120CSAssert());
  CC1120_DEASSERT_RETURN_IF_ERROR_CODE(cc1120SendByteReceiveStatus(header));

  uint8_t ignore;
  for (uint8_t i = 0; i < len; i++) CC1120_DEASSERT_RETURN_IF_ERROR_CODE(mcuCC1120SpiTransfer(data[i], &ignore));

  RETURN_IF_ERROR_CODE(mcuCC1120CSDeassert());

  return OBC_ERR_CODE_SUCCESS;
}

/**
 * @brief - Reads consecutive registers directly from the FIFO on the CC1120.
 *
 * @param addr - The address of the first register to read. Range 0x00 - 0xFF.
 * @param data - The array to store the read data, or a pointer to a single uint8_t if len=1.
 * @param len - The number of registers to read.
 * @return OBC_ERR_CODE_SUCCESS - If the read was successful.
 * @return An error code - If the register is not valid, or the errCode byte is invalid.
 */
obc_error_code_t cc1120ReadFifoDirect(uint8_t addr, uint8_t data[], uint8_t len) {
  obc_error_code_t errCode;

  if (data == NULL) return OBC_ERR_CODE_INVALID_ARG;

  if (len < 1) return OBC_ERR_CODE_INVALID_ARG;

  uint8_t header =
      (len > 1) ? (READ_BIT | BURST_BIT | CC1120_REGS_FIFO_ACCESS_DIR) : (READ_BIT | CC1120_REGS_FIFO_ACCESS_DIR);

  RETURN_IF_ERROR_CODE(mcuCC1120CSAssert());
  CC1120_DEASSERT_RETURN_IF_ERROR_CODE(cc1120SendByteReceiveStatus(header));

  uint8_t ignore;
  CC1120_DEASSERT_RETURN_IF_ERROR_CODE(mcuCC1120SpiTransfer(addr, &ignore));
  for (uint8_t i = 0; i < len; i++) CC1120_DEASSERT_RETURN_IF_ERROR_CODE(mcuCC1120SpiTransfer(0x00, &data[i]));

  RETURN_IF_ERROR_CODE(mcuCC1120CSDeassert());

  return OBC_ERR_CODE_SUCCESS;
}

/**
 * @brief - Writes consecutive registers directly to the FIFO on the CC1120.
 *
 * @param addr - The address of the first register to write to. Range 0x00 - 0xFF.
 * @param data - The array of data to write to the registers, or a pointer to a single uint8_t if len=1.
 * @param len - The number of registers to write.
 * @return OBC_ERR_CODE_SUCCESS - If the write was successful.
 * @return An error code - If the register is not valid, or the errCode byte is invalid.
 */
obc_error_code_t cc1120WriteFifoDirect(uint8_t addr, uint8_t data[], uint8_t len) {
  obc_error_code_t errCode = OBC_ERR_CODE_SUCCESS;

  if (data == NULL) return OBC_ERR_CODE_INVALID_ARG;

  if (len < 1) return OBC_ERR_CODE_INVALID_ARG;

  uint8_t header = (len > 1) ? (BURST_BIT | CC1120_REGS_FIFO_ACCESS_DIR) : CC1120_REGS_FIFO_ACCESS_DIR;

  RETURN_IF_ERROR_CODE(mcuCC1120CSAssert());
  CC1120_DEASSERT_RETURN_IF_ERROR_CODE(cc1120SendByteReceiveStatus(header));

  uint8_t ignore;
  CC1120_DEASSERT_RETURN_IF_ERROR_CODE(mcuCC1120SpiTransfer(addr, &ignore));
  for (uint8_t i = 0; i < len; i++) CC1120_DEASSERT_RETURN_IF_ERROR_CODE(mcuCC1120SpiTransfer(data[i], &ignore));

  RETURN_IF_ERROR_CODE(mcuCC1120CSDeassert());
  return OBC_ERR_CODE_SUCCESS;
}

/**
 * @brief - Reads the status register on the CC1120 and consecutively sends a byte over SPI.
 *
 * @param data - The data to send to the status register.
 * @return OBC_ERR_CODE_SUCCESS - If the status byte is valid.
 * @return OBC_ERR_CODE_CC1120_INVALID_STATUS_BYTE - If the status byte is invalid.
 */
obc_error_code_t cc1120SendByteReceiveStatus(uint8_t data) {
  obc_error_code_t errCode;
  uint8_t ccStatus;

  for (uint8_t i = 1; i <= 5; i++) {
    RETURN_IF_ERROR_CODE(mcuCC1120SpiTransfer(data, &ccStatus));
    if ((ccStatus & CHIP_READY_MASK) == CHIP_READY) {
      return OBC_ERR_CODE_SUCCESS;
    }
    /*
    Might be better to wait until the MISO pin goes low and then check 
    instead of always implementing a delay, severity depends on how many times this is called
    */
    vTaskDelay(pdMS_TO_TICKS(1));
  }
  switch ((ccStatus & CHIP_STATE) >> 4) {
    case 0b100:
      return OBC_ERR_CODE_CC1120_CHIP_CALIBRATING;
    case 0b101:
      return OBC_ERR_CODE_CC1120_CHIP_SETTLING;
    case 0b110:
      RETURN_IF_ERROR_CODE(cc1120StrobeSpi(CC1120_STROBE_SFRX));
      return OBC_ERR_CODE_CC1120_RX_FIFO_ERROR;
    case 0b111:
      RETURN_IF_ERROR_CODE(cc1120StrobeSpi(CC1120_STROBE_SFTX));
      return OBC_ERR_CODE_CC1120_TX_FIFO_ERROR;
    default:
      return OBC_ERR_CODE_CC1120_INVALID_STATUS_BYTE;
  }
}
/**
 * @brief Gets the number of bytes queued in the TX FIFO
 *
 * @param numBytes - A pointer to an 8-bit integer to store the number of bytes in
 * @return obc_error_code_t - Whether or not the registe read was successful
 */
obc_error_code_t cc1120GetBytesInTxFifo(uint8_t *numBytes) {
  if (numBytes == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }
  obc_error_code_t errCode;
  RETURN_IF_ERROR_CODE(cc1120ReadExtAddrSpi(CC1120_REGS_EXT_NUM_TXBYTES, numBytes, 1));
  return OBC_ERR_CODE_SUCCESS;
}

/**
 * @brief Gets the state of the CC1120 from the MARCSTATE register
 *
 * @param stateNum - A pointer to an 8-bit integer to store the state in
 * @return obc_error_code_t - Whether or not the register read was successful
 */
obc_error_code_t cc1120GetState(cc1120_state_t *stateNum) {
  if (stateNum == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }
  obc_error_code_t errCode;
  RETURN_IF_ERROR_CODE(cc1120ReadExtAddrSpi(CC1120_REGS_EXT_MARCSTATE, stateNum, 1));
  *stateNum &= 0b11111;
  return OBC_ERR_CODE_SUCCESS;
}

/**
 * @brief Resets CC1120 & initializes transmit mode
 *
 * @return obc_error_code_t - Whether or not the setup was a success
 */
obc_error_code_t cc1120Init(void) {
  obc_error_code_t errCode;

  RETURN_IF_ERROR_CODE(cc1120StrobeSpi(CC1120_STROBE_SRES));

  // When changing which signals are sent by each gpio, the output will be unstable so interrupts should be disabled
  // see chapter 3.4 in the datasheet for more info
  gioDisableNotification(gioPORTB, CC1120_RX_THR_PKT_gioPORTB_PIN);
  gioDisableNotification(gioPORTA, CC1120_TX_THR_PKT_gioPORTA_PIN);
  gioDisableNotification(gioPORTA, CC1120_PKT_SYNC_RXTX_gioPORTA_PIN);

  for (uint8_t i = 0; i < sizeof(cc1120SettingsStd) / sizeof(register_setting_t); i++) {
    RETURN_IF_ERROR_CODE(cc1120WriteSpi(cc1120SettingsStd[i].addr, &cc1120SettingsStd[i].val, 1));
  }

  for (uint8_t i = 0; i < sizeof(cc1120SettingsExt) / sizeof(register_setting_t); i++) {
    RETURN_IF_ERROR_CODE(cc1120WriteExtAddrSpi(cc1120SettingsExt[i].addr, &cc1120SettingsExt[i].val, 1));
  }

  // enable interrupts again now that the gpio signals are set
  gioEnableNotification(gioPORTB, CC1120_RX_THR_PKT_gioPORTB_PIN);
  gioEnableNotification(gioPORTA, CC1120_TX_THR_PKT_gioPORTA_PIN);
  gioEnableNotification(gioPORTA, CC1120_PKT_SYNC_RXTX_gioPORTA_PIN);
  RETURN_IF_ERROR_CODE(cc1120StrobeSpi(CC1120_STROBE_SFSTXON));

  return OBC_ERR_CODE_SUCCESS;
}

/**
 * @brief Gets the number of bytes queued in the RX FIFO
 *
 * @param numBytes - A pointer to an 8-bit integer to store the number of bytes in
 * @return obc_error_code_t - Whether or not the register read was successful
 */
obc_error_code_t cc1120GetBytesInRxFifo(uint8_t *numBytes) {
  if (numBytes == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }
  obc_error_code_t errCode;
  RETURN_IF_ERROR_CODE(cc1120ReadExtAddrSpi(CC1120_REGS_EXT_NUM_RXBYTES, numBytes, 1));
  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t cc1120Rng(uint8_t *randomValue) {
  obc_error_code_t errCode;
  uint8_t receivedData;
  if (randomValue == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }
  RETURN_IF_ERROR_CODE(cc1120StrobeSpi(CC1120_STROBE_SRX));
  RETURN_IF_ERROR_CODE(cc1120ReadExtAddrSpi(CC1120_REGS_EXT_RNDGEN, randomValue, 1));
  RETURN_IF_ERROR_CODE(cc1120ReadFifo(&receivedData, 1));
  (*randomValue) &= ~(1 << 7);
  (*randomValue) ^= receivedData;
  return OBC_ERR_CODE_SUCCESS;
}
