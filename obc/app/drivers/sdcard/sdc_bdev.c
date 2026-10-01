#include <stdint.h>
#include <stdbool.h>

#include <FreeRTOS.h>
#include <os_task.h>

#include <sys_common.h>
#include <gio.h>
#include <spi.h>

#include "sdc_diskio.h"
#include "sdc_rm46.h"
#include "obc_spi_io.h"
#include "obc_logging.h"
#include "obc_assert.h"
#include "obc_board_config.h"

/* This driver logs errors. If the logging output is set to the
   microSD card, this driver will log errors to itself, which
   will cause an infinite loop. We can just disable logging from
   this driver and the SPI helpers if necessary.
*/

/*---------------------------------------------*/
/* SD Card Definitions                         */
/*---------------------------------------------*/

#define SDC_CMD_BASE 0x40U
#define SDC_CMD0 (SDC_CMD_BASE)       /* GO_IDLE_STATE */
#define SDC_CMD1 (SDC_CMD_BASE + 1)   /* SEND_OP_COND */
#define SDC_CMD8 (SDC_CMD_BASE + 8)   /* SEND_IF_COND */
#define SDC_CMD9 (SDC_CMD_BASE + 9)   /* SEND_CSD */
#define SDC_CMD10 (SDC_CMD_BASE + 10) /* SEND_CID */
#define SDC_CMD12 (SDC_CMD_BASE + 12) /* STOP_TRANSMISSION */
#define SDC_CMD16 (SDC_CMD_BASE + 16) /* SET_BLOCKLEN */
#define SDC_CMD17 (SDC_CMD_BASE + 17) /* READ_SINGLE_BLOCK */
#define SDC_CMD18 (SDC_CMD_BASE + 18) /* READ_MULTIPLE_BLOCK */
#define SDC_CMD23 (SDC_CMD_BASE + 23) /* SET_BLOCK_COUNT */
#define SDC_CMD24 (SDC_CMD_BASE + 24) /* WRITE_BLOCK */
#define SDC_CMD25 (SDC_CMD_BASE + 25) /* WRITE_MULTIPLE_BLOCK */
#define SDC_CMD41 (SDC_CMD_BASE + 41) /* SEND_OP_COND (ACMD) */
#define SDC_CMD55 (SDC_CMD_BASE + 55) /* APP_CMD */
#define SDC_CMD58 (SDC_CMD_BASE + 58) /* READ_OCR */

#define SD_SECTOR_SIZE 512U
#define SD_STOP_TRANSMISSION 0xFDU
#define SD_DATA_RESPONSE_MASK 0x1FU
#define SD_DATA_RESPONSE_ACCEPTED 0x05U

#define SDC_CMD17_DATA_TOKEN 0xFEU
#define SDC_CMD18_DATA_TOKEN 0xFEU
#define SDC_CMD24_DATA_TOKEN 0xFEU
#define SDC_CMD25_DATA_TOKEN 0xFCU

// Card type masks (b0:MMC, b1:SDC, b2:Block addressing), see cardType variable
#define CARD_TYPE_MMC_MASK 0b001U
#define CARD_TYPE_SDC_MASK 0b010U
#define CARD_TYPE_BLOCK_ADDR_MASK 0b100U

#define SDC_CMD_RESP_MASK 0x80U
#define CARD_CAPACITY_OCR_MASK (1 << 6)

#define SDC_CMD0_RESET_CRC 0x95U
#define SDC_CMD8_CHECK_VOLTAGE_CRC 0x87U

#define DISK_INIT_RESET_ARG 0x1AAUL

// 10 bytes = 80 clock transitions
#define SDC_CLOCK_TRANSITION_BYTES 10U

#define SDC_ACTION_NUM_ATTEMPTS_DEFAULT 50U
STATIC_ASSERT(SDC_ACTION_NUM_ATTEMPTS_DEFAULT <= 255, "SDC_ACTION_NUM_ATTEMPTS_DEFAULT must be <= 255");

#define SDC_DELAY_1MS pdMS_TO_TICKS(1)

#define SDC_MOSI_HIGH 0xFFU  // Keep MOSI high during read operations

/*---------------------------------------------*/
/* Global Variables                            */
/*---------------------------------------------*/

// SPI configuration for SD card
// CS selected using SPI assert/deassert functions; not using CSNR
static spiDAT1_t sdcSpiConfig = {
    .CS_HOLD = false,
    .WDEL = false,
    .DFSEL = SDC_SPI_DATA_FORMAT,
    .CSNR = SPI_CS_NONE,
};

static volatile DSTATUS stat = STA_NOINIT; /* Disk status */
static uint8_t cardType;                   /* Card type flags: b0:MMC, b1:SDC, b2:Block addressing */
static sdc_power_t powerFlag = POWER_OFF;  /* indicates if "power" is on */

/*---------------------------------------------*/
/* SD Card Private Functions                   */
/*---------------------------------------------*/

/**
 * @brief Check if card is ready
 *
 * @return bool True if card is ready, false otherwise.
 */
static bool isCardReady(void) {
  obc_error_code_t errCode;

  // Assume CS is already asserted

  for (uint8_t i = 0; i < SDC_ACTION_NUM_ATTEMPTS_DEFAULT; i++) {
    uint8_t res;

    LOG_IF_ERROR_CODE(spiTransmitAndReceiveByte(SDC_SPI_REG, &sdcSpiConfig, SDC_MOSI_HIGH, &res));

    if (res == 0xFF) return true;

    vTaskDelay(SDC_DELAY_1MS);
  }

  return false;
}

/**
 * @brief Send >74 clock transitions with CS and DI held high. This is
 * required after card power up to get it into SPI mode.
 */
static void sendClockTrain(void) {
  obc_error_code_t errCode;

  // Assume CS is not asserted

  spiTakeBusMutex(SDC_SPI_REG);
  for (uint8_t i = 0; i < SDC_CLOCK_TRANSITION_BYTES; i++) {
    LOG_IF_ERROR_CODE(spiTransmitByte(SDC_SPI_REG, &sdcSpiConfig, 0xFF));
  }
  spiReleaseBusMutex(SDC_SPI_REG);
}

/**
 * @brief Turn the SD card power on.
 * @warning This function doesn't actually turn the power on, it just
 * sets a flag to indicate that the power is on.
 */
static void turnOnSDC(void) {
  sendClockTrain();
  powerFlag = POWER_ON;
}

/**
 * @brief Turn the SD card power off.
 * @warning This function doesn't actually turn the power off, it just
 * sets a flag to indicate that the power is off.
 */
static void turnOffSDC(void) { powerFlag = POWER_OFF; }

/**
 * @brief Check the power status of the SD card.
 *
 * @return sdc_power_t POWER_ON if the SD card is powered on, POWER_OFF otherwise.
 */
static sdc_power_t checkPower(void) { return powerFlag; }

/**
 * @brief Receive a data packet from the SD card.
 * @param buff Buffer to store the received data.
 * @param btr Number of bytes to receive (Must be an even number).
 * @return bool True if the packet was received successfully, false otherwise.
 */
static bool rcvDataBlock(uint8_t *buff, uint32_t btr) {
  obc_error_code_t errCode;

  // Assume CS is already asserted

  if (btr % 2 != 0)  // Must be an even number
    return false;

  uint8_t token = 0xFF;

  /* Wait for a data packet */
  for (uint8_t i = 0; i < SDC_ACTION_NUM_ATTEMPTS_DEFAULT; i++) {
    LOG_IF_ERROR_CODE(spiTransmitAndReceiveByte(SDC_SPI_REG, &sdcSpiConfig, SDC_MOSI_HIGH, &token));
    if (token != 0xFF) break;
    vTaskDelay(SDC_DELAY_1MS);
  }

  /* If not valid data token, return with error */
  if (token != SDC_CMD17_DATA_TOKEN) return false;

  /* Receive the data block into buffer */
  while (btr) {
    LOG_IF_ERROR_CODE(spiTransmitAndReceiveByte(SDC_SPI_REG, &sdcSpiConfig, SDC_MOSI_HIGH, buff++));
    LOG_IF_ERROR_CODE(spiTransmitAndReceiveByte(SDC_SPI_REG, &sdcSpiConfig, SDC_MOSI_HIGH, buff++));
    btr -= 2;
  }

  /* Discard CRC */
  unsigned char crc;
  LOG_IF_ERROR_CODE(spiTransmitAndReceiveByte(SDC_SPI_REG, &sdcSpiConfig, SDC_MOSI_HIGH, &crc));
  LOG_IF_ERROR_CODE(spiTransmitAndReceiveByte(SDC_SPI_REG, &sdcSpiConfig, SDC_MOSI_HIGH, &crc));

  return true;
}

#if _READONLY == 0

/**
 * @brief Send a data packet to the SD card.
 * @param buff 512 uint8_t buffer containing the data to send.
 * @param token Data/Stop token.
 * @return bool True if the packet was sent successfully, false otherwise.
 */
static bool sendDataBlock(const uint8_t *buff, uint8_t token) {
  obc_error_code_t errCode;

  // Assume CS is already asserted

  if (!isCardReady()) return false;

  LOG_IF_ERROR_CODE(spiTransmitByte(SDC_SPI_REG, &sdcSpiConfig, token));  // Send token

  if (token != SD_STOP_TRANSMISSION) {
    for (unsigned int wc = 0; wc < SD_SECTOR_SIZE; wc++) {
      // Send the data block
      LOG_IF_ERROR_CODE(spiTransmitByte(SDC_SPI_REG, &sdcSpiConfig, *buff++));
    }

    // Send dummy CRC
    LOG_IF_ERROR_CODE(spiTransmitByte(SDC_SPI_REG, &sdcSpiConfig, 0xFF));
    LOG_IF_ERROR_CODE(spiTransmitByte(SDC_SPI_REG, &sdcSpiConfig, 0xFF));

    uint8_t resp;
    LOG_IF_ERROR_CODE(
        spiTransmitAndReceiveByte(SDC_SPI_REG, &sdcSpiConfig, SDC_MOSI_HIGH, &resp)); /* Receive data response */
    if ((resp & SD_DATA_RESPONSE_MASK) != SD_DATA_RESPONSE_ACCEPTED) return false;
  }

  return true;
}

#endif /* _READONLY */

/**
 * @brief Send a command packet to the SD card.
 * @param cmd Command uint8_t.
 * @param arg Argument.
 * @return uint8_t Response uint8_t.
 */
static uint8_t sendCMD(uint8_t cmd, uint32_t arg) {
  obc_error_code_t errCode;
  // Assume CS is already asserted

  if (!isCardReady()) return 0xFFU;

  /* Send command packet */
  LOG_IF_ERROR_CODE(spiTransmitByte(SDC_SPI_REG, &sdcSpiConfig, cmd));                  /* Command */
  LOG_IF_ERROR_CODE(spiTransmitByte(SDC_SPI_REG, &sdcSpiConfig, (uint8_t)(arg >> 24))); /* Argument[31..24] */
  LOG_IF_ERROR_CODE(spiTransmitByte(SDC_SPI_REG, &sdcSpiConfig, (uint8_t)(arg >> 16))); /* Argument[23..16] */
  LOG_IF_ERROR_CODE(spiTransmitByte(SDC_SPI_REG, &sdcSpiConfig, (uint8_t)(arg >> 8)));  /* Argument[15..8] */
  LOG_IF_ERROR_CODE(spiTransmitByte(SDC_SPI_REG, &sdcSpiConfig, (uint8_t)arg));         /* Argument[7..0] */

  /* Some commands require a CRC to be sent */
  uint8_t crc = 0xFFU;
  if (cmd == SDC_CMD0) {
    crc = SDC_CMD0_RESET_CRC;
  } else if (cmd == SDC_CMD8) {
    crc = SDC_CMD8_CHECK_VOLTAGE_CRC;
  }

  LOG_IF_ERROR_CODE(spiTransmitByte(SDC_SPI_REG, &sdcSpiConfig, crc));

  /* Skip a uint8_t after "stop reading" cmd is sent */
  unsigned char tmp;
  if (cmd == SDC_CMD12) {
    LOG_IF_ERROR_CODE(spiTransmitAndReceiveByte(SDC_SPI_REG, &sdcSpiConfig, SDC_MOSI_HIGH, &tmp));
  }

  /* Receive command response */
  uint8_t res;
  for (uint8_t i = 0; i < SDC_ACTION_NUM_ATTEMPTS_DEFAULT; i++) {
    LOG_IF_ERROR_CODE(spiTransmitAndReceiveByte(SDC_SPI_REG, &sdcSpiConfig, SDC_MOSI_HIGH, &res));
    if (!(res & SDC_CMD_RESP_MASK)) break;
    vTaskDelay(SDC_DELAY_1MS);
  }

  return res;
}

/**
 * @brief Send SDC_CMD12 to the SD card to stop a multi-block read.
 *
 * @return uint8_t Response uint8_t.
 */
static uint8_t stopTransmission(void) {
  obc_error_code_t errCode;

  // Assume CS is already asserted

  /* Send command packet - the argument for SDC_CMD12 is ignored. */
  LOG_IF_ERROR_CODE(spiTransmitByte(SDC_SPI_REG, &sdcSpiConfig, SDC_CMD12));
  LOG_IF_ERROR_CODE(spiTransmitByte(SDC_SPI_REG, &sdcSpiConfig, 0));
  LOG_IF_ERROR_CODE(spiTransmitByte(SDC_SPI_REG, &sdcSpiConfig, 0));
  LOG_IF_ERROR_CODE(spiTransmitByte(SDC_SPI_REG, &sdcSpiConfig, 0));
  LOG_IF_ERROR_CODE(spiTransmitByte(SDC_SPI_REG, &sdcSpiConfig, 0));
  LOG_IF_ERROR_CODE(spiTransmitByte(SDC_SPI_REG, &sdcSpiConfig, 0));

  /* Data transfer stops 2 bytes after 6-uint8_t SDC_CMD12 */
  uint8_t val;
  for (uint8_t i = 0; i < 2; i++) {
    LOG_IF_ERROR_CODE(spiTransmitAndReceiveByte(SDC_SPI_REG, &sdcSpiConfig, SDC_MOSI_HIGH, &val));
  }

  /* SDC should now send 2-6 0xFF bytes, the response uint8_t, and then another 0xFF */
  /* Some cards don't send the 2-6 0xFF bytes */
  uint8_t res;
  const uint8_t numBytesRcv = 8U;
  for (unsigned int n = 0; n < numBytesRcv; n++) {
    LOG_IF_ERROR_CODE(spiTransmitAndReceiveByte(SDC_SPI_REG, &sdcSpiConfig, SDC_MOSI_HIGH, &val));
    if (val != 0xFF) res = val;
  }

  return res;
}

/*---------------------------------------------------------------------------*/
/* Public Functions                                                          */
/*---------------------------------------------------------------------------*/

/**
 * @brief Initialize the SD card.
 * @param pdrv Physical drive number (0).
 * @return DSTATUS Status
 */
DSTATUS disk_initialize(uint8_t drv) {
  obc_error_code_t errCode;

  // Re-initialize static variables
  stat = STA_NOINIT;
  cardType = 0;
  powerFlag = POWER_OFF;

  if (drv) return STA_NOINIT;         /* Supports only single drive */
  if (stat & STA_NODISK) return stat; /* No card in the socket */

  turnOnSDC();

  // Recursive take done so we can send byte when CS is high at the end of transaction
  LOG_IF_ERROR_CODE(spiTakeBusMutex(SDC_SPI_REG));
  if (errCode != OBC_ERR_CODE_SUCCESS) {
    return stat;
  }

  LOG_IF_ERROR_CODE(assertChipSelect(SDC_SPI_PORT, SDC_SPI_CS));
  if (errCode != OBC_ERR_CODE_SUCCESS) {
    return stat;
  }

  uint8_t ty = 0;

  for (uint8_t i = 0; i < SDC_ACTION_NUM_ATTEMPTS_DEFAULT; i++) {
    // Reset the card and put it into SPI mode; response should be 0x01 if successful
    if (sendCMD(SDC_CMD0, 0) == 1U) break;
    vTaskDelay(SDC_DELAY_1MS);
  }
  // Even if SDC_CMD0 fails, we'll try to continue.

  // Send CMD8 with 0x1AA arg. CRC handled in sendCMD.
  if (sendCMD(SDC_CMD8, DISK_INIT_RESET_ARG) == 1U) {
    // Card is SDC Ver2+
    const uint8_t ocrSize = 4U;
    uint8_t ocr[ocrSize];

    for (uint8_t i = 0; i < ocrSize; i++) {
      LOG_IF_ERROR_CODE(spiTransmitAndReceiveByte(SDC_SPI_REG, &sdcSpiConfig, SDC_MOSI_HIGH, &ocr[i]));
    }

    // Check if the lower 12 bits in the response are 0x1AA
    if (ocr[2] == 0x01 && ocr[3] == 0xAA) {
      // The card can work at vdd range of 2.7-3.6V
      for (uint8_t i = 0; i < SDC_ACTION_NUM_ATTEMPTS_DEFAULT; i++) {
        // Begin initialization process with ACMD41 with HCS[bit30] set as argument
        // CMD55 must be sent before ACMD41
        if (sendCMD(SDC_CMD55, 0) <= 1 && sendCMD(SDC_CMD41, 1UL << 30) == 0) {
          // Read ocr with CMD58
          if (sendCMD(SDC_CMD58, 0) == 0) {
            // Check bit 6 of response to determine if card is SDHC or standard SD card
            for (unsigned int i = 0; i < ocrSize; i++) {
              LOG_IF_ERROR_CODE(spiTransmitAndReceiveByte(SDC_SPI_REG, &sdcSpiConfig, SDC_MOSI_HIGH, &ocr[i]));
            }
            ty = (ocr[0] & CARD_CAPACITY_OCR_MASK) ? (CARD_TYPE_SDC_MASK | CARD_TYPE_BLOCK_ADDR_MASK)
                                                   : (CARD_TYPE_SDC_MASK);
          }
        }
        vTaskDelay(SDC_DELAY_1MS);
      }
    } else {
      LOG_ERROR_CODE(OBC_ERR_CODE_SD_CARD_INVALID_VOLTAGE);
    }
  } else {
    // Card is SDC Ver1 or MMC
    ty = (sendCMD(SDC_CMD55, 0) <= 1 && sendCMD(SDC_CMD41, 0) <= 1) ? (CARD_TYPE_SDC_MASK) : (CARD_TYPE_MMC_MASK);

    bool initSuccess = false;
    for (uint8_t i = 0; i < SDC_ACTION_NUM_ATTEMPTS_DEFAULT; i++) {
      // Begin the initialization process with ACMD41 for SDC or CMD1 for MMC
      // CMD55 must be sent before ACMD41
      if (ty & CARD_TYPE_SDC_MASK) {
        if (sendCMD(SDC_CMD55, 0) <= 1 && sendCMD(SDC_CMD41, 0) == 0) initSuccess = true;
      } else {
        if (sendCMD(SDC_CMD1, 0) == 0) initSuccess = true;
      }

      if (initSuccess) break;
      vTaskDelay(SDC_DELAY_1MS);
    }

    if (!initSuccess) {
      LOG_ERROR_CODE(OBC_ERR_CODE_SD_CARD_INIT_FAILED);
      ty = 0;
    }

    // Set block length to 512 bytes
    if (sendCMD(SDC_CMD16, SD_SECTOR_SIZE) != 0) ty = 0;
  }

  cardType = ty;

  LOG_IF_ERROR_CODE(deassertChipSelect(SDC_SPI_PORT, SDC_SPI_CS));
  if (errCode == OBC_ERR_CODE_SUCCESS) {
    // Ensure SDC releases MISO line by sending a dummy uint8_t
    LOG_IF_ERROR_CODE(spiTransmitByte(SDC_SPI_REG, &sdcSpiConfig, 0xFF));
  }
  LOG_IF_ERROR_CODE(spiReleaseBusMutex(SDC_SPI_REG));

  if (ty) {
    stat &= ~STA_NOINIT;  // Clear STA_NOINIT
  } else {
    turnOffSDC();  // Initialization failed
  }

  return stat;
}

/**
 * @brief Get disk status.
 * @param pdrv Physical drive number (0).
 * @return DSTATUS Status
 */
DSTATUS disk_status(uint8_t pdrv) {
  if (pdrv != 0) return STA_NOINIT; /* Supports only single drive */
  return stat;
}

/**
 * @brief Read sector(s) from the SD card.
 * @param pdrv Physical drive number (0).
 * @param buff Pointer to the data buffer to store read data.
 * @param sector Start sector number (LBA).
 * @param count Sector count (1..255).
 * @return DRESULT Result
 */
DRESULT disk_read(uint8_t pdrv, uint8_t *buff, uint32_t sector, uint32_t count) {
  obc_error_code_t errCode;

  if (pdrv || !count) return RES_PARERR;
  if (stat & STA_NOINIT) return RES_NOTRDY;

  if (!(cardType & CARD_TYPE_BLOCK_ADDR_MASK)) sector *= SD_SECTOR_SIZE; /* Convert to uint8_t address if needed */

  // Recursive take done so we can send byte when CS is high at the end of transaction
  LOG_IF_ERROR_CODE(spiTakeBusMutex(SDC_SPI_REG));
  if (errCode != OBC_ERR_CODE_SUCCESS) {
    return stat;
  }

  LOG_IF_ERROR_CODE(assertChipSelect(SDC_SPI_PORT, SDC_SPI_CS));
  if (errCode != OBC_ERR_CODE_SUCCESS) {
    return stat;
  }

  if (count == 1) {
    /* Single block read */
    if ((sendCMD(SDC_CMD17, sector) == 0) && rcvDataBlock(buff, SD_SECTOR_SIZE)) count = 0;
  } else {
    /* Multiple block read */
    if (sendCMD(SDC_CMD18, sector) == 0) {
      do {
        if (!rcvDataBlock(buff, SD_SECTOR_SIZE)) break;
        buff += SD_SECTOR_SIZE;
      } while (--count);
      stopTransmission();
    }
  }

  LOG_IF_ERROR_CODE(deassertChipSelect(SDC_SPI_PORT, SDC_SPI_CS));
  if (errCode == OBC_ERR_CODE_SUCCESS) {
    // Ensure SDC releases MISO line by sending a dummy uint8_t
    LOG_IF_ERROR_CODE(spiTransmitByte(SDC_SPI_REG, &sdcSpiConfig, 0xFF));
  }
  LOG_IF_ERROR_CODE(spiReleaseBusMutex(SDC_SPI_REG));

  return count ? RES_ERROR : RES_OK;
}

#if _READONLY == 0
/**
 * @brief Write sector(s) to the SD card.
 * @param pdrv Physical drive number (0).
 * @param buff Pointer to the data to be written.
 * @param sector Start sector number (LBA).
 * @param count Sector count (1..255).
 * @return DRESULT Result
 */
DRESULT disk_write(uint8_t pdrv, const uint8_t *buff, uint32_t sector, uint32_t count) {
  obc_error_code_t errCode;

  if (pdrv || !count) return RES_PARERR;
  if (stat & STA_NOINIT) return RES_NOTRDY;
  if (stat & STA_PROTECT) return RES_WRPRT;

  if (!(cardType & (CARD_TYPE_BLOCK_ADDR_MASK))) sector *= SD_SECTOR_SIZE; /* Convert to uint8_t address if needed */

  // Recursive take done so we can send byte when CS is high at the end of transaction
  LOG_IF_ERROR_CODE(spiTakeBusMutex(SDC_SPI_REG));
  if (errCode != OBC_ERR_CODE_SUCCESS) {
    return stat;
  }

  LOG_IF_ERROR_CODE(assertChipSelect(SDC_SPI_PORT, SDC_SPI_CS));
  if (errCode != OBC_ERR_CODE_SUCCESS) {
    return stat;
  }

  if (count == 1) {
    /* Single block write */
    if ((sendCMD(SDC_CMD24, sector) == 0) && sendDataBlock(buff, SDC_CMD24_DATA_TOKEN)) count = 0;
  } else {
    /* Multiple block write */
    if (cardType & CARD_TYPE_SDC_MASK) {
      sendCMD(SDC_CMD55, 0);
      sendCMD(SDC_CMD23, count);
    }

    if (sendCMD(SDC_CMD25, sector) == 0) {
      do {
        if (!sendDataBlock(buff, SDC_CMD25_DATA_TOKEN)) break;
        buff += SD_SECTOR_SIZE;
      } while (--count);

      // Send stop transmission token
      if (!sendDataBlock(0, SD_STOP_TRANSMISSION)) count = 1;
    }
  }

  LOG_IF_ERROR_CODE(deassertChipSelect(SDC_SPI_PORT, SDC_SPI_CS));
  if (errCode == OBC_ERR_CODE_SUCCESS) {
    // Ensure SDC releases MISO line by sending a dummy uint8_t
    LOG_IF_ERROR_CODE(spiTransmitByte(SDC_SPI_REG, &sdcSpiConfig, 0xFF));
  }
  LOG_IF_ERROR_CODE(spiReleaseBusMutex(SDC_SPI_REG));

  return count ? RES_ERROR : RES_OK;
}
#endif /* _READONLY */

/**
 * @brief Disk I/O control.
 * @param pdrv Physical drive number (0).
 * @param cmd Control command code.
 * @param buff Pointer to the control data.
 * @return DRESULT Result
 * @warning buff must have a size of at least 2 bytes.
 */
DRESULT disk_ioctl(uint8_t pdrv, uint8_t ctrl, void *buff) {
  obc_error_code_t errCode;

  if (pdrv) return RES_PARERR;

  DRESULT res = RES_ERROR;

  if (ctrl == CTRL_POWER) {
    uint8_t *ptr = buff;
    switch (*ptr) {
      case 0:
        /* Sub control code == 0 (turn off power) */
        if (checkPower() == POWER_ON) turnOffSDC();
        res = RES_OK;
        break;
      case 1:
        /* Sub control code == 1 (turn on power) */
        turnOnSDC();
        res = RES_OK;
        break;
      case 2:
        /* Sub control code == 2 (get power status) */
        *(ptr + 1) = (uint8_t)checkPower();
        res = RES_OK;
        break;
      default:
        res = RES_PARERR;
    }
  } else {
    if (stat & STA_NOINIT) return RES_NOTRDY;

    // Recursive take done so we can send byte when CS is high at the end of transaction
    LOG_IF_ERROR_CODE(spiTakeBusMutex(SDC_SPI_REG));
    if (errCode != OBC_ERR_CODE_SUCCESS) {
      return stat;
    }

    LOG_IF_ERROR_CODE(assertChipSelect(SDC_SPI_PORT, SDC_SPI_CS));
    if (errCode != OBC_ERR_CODE_SUCCESS) {
      return stat;
    }

    const uint8_t csdSize = 16U;  // Size of buffer to hold CSD register data
    uint8_t csd[csdSize];  // Card-specific data (CSD); Note the index numbers are opposite to the bit numbers in the
                           // CSD register
    uint32_t csize;        // Device size
    switch (ctrl) {
      case GET_SECTOR_COUNT:
        /* Get number of sectors on the disk (uint32_t) */
        // Read from CSD register
        if ((sendCMD(SDC_CMD9, 0) == 0) && rcvDataBlock(csd, csdSize)) {
          // Check if SDCv2+ or SDCv1/MMC
          uint8_t csdStructField = (csd[0] >> 6);  // 1 = SDCv2, 0 = SDCv1/MMC
          if (csdStructField == 1) {
            // CSIZE field spans bits [69:48] of the CSD register
            // Disk size (bytes) = (CSIZE+1) * 512KB * 1024B/KB
            csize = ((uint32_t)(csd[7] & 0x3F) << 16) | ((uint16_t)csd[8] << 8) | csd[9];
            *(uint32_t *)buff = (uint32_t)(csize + 1) << 10;
          } else {
            // CSIZE field spans bits [73:62] of the CSD register
            // Capacity = (CSIZE+1) * 2^(C_SIZE_MULT+READ_BL_LEN+2)
            csize = ((uint32_t)(csd[6] & 0x03) << 10) | ((uint16_t)csd[7] << 2) | (csd[8] >> 6);
            const uint8_t readBlLen = csd[5] & 0xF;  // READ_BL_LEN field spans bits [83:80] of the CSD register
            const uint8_t cSizeMult =
                ((csd[9] & 0x3) << 1) | (csd[10] >> 7);  // C_SIZE_MULT field spans bits [49:47] of the CSD register
            const uint8_t n = (cSizeMult + readBlLen + 2);
            *(uint32_t *)buff = (uint32_t)(csize + 1) << (n - 9);
          }
          res = RES_OK;
        }
        break;
      case GET_SECTOR_SIZE:
        *(uint16_t *)buff = SD_SECTOR_SIZE;
        res = RES_OK;
        break;
      case CTRL_SYNC:
        /* Make sure that data has been written */
        if (isCardReady()) res = RES_OK;
        break;
      default:
        res = RES_PARERR;
    }

    LOG_IF_ERROR_CODE(deassertChipSelect(SDC_SPI_PORT, SDC_SPI_CS));
    if (errCode == OBC_ERR_CODE_SUCCESS) {
      // Ensure SDC releases MISO line by sending a dummy uint8_t
      LOG_IF_ERROR_CODE(spiTransmitByte(SDC_SPI_REG, &sdcSpiConfig, 0xFF));
    }
    LOG_IF_ERROR_CODE(spiReleaseBusMutex(SDC_SPI_REG));
  }

  return res;
}
