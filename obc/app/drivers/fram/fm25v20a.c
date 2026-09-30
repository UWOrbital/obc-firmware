#include "fm25v20a.h"

#include <FreeRTOS.h>
#include <stdlib.h>
#include <sys_common.h>

#include "obc_board_config.h"
#include "obc_errors.h"
#include "obc_logging.h"
#include "obc_spi_io.h"
#include "spi.h"

// SPI values
static spiDAT1_t framSPIDataFmt = {.CS_HOLD = 0, .CSNR = SPI_CS_NONE, .DFSEL = FRAM_spiFMT, .WDEL = 0};

// FRAM OPCODES
#define OP_WRITE_ENABLE 0x06U
#define OP_WRITE_RESET 0x04U
#define OP_READ_STAT_REG 0x05U
#define OP_WRITE_STAT_REG 0x01U

#define OP_READ 0x03U
#define OP_FREAD 0x0BU
#define OP_WRITE 0x02U

#define OP_SLEEP 0xB9U
#define OP_GET_ID 0x9FU

#define FRAM_WAKE_BUSY_WAIT \
  99000U  // Assume RM46 clk is 220 MHz, value for wait loop should give ~450us delay (datasheet pg.15)
#define FRAM_WAKE_TIME_MS \
  2U  // Not woken up often, so ok to use minimum task delay of 2 ms, value expirementally determined to be 2 ¯\_(ツ)_/¯
static bool isAsleep = false;

typedef enum cmd {
  FRAM_READ_STATUS_REG,   // Read Status Register
  FRAM_READ,              // Normal read
  FRAM_FAST_READ,         // Fast read, Note this is used for serial flash compatibility not to read data fast!
  FRAM_READ_ID,           // Get Device ID
  FRAM_WRITE_EN,          // Set Write EN
  FRAM_WRITE_EN_RESET,    // Reset Write EN
  FRAM_WRITE_STATUS_REG,  // Write to Status Register
  FRAM_WRITE,             // Write memory data
  FRAM_SLEEP              // Put FRAM to sleep
} cmd_t;

// Function Declarations
static obc_error_code_t framTransmitOpCode(cmd_t cmd);
static obc_error_code_t framTransmitAddress(uint32_t addr);

// CS assumed to be asserted
static obc_error_code_t framTransmitOpCode(cmd_t cmd) {
  obc_error_code_t errCode;
  switch (cmd) {
    case FRAM_READ:
      LOG_IF_ERROR_CODE(spiTransmitByte(FRAM_spiREG, &framSPIDataFmt, OP_READ));
      break;
    case FRAM_FAST_READ:
      LOG_IF_ERROR_CODE(spiTransmitByte(FRAM_spiREG, &framSPIDataFmt, OP_FREAD));
      break;
    case FRAM_READ_STATUS_REG:
      LOG_IF_ERROR_CODE(spiTransmitByte(FRAM_spiREG, &framSPIDataFmt, OP_READ_STAT_REG));
      break;
    case FRAM_READ_ID:
      LOG_IF_ERROR_CODE(spiTransmitByte(FRAM_spiREG, &framSPIDataFmt, OP_GET_ID));
      break;
    case FRAM_WRITE_EN:
      LOG_IF_ERROR_CODE(spiTransmitByte(FRAM_spiREG, &framSPIDataFmt, OP_WRITE_ENABLE));
      break;
    case FRAM_WRITE_EN_RESET:
      LOG_IF_ERROR_CODE(spiTransmitByte(FRAM_spiREG, &framSPIDataFmt, OP_WRITE_RESET));
      break;
    case FRAM_WRITE_STATUS_REG:
      LOG_IF_ERROR_CODE(spiTransmitByte(FRAM_spiREG, &framSPIDataFmt, OP_WRITE_STAT_REG));
      break;
    case FRAM_WRITE:
      LOG_IF_ERROR_CODE(spiTransmitByte(FRAM_spiREG, &framSPIDataFmt, OP_WRITE));
      break;
    case FRAM_SLEEP:
      LOG_IF_ERROR_CODE(spiTransmitByte(FRAM_spiREG, &framSPIDataFmt, OP_SLEEP));
      break;
    default:
      LOG_ERROR_CODE(OBC_ERR_CODE_INVALID_ARG);
      errCode = OBC_ERR_CODE_INVALID_ARG;
  }

  return errCode;
}

// CS assumed to be asserted
static obc_error_code_t framTransmitAddress(uint32_t addr) {
  obc_error_code_t errCode;
  if (addr > FRAM_MAX_ADDRESS) {
    return OBC_ERR_CODE_FRAM_ADDRESS_OUT_OF_RANGE;
  }

  // Send last 3 bytes MSB first
  for (int i = 2; i >= 0; i--) {
    uint8_t byte = (addr >> (i * 8)) & (0xFF);
    RETURN_IF_ERROR_CODE(spiTransmitByte(FRAM_spiREG, &framSPIDataFmt, byte));
  }
  return OBC_ERR_CODE_SUCCESS;
}

void initFRAM(void) { isAsleep = false; }

obc_error_code_t framReadStatusReg(uint8_t *status) {
  obc_error_code_t errCode;
  if (status == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }
  // Recursive take mutex
  RETURN_IF_ERROR_CODE(spiTakeBusMutex(FRAM_spiREG));

  if (isAsleep) {
    LOG_ERROR_CODE(OBC_ERR_CODE_FRAM_IS_ASLEEP);
    errCode = OBC_ERR_CODE_FRAM_IS_ASLEEP;
  }

  if (errCode == OBC_ERR_CODE_SUCCESS) {
    LOG_IF_ERROR_CODE(assertChipSelect(FRAM_spiPORT, FRAM_CS));
  }

  if (errCode == OBC_ERR_CODE_SUCCESS) {
    LOG_IF_ERROR_CODE(framTransmitOpCode(FRAM_READ_STATUS_REG));
  }

  if (errCode == OBC_ERR_CODE_SUCCESS) {
    LOG_IF_ERROR_CODE(spiReceiveByte(FRAM_spiREG, &framSPIDataFmt, status));
  }
  obc_error_code_t prev_errCode = errCode;
  if (errCode != OBC_ERR_CODE_FRAM_IS_ASLEEP) {
    // Do not deassert since never asserted in the first place
    RETURN_IF_ERROR_CODE(deassertChipSelect(FRAM_spiPORT, FRAM_CS));
  }
  // Reset overwritten error code
  errCode = prev_errCode;
  RETURN_IF_ERROR_CODE(spiReleaseBusMutex(FRAM_spiREG));
  // Reset overwritten error code
  errCode = prev_errCode;
  return errCode;
}

obc_error_code_t framWriteStatusReg(uint8_t status) {
  obc_error_code_t errCode;
  // Recursive take mutex
  RETURN_IF_ERROR_CODE(spiTakeBusMutex(FRAM_spiREG));

  if (isAsleep) {
    LOG_ERROR_CODE(OBC_ERR_CODE_FRAM_IS_ASLEEP);
    errCode = OBC_ERR_CODE_FRAM_IS_ASLEEP;
  }

  // Send WREN
  if (errCode == OBC_ERR_CODE_SUCCESS) {
    LOG_IF_ERROR_CODE(assertChipSelect(FRAM_spiPORT, FRAM_CS));
  }
  if (errCode == OBC_ERR_CODE_SUCCESS) {
    LOG_IF_ERROR_CODE(framTransmitOpCode(FRAM_WRITE_EN));
  }

  // Deassert and assert needed for write operations (datasheet pg. 9)
  if (errCode == OBC_ERR_CODE_SUCCESS) {
    LOG_IF_ERROR_CODE(deassertChipSelect(FRAM_spiPORT, FRAM_CS));
  }
  if (errCode == OBC_ERR_CODE_SUCCESS) {
    LOG_IF_ERROR_CODE(assertChipSelect(FRAM_spiPORT, FRAM_CS));
  }

  if (errCode == OBC_ERR_CODE_SUCCESS) {
    LOG_IF_ERROR_CODE(framTransmitOpCode(FRAM_WRITE_STATUS_REG));
  }
  if (errCode == OBC_ERR_CODE_SUCCESS) {
    LOG_IF_ERROR_CODE(spiTransmitByte(FRAM_spiREG, &framSPIDataFmt, status));
  }

  obc_error_code_t prev_errCode = errCode;
  if (errCode != OBC_ERR_CODE_FRAM_IS_ASLEEP) {
    // Do not deassert since never asserted in the first place
    RETURN_IF_ERROR_CODE(deassertChipSelect(FRAM_spiPORT, FRAM_CS));
  }
  // Reset overwritten error code
  errCode = prev_errCode;
  RETURN_IF_ERROR_CODE(spiReleaseBusMutex(FRAM_spiREG));
  // Reset overwritten error code
  errCode = prev_errCode;
  return errCode;
}

obc_error_code_t framFastRead(uint32_t addr, uint8_t *buffer, size_t nBytes) {
  obc_error_code_t errCode;
  if (buffer == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  // Recursive take mutex
  RETURN_IF_ERROR_CODE(spiTakeBusMutex(FRAM_spiREG));

  if (isAsleep) {
    LOG_ERROR_CODE(OBC_ERR_CODE_FRAM_IS_ASLEEP);
    errCode = OBC_ERR_CODE_FRAM_IS_ASLEEP;
  }

  if (errCode == OBC_ERR_CODE_SUCCESS) {
    LOG_IF_ERROR_CODE(assertChipSelect(FRAM_spiPORT, FRAM_CS));
  }

  if (errCode == OBC_ERR_CODE_SUCCESS) {
    LOG_IF_ERROR_CODE(framTransmitOpCode(FRAM_FAST_READ));
  }

  if (errCode == OBC_ERR_CODE_SUCCESS) {
    LOG_IF_ERROR_CODE(framTransmitAddress(addr));
  }

  // Send dummy byte
  if (errCode == OBC_ERR_CODE_SUCCESS) {
    LOG_IF_ERROR_CODE(spiTransmitByte(FRAM_spiREG, &framSPIDataFmt, 0xFF));
  }

  for (uint32_t i = 0; i < nBytes; i++) {
    if (errCode == OBC_ERR_CODE_SUCCESS) {
      uint8_t receiveByte;
      LOG_IF_ERROR_CODE(spiReceiveByte(FRAM_spiREG, &framSPIDataFmt, &receiveByte));
      buffer[i] = receiveByte;
    } else {
      break;
    }
  }

  obc_error_code_t prev_errCode = errCode;
  if (errCode != OBC_ERR_CODE_FRAM_IS_ASLEEP) {
    // Do not deassert since never asserted in the first place
    RETURN_IF_ERROR_CODE(deassertChipSelect(FRAM_spiPORT, FRAM_CS));
  }
  // Reset overwritten error code
  errCode = prev_errCode;
  RETURN_IF_ERROR_CODE(spiReleaseBusMutex(FRAM_spiREG));
  // Reset overwritten error code
  errCode = prev_errCode;
  return errCode;
}

obc_error_code_t framRead(uint32_t addr, uint8_t *buffer, size_t nBytes) {
  obc_error_code_t errCode;
  if (buffer == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  // Recursive take mutex
  RETURN_IF_ERROR_CODE(spiTakeBusMutex(FRAM_spiREG));

  if (isAsleep) {
    LOG_ERROR_CODE(OBC_ERR_CODE_FRAM_IS_ASLEEP);
    errCode = OBC_ERR_CODE_FRAM_IS_ASLEEP;
  }

  if (errCode == OBC_ERR_CODE_SUCCESS) {
    LOG_IF_ERROR_CODE(assertChipSelect(FRAM_spiPORT, FRAM_CS));
  }

  if (errCode == OBC_ERR_CODE_SUCCESS) {
    LOG_IF_ERROR_CODE(framTransmitOpCode(FRAM_READ));
  }

  if (errCode == OBC_ERR_CODE_SUCCESS) {
    LOG_IF_ERROR_CODE(framTransmitAddress(addr));
  }

  for (uint32_t i = 0; i < nBytes; i++) {
    if (errCode == OBC_ERR_CODE_SUCCESS) {
      uint8_t receiveByte;
      LOG_IF_ERROR_CODE(spiReceiveByte(FRAM_spiREG, &framSPIDataFmt, &receiveByte));
      buffer[i] = receiveByte;
    } else {
      break;
    }
  }

  obc_error_code_t prev_errCode = errCode;
  if (errCode != OBC_ERR_CODE_FRAM_IS_ASLEEP) {
    // Do not deassert since never asserted in the first place
    RETURN_IF_ERROR_CODE(deassertChipSelect(FRAM_spiPORT, FRAM_CS));
  }
  // Reset overwritten error code
  errCode = prev_errCode;
  RETURN_IF_ERROR_CODE(spiReleaseBusMutex(FRAM_spiREG));
  // Reset overwritten error code
  errCode = prev_errCode;
  return errCode;
}

obc_error_code_t framWrite(uint32_t addr, const uint8_t *data, size_t nBytes) {
  obc_error_code_t errCode;
  if (data == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  // Recursive take mutex
  RETURN_IF_ERROR_CODE(spiTakeBusMutex(FRAM_spiREG));

  if (isAsleep) {
    LOG_ERROR_CODE(OBC_ERR_CODE_FRAM_IS_ASLEEP);
    errCode = OBC_ERR_CODE_FRAM_IS_ASLEEP;
  }

  // Send WREN
  if (errCode == OBC_ERR_CODE_SUCCESS) {
    LOG_IF_ERROR_CODE(assertChipSelect(FRAM_spiPORT, FRAM_CS));
  }

  if (errCode == OBC_ERR_CODE_SUCCESS) {
    LOG_IF_ERROR_CODE(framTransmitOpCode(FRAM_WRITE_EN));
  }

  // Deassert and assert needed for write operations (datasheet pg. 9)
  if (errCode == OBC_ERR_CODE_SUCCESS) {
    LOG_IF_ERROR_CODE(deassertChipSelect(FRAM_spiPORT, FRAM_CS));
  }
  if (errCode == OBC_ERR_CODE_SUCCESS) {
    LOG_IF_ERROR_CODE(assertChipSelect(FRAM_spiPORT, FRAM_CS));
  }

  if (errCode == OBC_ERR_CODE_SUCCESS) {
    LOG_IF_ERROR_CODE(framTransmitOpCode(FRAM_WRITE));
  }
  if (errCode == OBC_ERR_CODE_SUCCESS) {
    LOG_IF_ERROR_CODE(framTransmitAddress(addr));
  }

  for (uint32_t i = 0; i < nBytes; i++) {
    if (errCode == OBC_ERR_CODE_SUCCESS) {
      LOG_IF_ERROR_CODE(spiTransmitByte(FRAM_spiREG, &framSPIDataFmt, data[i]));
    } else {
      break;
    }
  }

  obc_error_code_t prev_errCode = errCode;
  if (errCode != OBC_ERR_CODE_FRAM_IS_ASLEEP) {
    // Do not deassert since never asserted in the first place
    RETURN_IF_ERROR_CODE(deassertChipSelect(FRAM_spiPORT, FRAM_CS));
  }
  // Reset overwritten error code
  errCode = prev_errCode;
  RETURN_IF_ERROR_CODE(spiReleaseBusMutex(FRAM_spiREG));
  // Reset overwritten error code
  errCode = prev_errCode;
  return errCode;
}

obc_error_code_t framSleep(void) {
  obc_error_code_t errCode;
  // Recursive take mutex
  RETURN_IF_ERROR_CODE(spiTakeBusMutex(FRAM_spiREG));
  if (isAsleep) {
    RETURN_IF_ERROR_CODE(spiReleaseBusMutex(FRAM_spiREG));
    return OBC_ERR_CODE_SUCCESS;
  }
  if (errCode == OBC_ERR_CODE_SUCCESS) {
    LOG_IF_ERROR_CODE(assertChipSelect(FRAM_spiPORT, FRAM_CS));
  }
  if (errCode == OBC_ERR_CODE_SUCCESS) {
    LOG_IF_ERROR_CODE(framTransmitOpCode(FRAM_SLEEP));
  }
  if (errCode == OBC_ERR_CODE_SUCCESS) {
    LOG_IF_ERROR_CODE(deassertChipSelect(FRAM_spiPORT, FRAM_CS));
  }
  if (errCode == OBC_ERR_CODE_SUCCESS) {
    isAsleep = true;
  }
  RETURN_IF_ERROR_CODE(spiReleaseBusMutex(FRAM_spiREG));
  return errCode;
}

obc_error_code_t framWakeUp(void) {
  obc_error_code_t errCode;
  RETURN_IF_ERROR_CODE(assertChipSelect(FRAM_spiPORT, FRAM_CS));
  BaseType_t schedulerState = xTaskGetSchedulerState();
  if (schedulerState == taskSCHEDULER_NOT_STARTED) {
    for (volatile uint32_t i = 0; i < FRAM_WAKE_BUSY_WAIT; i++) {
      // Do Nothing
    }
  } else {
    vTaskDelay(pdMS_TO_TICKS(FRAM_WAKE_TIME_MS));
  }
  isAsleep = false;
  RETURN_IF_ERROR_CODE(deassertChipSelect(FRAM_spiPORT, FRAM_CS));
  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t framReadID(uint8_t *id, size_t nBytes) {
  obc_error_code_t errCode;
  if (id == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }
  RETURN_IF_ERROR_CODE(spiTakeBusMutex(FRAM_spiREG));
  if (isAsleep) {
    LOG_ERROR_CODE(OBC_ERR_CODE_FRAM_IS_ASLEEP);
    errCode = OBC_ERR_CODE_FRAM_IS_ASLEEP;
  }
  if (errCode == OBC_ERR_CODE_SUCCESS) {
    LOG_IF_ERROR_CODE(assertChipSelect(FRAM_spiPORT, FRAM_CS));
  }
  if (errCode == OBC_ERR_CODE_SUCCESS) {
    LOG_IF_ERROR_CODE(framTransmitOpCode(FRAM_READ_ID));
  }

  for (uint32_t i = 0; i < nBytes && i < FRAM_ID_LEN; i++) {
    if (errCode == OBC_ERR_CODE_SUCCESS) {
      uint8_t receiveByte;
      LOG_IF_ERROR_CODE(spiReceiveByte(FRAM_spiREG, &framSPIDataFmt, &receiveByte));
      id[i] = receiveByte;
    } else {
      break;
    }
  }

  obc_error_code_t prev_errCode = errCode;
  if (errCode != OBC_ERR_CODE_FRAM_IS_ASLEEP) {
    // Do not deassert since never asserted in the first place
    RETURN_IF_ERROR_CODE(deassertChipSelect(FRAM_spiPORT, FRAM_CS));
  }
  // Reset overwritten error code
  errCode = prev_errCode;
  RETURN_IF_ERROR_CODE(spiReleaseBusMutex(FRAM_spiREG));
  // Reset overwritten error code
  errCode = prev_errCode;
  return errCode;
}
