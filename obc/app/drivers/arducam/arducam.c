#include "arducam.h"

#include "ov5642_config.h"

#include "obc_spi_io.h"
#include "obc_board_config.h"
#include "spi.h"

// Constants
#define ARDUCAM_DELAY_2MS pdMS_TO_TICKS(2)
#define MAX_FIFO_SIZE 0x7FFFFF  // 8MByte

// Sensor Timing Control Masks
#define HSYNC_ACTIVE_LOW_MASK (0x01U << 0)
#define VSYNC_ACTIVE_LOW_MASK (0x01U << 1)
#define PCLK_REVERSED_MASK (0x01U << 3)
// FIFO Control Masks
#define FIFO_CLEAR_CAPTURE_DONE_FLAG (0x01U << 0)
#define FIFO_START_CAPTURE (0x01U << 1)
#define FIFO_RESET_WRITE_PTR (0x01U << 4)
#define FIFO_RESET_READ_PTR (0x01U << 5)
// Sensor Control Masks
#define SENSOR_RESET_MASK (0x01U << 0)
#define SENSOR_STANDBY_MASK (0x01 << 1)
#define SENSOR_POWER_EN_MASK (0x01U << 2)
// Capture Status Masks
#define STATUS_VSYNC_MASK (0x01U << 0)
#define STATUS_CAPTURE_DONE_MASK (0x01 << 3)

// Arduchip masks
#define ARDUCAM_RESET_CPLD_MASK 0x80

// SPI values
static spiDAT1_t arducamSPIDataFmt = {.CS_HOLD = 0, .CSNR = SPI_CS_NONE, .DFSEL = CAM_SPI_DATA_FORMAT, .WDEL = 0};

static const uint8_t cameraCS[CAMERA_COUNT] = {
    [PRIMARY] = CAM_CS_1,
    [SECONDARY] = CAM_CS_2,
};

static uint32_t totalBytesToRead[CAMERA_COUNT] = {0};
static uint32_t FIFOReadPtr[CAMERA_COUNT] = {0};

typedef enum opcode {
  ARDUCAM_TEST_REG = 0x00,
  ARDUCAM_CAPTURE_CONTROL_REG = 0x01,
  ARDUCAM_SENSOR_TIMING_CONTROL_REG = 0x03,
  ARDUCAM_FIFO_CONTROL_REG = 0x04,
  ARDUCAM_SENSOR_POWER_CONTROL_REG = 0x06,
  ARDUCAM_FIFO_BURST_READ = 0x3C,
  ARDUCAM_FIFO_READ = 0x3D,
  ARDUCAM_FW_VERSION = 0x40,
  ARDUCAM_CAPTURE_STATUS_REG = 0x41,
  ARDUCAM_FIFO_SIZE_LOWER = 0x42,
  ARDUCAM_FIFO_SIZE_MIDDLE = 0x43,
  ARDUCAM_FIFO_SIZE_UPPER = 0x44,
  ARDUCAM_RESET_CPLD = 0x07,
} arducam_opcode_t;

/**
 * @brief Resets arducam chip. Needed for some ARM architecture for SPI to work. IDK why this is not in the docs >:(
 *        https://docs.arducam.com/Arduino-SPI-camera/Legacy-SPI-camera/FAQ/
 *
 * @param cameraID Camera ID of camera
 * @return Error code. OBC_ERR_CODE_SUCCESS if successful.
 */
static obc_error_code_t arducamResetCPLD(camera_id_t cameraID);

/**
 * @brief Write a value to Arducam's Sensor Power Control Register
 *        Bit[0]: Reset Sensor: 0 = reset sensor, 1 = out of reset;
 *        Bit[1]: Standby Sensor 0 = out of standby, 1 = in standby;
 *        Bit[2]: Power Down Sensor 0 = disable power, 1 = enable power;
 *        Note: After Power Down, Sensor will need to be reinitialized
 *
 * @param cameraID Camera ID of camera
 * @param value 1 byte value to be written.
 * @return Error code. OBC_ERR_CODE_SUCCESS if successful.
 */
static obc_error_code_t arducamWriteSensorPowerControlReg(camera_id_t cameraID, uint8_t value);

/**
 * @brief Read bufferSize bytes from the Arducam FIFO
 *
 * @param cameraID Camera ID of camera
 * @param buffer bufferSize byte buffer to store value read.
 * @param bufferSize size of buffer.
 * @return Error code. OBC_ERR_CODE_SUCCESS if successful.
 */
static obc_error_code_t arducamBurstReadFIFO(camera_id_t cameraID, uint8_t *buffer, size_t bufferSize);

/**
 * @brief Read Arducam's capture status.
 *        Bit[0] : vsync pin realtime status;
 *        Bit[3] : capture done flag
 * @param cameraID Camera ID of camera
 * @param version 1 byte buffer to store value read.
 * @return Error code. OBC_ERR_CODE_SUCCESS if successful.
 */
static obc_error_code_t arducamReadCaptureStatusReg(camera_id_t cameraID, uint8_t *status);

/**
 * @brief Returns number of Bytes left in FIFO to read
 *
 * @param cameraID Camera ID of camera
 * @return number of bytes
 */
static size_t bytesLeftInFIFO(camera_id_t cameraID);

// CS assumed to be asserted
static obc_error_code_t arducamTransmitOpcode(arducam_opcode_t opcode) {
  obc_error_code_t errCode;
  LOG_IF_ERROR_CODE(spiTransmitByte(CAM_SPI_REG, &arducamSPIDataFmt, opcode));
  return errCode;
}

static obc_error_code_t arducamWriteRegister(camera_id_t cameraID, arducam_opcode_t opcode, uint8_t value) {
  obc_error_code_t errCode;
  obc_error_code_t prevCode;

  RETURN_IF_ERROR_CODE(assertChipSelect(CAM_SPI_PORT, cameraCS[cameraID]));
  LOG_IF_ERROR_CODE(arducamTransmitOpcode(opcode | 0x80));  // first bit is R/W
  if (errCode == OBC_ERR_CODE_SUCCESS) {
    LOG_IF_ERROR_CODE(spiTransmitByte(CAM_SPI_REG, &arducamSPIDataFmt, value));
  }
  prevCode = errCode;
  RETURN_IF_ERROR_CODE(deassertChipSelect(CAM_SPI_PORT, cameraCS[cameraID]));
  // Reset overwritten error code
  errCode = prevCode;

  return errCode;
}

static obc_error_code_t arducamReadRegister(camera_id_t cameraID, arducam_opcode_t opcode, uint8_t *buffer) {
  obc_error_code_t errCode;
  obc_error_code_t prevCode;
  if (buffer == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  RETURN_IF_ERROR_CODE(assertChipSelect(CAM_SPI_PORT, cameraCS[cameraID]));
  LOG_IF_ERROR_CODE(arducamTransmitOpcode(opcode & 0x7F));  // first bit is R/W
  if (errCode == OBC_ERR_CODE_SUCCESS) {
    LOG_IF_ERROR_CODE(spiReceiveByte(CAM_SPI_REG, &arducamSPIDataFmt, buffer));
  }
  prevCode = errCode;
  RETURN_IF_ERROR_CODE(deassertChipSelect(CAM_SPI_PORT, cameraCS[cameraID]));
  // Reset overwritten error code
  errCode = prevCode;

  return errCode;
}

obc_error_code_t arducamReadTestReg(camera_id_t cameraID, uint8_t *buffer) {
  return arducamReadRegister(cameraID, ARDUCAM_TEST_REG, buffer);
}

obc_error_code_t arducamWriteTestReg(camera_id_t cameraID, uint8_t value) {
  return arducamWriteRegister(cameraID, ARDUCAM_TEST_REG, value);
}

obc_error_code_t arducamReadSensorPowerControlReg(camera_id_t cameraID, uint8_t *buffer) {
  return arducamReadRegister(cameraID, ARDUCAM_SENSOR_POWER_CONTROL_REG, buffer);
}

static obc_error_code_t arducamWriteSensorPowerControlReg(camera_id_t cameraID, uint8_t value) {
  return arducamWriteRegister(cameraID, ARDUCAM_SENSOR_POWER_CONTROL_REG, value);
}

static obc_error_code_t arducamBurstReadFIFO(camera_id_t cameraID, uint8_t *buffer, size_t bufferSize) {
  obc_error_code_t errCode;
  obc_error_code_t prevCode;

  if (buffer == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  RETURN_IF_ERROR_CODE(assertChipSelect(CAM_SPI_PORT, cameraCS[cameraID]));
  LOG_IF_ERROR_CODE(arducamTransmitOpcode(ARDUCAM_FIFO_BURST_READ));
  for (size_t index = 0; (index < bufferSize) && (errCode == OBC_ERR_CODE_SUCCESS); index++) {
    LOG_IF_ERROR_CODE(spiReceiveByte(CAM_SPI_REG, &arducamSPIDataFmt, &buffer[index]));
  }
  prevCode = errCode;
  RETURN_IF_ERROR_CODE(deassertChipSelect(CAM_SPI_PORT, cameraCS[cameraID]));
  // Reset overwritten error code
  errCode = prevCode;

  return errCode;
}

static obc_error_code_t arducamReadCaptureStatusReg(camera_id_t cameraID, uint8_t *status) {
  return arducamReadRegister(cameraID, ARDUCAM_CAPTURE_STATUS_REG, status);
}

static obc_error_code_t arducamResetCPLD(camera_id_t cameraID) {
  obc_error_code_t errCode;
  RETURN_IF_ERROR_CODE(arducamWriteRegister(cameraID, ARDUCAM_RESET_CPLD, ARDUCAM_RESET_CPLD_MASK));
  vTaskDelay(ARDUCAM_DELAY_2MS);
  RETURN_IF_ERROR_CODE(arducamWriteRegister(cameraID, ARDUCAM_RESET_CPLD, 0));
  return errCode;
}

obc_error_code_t arducamReadFWVersion(camera_id_t cameraID, uint8_t *version) {
  return arducamReadRegister(cameraID, ARDUCAM_FW_VERSION, version);
}

obc_error_code_t arducamReadFIFOSize(camera_id_t cameraID, uint32_t *fifoSize) {
  obc_error_code_t errCode;
  uint8_t upper = 0;
  uint8_t middle = 0;
  uint8_t lower = 0;
  if (fifoSize == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  *fifoSize = 0;

  // Get upper bits
  RETURN_IF_ERROR_CODE(arducamReadRegister(cameraID, ARDUCAM_FIFO_SIZE_UPPER, &upper));
  // Get middle bits
  RETURN_IF_ERROR_CODE(arducamReadRegister(cameraID, ARDUCAM_FIFO_SIZE_MIDDLE, &middle));
  // Get lower bits
  RETURN_IF_ERROR_CODE(arducamReadRegister(cameraID, ARDUCAM_FIFO_SIZE_LOWER, &lower));

  *fifoSize = upper;
  *fifoSize = (*fifoSize << 8) | middle;
  *fifoSize = (*fifoSize << 8) | lower;

  return errCode;
}

obc_error_code_t initCamera(camera_id_t cameraID) {
  obc_error_code_t errCode;
  totalBytesToRead[cameraID] = 0;
  FIFOReadPtr[cameraID] = 0;
  RETURN_IF_ERROR_CODE(arducamResetCPLD(cameraID));
  // Make sure sensor is powered and reset is not asserted (reset is active low).
  RETURN_IF_ERROR_CODE(arducamWriteSensorPowerControlReg(cameraID, SENSOR_POWER_EN_MASK | SENSOR_RESET_MASK));
  // Make sure sensor vsync timing is set to active low
  RETURN_IF_ERROR_CODE(arducamWriteRegister(cameraID, ARDUCAM_SENSOR_TIMING_CONTROL_REG, VSYNC_ACTIVE_LOW_MASK));
  return errCode;
}

obc_error_code_t startImageCapture(camera_id_t cameraID) {
  obc_error_code_t errCode;
  // Make sure output is jpeg and set resolution is 320x240
  RETURN_IF_ERROR_CODE(applyCamCaptureConfig());
  RETURN_IF_ERROR_CODE(applyCamResolutionConfig());

  // Start capture
  RETURN_IF_ERROR_CODE(
      arducamWriteRegister(cameraID, ARDUCAM_FIFO_CONTROL_REG, FIFO_CLEAR_CAPTURE_DONE_FLAG | FIFO_START_CAPTURE));
  totalBytesToRead[cameraID] = 0;
  FIFOReadPtr[cameraID] = 0;
  return errCode;
}

obc_error_code_t isCaptureDone(camera_id_t cameraID) {
  obc_error_code_t errCode;
  uint8_t status;
  RETURN_IF_ERROR_CODE(arducamReadCaptureStatusReg(cameraID, &status));
  return ((bool)(status & STATUS_CAPTURE_DONE_MASK)) ? OBC_ERR_CODE_CAMERA_CAPTURE_COMPLETE
                                                     : OBC_ERR_CODE_CAMERA_CAPTURE_INCOMPLETE;
}

obc_error_code_t readImage(camera_id_t cameraID, uint8_t *buffer, size_t bufferLen, size_t *bytesRead) {
  if (buffer == NULL || bytesRead == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }
  obc_error_code_t errCode;
  // Start of a new image, get image length
  if (totalBytesToRead[cameraID] == 0) {
    RETURN_IF_ERROR_CODE(arducamReadFIFOSize(cameraID, &totalBytesToRead[cameraID]));
  }
  size_t bytesToRead = bytesLeftInFIFO(cameraID);
  if (bytesToRead > bufferLen) {
    bytesToRead = bufferLen;
  }

  RETURN_IF_ERROR_CODE(arducamBurstReadFIFO(cameraID, buffer, bytesToRead));
  *bytesRead = bytesToRead;
  FIFOReadPtr[cameraID] += bytesToRead;
  if (bytesLeftInFIFO(cameraID) > 0) {
    errCode = OBC_ERR_CODE_CAMERA_IMAGE_READ_INCOMPLETE;
  }
  return errCode;
}

static size_t bytesLeftInFIFO(camera_id_t cameraID) {
  return (totalBytesToRead[cameraID] < FIFOReadPtr[cameraID]) ? 0 : totalBytesToRead[cameraID] - FIFOReadPtr[cameraID];
}

obc_error_code_t wakeupCamera(camera_id_t cameraID) {
  obc_error_code_t errCode;
  uint8_t registerVal = 0;
  RETURN_IF_ERROR_CODE(arducamReadSensorPowerControlReg(cameraID, &registerVal));
  // Unset standby bit (active high)
  registerVal &= (~SENSOR_STANDBY_MASK);
  RETURN_IF_ERROR_CODE(arducamWriteSensorPowerControlReg(cameraID, registerVal));
  return errCode;
}

obc_error_code_t standbyCamera(camera_id_t cameraID) {
  obc_error_code_t errCode;
  uint8_t registerVal = 0;
  RETURN_IF_ERROR_CODE(arducamReadSensorPowerControlReg(cameraID, &registerVal));
  // Set standby bit (active high)
  registerVal |= (SENSOR_STANDBY_MASK);
  RETURN_IF_ERROR_CODE(arducamWriteSensorPowerControlReg(cameraID, registerVal));
  return errCode;
}

obc_error_code_t powerOnCamera(camera_id_t cameraID) {
  obc_error_code_t errCode;
  uint8_t registerVal = 0;
  RETURN_IF_ERROR_CODE(arducamReadSensorPowerControlReg(cameraID, &registerVal));
  // Set power en bit (active high)
  registerVal |= (SENSOR_POWER_EN_MASK);
  RETURN_IF_ERROR_CODE(arducamWriteSensorPowerControlReg(cameraID, registerVal));
  return errCode;
}

obc_error_code_t powerOffCamera(camera_id_t cameraID) {
  obc_error_code_t errCode;
  uint8_t registerVal = 0;
  RETURN_IF_ERROR_CODE(arducamReadSensorPowerControlReg(cameraID, &registerVal));
  // Unset power en bit (active high)
  registerVal &= (~SENSOR_POWER_EN_MASK);
  RETURN_IF_ERROR_CODE(arducamWriteSensorPowerControlReg(cameraID, registerVal));
  return errCode;
}

obc_error_code_t resetCamera(camera_id_t cameraID) {
  obc_error_code_t errCode;
  uint8_t registerVal = 0;
  RETURN_IF_ERROR_CODE(arducamReadSensorPowerControlReg(cameraID, &registerVal));
  // Unset reset bit (active low)
  registerVal &= (~SENSOR_RESET_MASK);
  RETURN_IF_ERROR_CODE(arducamWriteSensorPowerControlReg(cameraID, registerVal));
  // Set reset bit
  registerVal |= SENSOR_RESET_MASK;
  RETURN_IF_ERROR_CODE(arducamWriteSensorPowerControlReg(cameraID, registerVal));
  return errCode;
}
