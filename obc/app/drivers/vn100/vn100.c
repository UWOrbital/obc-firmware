#include "vn100.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "obc_board_config.h"
#include "obc_errors.h"
#include "obc_logging.h"
#include "obc_sci_io.h"

#define VN100_DEFAULT_BAUDRATE 115200U
#define MAX_SEND_SIZE 120U
#define MUTEX_TIMEOUT portMAX_DELAY
#define MAX_BAUDRATE_LENGTH 7U
#define MAX_OUTPUT_RATE_LENGTH 3U
#define DEFAULT_OUTPUT_RATE_HZ 20U
#define SCI_SEMAPHORE_TIMEOUT_MS \
  50U /* Time between successive sensor outputs (period): 1/output rate = 1/20Hz = 0.05s = 50ms */

/* Building start and stop binary output comands */
#define BINARY_OUTPUT_START_PREFIX "$VNWRG,75,2,"  // Configure write command to output on register 75 and serial port 2
#define BINARY_OUTPUT_STOP_PREFIX "$VNWRG,75,0,"   // Configure write command to stop outputs on register 75
#define BINARY_OUTPUT_RATE_DIVISOR \
  "40"  // Calculated by taking the IMU rate = 800Hz and dividing by the desired Output rate = 20Hz (800/20 = 40)
#define BINARY_OUTPUT_POSTFIX \
  ",01,0528*XX\r\n"  // Configure to use output group 1 and enable the aforemore mentioned outputs, see section 4.2.4
                     // for more details.
#define START_BINARY_OUTPUTS \
  BINARY_OUTPUT_START_PREFIX BINARY_OUTPUT_RATE_DIVISOR BINARY_OUTPUT_POSTFIX /* $VNWRG,75,2,80,01,0528*XX\r\n */
#define STOP_BINARY_OUTPUTS \
  BINARY_OUTPUT_STOP_PREFIX BINARY_OUTPUT_RATE_DIVISOR BINARY_OUTPUT_POSTFIX /* $VNWRG,75,0,80,01,0528*XX\r\n */

static obc_error_code_t isValidBaudRate(uint32_t baudRate);

static obc_error_code_t isValidOutputRate(uint32_t outputRateHz);

void initVn100(void) {
  sciSetBaudrate(UART_VN100_REG, VN100_DEFAULT_BAUDRATE);

  vn100SetBaudrate(VN100_DEFAULT_BAUDRATE);

  vn100StartBinaryOutputs();

  vn100SetOutputRate(DEFAULT_OUTPUT_RATE_HZ);

  vn100ResumeAsync();
}

obc_error_code_t vn100ResetModule(void) {
  obc_error_code_t errCode;
  unsigned char buf[] = "$VNRST*4D\r\n";
  RETURN_IF_ERROR_CODE(sciSendBytes(buf, (sizeof(buf) - 1), MUTEX_TIMEOUT, UART_VN100_REG));
  return OBC_ERR_CODE_SUCCESS;
}

static obc_error_code_t isValidBaudRate(uint32_t baudRate) {
  const uint32_t validBaudRate[] = {9600, 19200, 38400, 57600, 115200, 128000, 230400, 460800, 921600};
  uint32_t length = sizeof(validBaudRate) / sizeof(uint32_t);
  for (uint32_t i = 0; i < length; i++) {
    if (baudRate == validBaudRate[i]) {
      return OBC_ERR_CODE_SUCCESS;
    }
  }
  return OBC_ERR_CODE_INVALID_ARG;
}

static obc_error_code_t isValidOutputRate(uint32_t outputRateHz) {
  const uint32_t validOutputRate[] = {1, 2, 4, 5, 10, 20, 25, 40, 50, 100, 200};
  uint32_t length = sizeof(validOutputRate) / sizeof(uint32_t);
  for (uint32_t i = 0; i < length; i++) {
    if (outputRateHz == validOutputRate[i]) {
      return OBC_ERR_CODE_SUCCESS;
    }
  }
  return OBC_ERR_CODE_INVALID_ARG;
}

obc_error_code_t vn100SetBaudrate(uint32_t baudrate) {
  obc_error_code_t errCode;
  RETURN_IF_ERROR_CODE(isValidBaudRate(baudrate));

  char baud[MAX_BAUDRATE_LENGTH] = {'\0'};

  /* For documentation on how these commands are formed, refer to section 5.2.6 of the user manual */
  const char checksum[] = "*XX\r\n";   // Checksum set to "XX", which means to ignore the checksum
  const char header[] = "$VNWRG,05,";  // Header set to write a command to register 7
  unsigned char buf[MAX_SEND_SIZE] = {'\0'};
  snprintf(baud, sizeof(baud), "%ld", baudrate);

  size_t headerLength = strlen(header);
  size_t baudrateLength = strlen(baud);
  size_t checksumLength = strlen(checksum);

  // Begin appending the command
  memcpy(buf, header, headerLength);
  memcpy(buf + headerLength, baud, baudrateLength);
  memcpy(buf + headerLength + baudrateLength, checksum, checksumLength);

  size_t numBytes = headerLength + baudrateLength + checksumLength;

  RETURN_IF_ERROR_CODE(sciSendBytes(buf, numBytes, MUTEX_TIMEOUT, UART_VN100_REG));
  sciSetBaudrate(UART_VN100_REG, baudrate);
  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t vn100SetOutputRate(uint32_t outputRateHz) {
  obc_error_code_t errCode;
  RETURN_IF_ERROR_CODE(isValidOutputRate(outputRateHz));
  // Make the size of the string representation sufficiently large, use memcpy to append string onto req
  char freq[MAX_OUTPUT_RATE_LENGTH];

  /* For documentation on how these commands are formed, refer to section 5.2.8 of the user manual */
  const char checksum[] = "*XX\r\n";   // Checksum set to "XX", which means to ignore the checksum
  const char header[] = "$VNWRG,07,";  // Header set to write a command to register 7
  unsigned char buf[MAX_SEND_SIZE];
  snprintf(freq, sizeof(freq), "%ld", outputRateHz);

  size_t headerLength = strlen(header);
  size_t freqLength = strlen(freq);
  size_t checksumLength = strlen(checksum);

  // Begin appending the command
  memcpy(buf, header, headerLength);
  memcpy(buf + headerLength, freq, freqLength);
  memcpy(buf + headerLength + freqLength, checksum, checksumLength);

  size_t numBytes = headerLength + freqLength + checksumLength;

  RETURN_IF_ERROR_CODE(sciSendBytes(buf, numBytes, MUTEX_TIMEOUT, UART_VN100_REG));
  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t vn100StartBinaryOutputs(void) {
  /* Outputs: Yaw Pitch Roll, Angular rates, Accelerometer data, Magnetometer, Temp and Pressure.
     Initialized to start with an output rate of 10Hz */
  obc_error_code_t errCode;
  RETURN_IF_ERROR_CODE(sciSendBytes((unsigned char*)(START_BINARY_OUTPUTS), (sizeof(START_BINARY_OUTPUTS) - 1),
                                    portMAX_DELAY, UART_VN100_REG));
  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t vn100StopBinaryOutputs(void) {
  obc_error_code_t errCode;
  RETURN_IF_ERROR_CODE(sciSendBytes((unsigned char*)(STOP_BINARY_OUTPUTS), (sizeof(STOP_BINARY_OUTPUTS) - 1),
                                    portMAX_DELAY, UART_VN100_REG));
  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t vn100ReadBinaryOutputs(vn100_binary_packet_t* parsedPacket) {
  if (parsedPacket == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  unsigned char buf[VN100_BINARY_PACKET_SIZE] = {'\0'};
  obc_error_code_t errCode;
  RETURN_IF_ERROR_CODE(sciReadBytes(buf, VN100_BINARY_PACKET_SIZE, portMAX_DELAY,
                                    pdMS_TO_TICKS(SCI_SEMAPHORE_TIMEOUT_MS), UART_VN100_REG));

  RETURN_IF_ERROR_CODE(vn100ParsePacket(buf, VN100_BINARY_PACKET_SIZE, parsedPacket));
  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t vn100PauseAsync(void) {
  obc_error_code_t errCode;
  unsigned char command[] = "$VNASY,0*XX\r\n";
  RETURN_IF_ERROR_CODE(sciSendBytes(command, (sizeof(command) - 1), MUTEX_TIMEOUT, UART_VN100_REG));
  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t vn100ResumeAsync(void) {
  obc_error_code_t errCode;
  unsigned char command[] = "$VNASY,1*XX\r\n";
  RETURN_IF_ERROR_CODE(sciSendBytes(command, (sizeof(command) - 1), MUTEX_TIMEOUT, UART_VN100_REG));
  return OBC_ERR_CODE_SUCCESS;
}
