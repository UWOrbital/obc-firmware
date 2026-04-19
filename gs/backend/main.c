#include "obc_gs_command_data.h"
#include "obc_gs_command_id.h"
#include "obc_gs_command_pack.h"

#include "gs_errors.h"
#include "obc_gs_errors.h"

#include "obc_gs_aes128.h"
#include "obc_gs_ax25.h"
#include "obc_gs_fec.h"

#include <aes.h>
#include <cserialport.h>

#ifdef __APPLE__
#ifdef TARGET_OS_MAC
#include <CoreFoundation/CoreFoundation.h>
#endif
#endif
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

const uint8_t TEMP_STATIC_KEY[AES_KEY_SIZE] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                                               0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

static correct_reed_solomon *rsGs;

static gs_error_code_t decodePacket(packed_ax25_i_frame_t *ax25data, packed_rs_packet_t *rsData);

static uint32_t getCurrentTime(void);

int main(void) {
  gs_error_code_t gsErrCode;
  obc_gs_error_code_t obcGsErrCode;

  char demoNum = 0;

  printf(
      "\n0: Immediate RTC Sync"
      "\n1: Immediate Ping"
      "\n2: Time-tagged Ping (15 sec in future)");

  scanf("%c", &demoNum);

  // Check if the input is within the valid range
  if (demoNum >= 0 && demoNum <= 2) {
    printf("You entered: %d\n", demoNum);
  } else {
    printf("Invalid input. Please enter a number between 0 and 2.\n");
  }

  /* ----------------------------------- Begin CSerialPort setup
   * ------------------------------------------- */

  void *pSerialPort = NULL;

  pSerialPort = CSerialPortMalloc();
  printf("Version: %s\n\n", CSerialPortGetVersion(pSerialPort));

  printf("Available Friendly Ports:\n");

  struct SerialPortInfoArray portInfoArray = {0};
  CSerialPortAvailablePortInfos(&portInfoArray);

  for (unsigned int i = 0; i < portInfoArray.size; ++i) {
    printf("%u - %s %s\n", i + 1, portInfoArray.portInfo[i].portName, portInfoArray.portInfo[i].description);
  }

  if (portInfoArray.size == 0) {
    printf("No Valid Port\n");
  } else {
    printf("\n");

    unsigned int input = 0;
    do {
      printf("Please Input The Index Of Port(1 - %d)\n", portInfoArray.size);

      scanf("%u", &input);

      if (input >= 1 && input <= portInfoArray.size) {
        break;
      }
    } while (1);

    char portName[256] = {0};
    strcpy(portName, portInfoArray.portInfo[input - 1].portName);
    printf("Port Name: %s\n", portName);

    CSerialPortAvailablePortInfosFree(&portInfoArray);

    CSerialPortInit(pSerialPort,
                    portName,    // windows:COM1 Linux:/dev/ttyS0
                    115200,      // baudrate
                    ParityNone,  // parity
                    DataBits8,   // data bit
                    StopTwo,     // stop bit
                    FlowNone,    // flow
                    4096         // read buffer size
    );

    if (demoNum == 2) {
      CSerialPortSetReadIntervalTimeout(pSerialPort,
                                        30000);  // read interval timeout
    } else {
      CSerialPortSetReadIntervalTimeout(pSerialPort,
                                        2500);  // read interval timeout
    }

    CSerialPortOpen(pSerialPort);

    printf("Open %s %s\n", portName, 1 == CSerialPortIsOpen(pSerialPort) ? "Success" : "Failed");
    printf("Code: %d, Message: %s\n", CSerialPortGetLastError(pSerialPort), CSerialPortGetLastErrorMsg(pSerialPort));
  }

  printf("Serial port configured!\n");

  /* ------------------------------------------- Demo Code
   * ----------------------------------------------------- */

  /* Construct packet */

  cmd_msg_t cmdMsg = {0};

  switch (demoNum) {
    case 0: {
      printf("Sending RTC Sync Command\n");
      cmdMsg.id = CMD_RTC_SYNC;
      cmdMsg.isTimeTagged = false;
      cmdMsg.rtcSync.unixTime = getCurrentTime();
      break;
    }

    case 1: {
      printf("Sending immediate ping command\n");
      cmdMsg.id = CMD_PING;
      cmdMsg.isTimeTagged = false;
      break;
    }

    case 2: {
      printf("Sending time-tagged ping command (15 sec in the future)\n");
      cmdMsg.id = CMD_PING;
      cmdMsg.isTimeTagged = true;
      cmdMsg.rtcSync.unixTime = getCurrentTime() + 15;
      break;
    }

    default:
      printf("Invalid demo number!");
      exit(1);
  }

  struct AES_ctx ctx = {0};
  AES_init_ctx(&ctx, TEMP_STATIC_KEY);

  uint8_t iv[AES_IV_SIZE] = {0};
  memset(iv, 1, AES_IV_SIZE);
  AES_ctx_set_iv(&ctx, iv);

  rsGs = correct_reed_solomon_create(correct_rs_primitive_polynomial_ccsds, 1, 1, 32);

  uint32_t cmdPacketOffset = 0;

  uint8_t packedSingleCmdSize = 0;
  uint8_t packedSingleCmd[MAX_CMD_MSG_SIZE] = {0};
  if (packCmdMsg(packedSingleCmd, &cmdPacketOffset, &cmdMsg, &packedSingleCmdSize) != OBC_GS_ERR_CODE_SUCCESS) {
    printf("Failed to pack command message!");
    exit(1);
  }

  uint8_t encryptedCmd[RS_DECODED_SIZE] = {0};

  memcpy(encryptedCmd + AES_IV_SIZE, packedSingleCmd, packedSingleCmdSize);

  AES_CTR_xcrypt_buffer(&ctx, encryptedCmd + AES_IV_SIZE, AES_DECRYPTED_SIZE);

  memcpy(encryptedCmd, iv, AES_IV_SIZE);

  if (cmdPacketOffset != 0) {
    packed_ax25_i_frame_t ax25Pkt = {0};
    unstuffed_ax25_i_frame_t unstuffedAx25Pkt = {0};

    // Perform AX.25 framing
    setCurrentLinkDestCallSign(GROUND_STATION_CALLSIGN, CALLSIGN_LENGTH, DEFAULT_SSID);
    obcGsErrCode = ax25SendIFrame(encryptedCmd, RS_DECODED_SIZE, &unstuffedAx25Pkt);
    if (obcGsErrCode != OBC_GS_ERR_CODE_SUCCESS) {
      printf("Failed to send AX.25 I-Frame!");
      exit(1);
    }

    packed_rs_packet_t fecPkt = {0};

    // Apply Reed Solomon FEC
    if ((uint8_t)correct_reed_solomon_encode(rsGs, unstuffedAx25Pkt.data + AX25_INFO_FIELD_POSITION, RS_DECODED_SIZE,
                                             fecPkt.data) < RS_ENCODED_SIZE) {
      exit(1);
    };

    memcpy(unstuffedAx25Pkt.data + AX25_INFO_FIELD_POSITION, fecPkt.data, RS_ENCODED_SIZE);

    obcGsErrCode = ax25Stuff(unstuffedAx25Pkt.data, unstuffedAx25Pkt.length, ax25Pkt.data, &ax25Pkt.length);
    if (obcGsErrCode != OBC_GS_ERR_CODE_SUCCESS) {
      printf("Failed to stuff AX.25 packet!");
      exit(1);
    }

    ax25Pkt.data[0] = AX25_FLAG;
    ax25Pkt.data[ax25Pkt.length - 1] = AX25_FLAG;

    long unsigned int bytesWritten = CSerialPortWriteData(pSerialPort, ax25Pkt.data, ax25Pkt.length);
    if (bytesWritten < ax25Pkt.length) {
      printf("Failed to write entire AX.25 packet!");
      exit(1);
    }

    printf("Bytes Written: %lu\n", bytesWritten);
  }

  /* Receive Data */

  uint16_t axDataIndex = 0;
  packed_ax25_i_frame_t axData = {0};
  bool startFlagReceived = false;

  while (1) {
    uint8_t byte = '\0';
    if (CSerialPortReadData(pSerialPort, &byte, 1) < 0) {
      printf("Error Reading! \r\n");
      break;
    }

    if (axDataIndex >= sizeof(axData.data)) {
      // Restart the decoding process
      memset(&axData, 0, sizeof(axData));
      axDataIndex = 0;
      startFlagReceived = false;
    }

    if (byte == AX25_FLAG) {
      axData.data[axDataIndex++] = byte;

      // Decode packet if we have start flag, end flag, and at least 1 byte of
      // data During idling, multiple AX25_FLAGs may be sent in a row, so we
      // enforce that axData.data[1] must be something other than AX25_FLAG
      if (axDataIndex > 2) {
        axData.length = axDataIndex;

        packed_rs_packet_t rsData = {0};
        gsErrCode = decodePacket(&axData, &rsData);
        if (gsErrCode != GS_ERR_CODE_SUCCESS) {
          printf("Failed to decode packet!");
          exit(1);
        }

        // Restart the decoding process
        memset(&axData, 0, sizeof(axData));
        axDataIndex = 0;
        startFlagReceived = false;
      } else {
        startFlagReceived = true;
        axDataIndex = 1;
      }

      continue;
    }

    if (startFlagReceived) {
      axData.data[axDataIndex++] = byte;
    }
  }

  /* ----------------------------- Disconnect from the Serial Port
   * ----------------------------- */

  CSerialPortFree(pSerialPort);
}

static gs_error_code_t decodePacket(packed_ax25_i_frame_t *ax25Data, packed_rs_packet_t *rsData) {
  obc_gs_error_code_t interfaceErr;

  // perform bit unstuffing
  unstuffed_ax25_i_frame_t unstuffedPacket = {0};
  interfaceErr = ax25Unstuff(ax25Data->data, ax25Data->length, unstuffedPacket.data, &unstuffedPacket.length);
  if (interfaceErr != OBC_GS_ERR_CODE_SUCCESS) {
    return GS_ERR_CODE_AX25_DECODE_FAILURE;
  }

  if (unstuffedPacket.length == AX25_MINIMUM_I_FRAME_LEN) {
    // copy the unstuffed data into rsData
    memcpy(rsData->data, unstuffedPacket.data + AX25_INFO_FIELD_POSITION, RS_ENCODED_SIZE);
    // clear the info field of the unstuffed packet
    memset(unstuffedPacket.data + AX25_INFO_FIELD_POSITION, 0, RS_ENCODED_SIZE);
    // decode the info field and store it in the unstuffed packet
    uint8_t decodedLength = correct_reed_solomon_decode(rsGs, rsData->data, RS_ENCODED_SIZE,
                                                        unstuffedPacket.data + AX25_INFO_FIELD_POSITION);

    if (decodedLength == -1) {
      return GS_ERR_CODE_CORRUPTED_MSG;
    }
  }

  // check for a valid ax25 frame and perform the command response if necessary
  u_frame_cmd_t receivedCmd = {0};
  interfaceErr = ax25Recv(&unstuffedPacket, &receivedCmd);

  uint8_t decodedData[RS_DECODED_SIZE] = {0};
  memcpy(decodedData, unstuffedPacket.data + AX25_INFO_FIELD_POSITION, RS_DECODED_SIZE);

  printf("Received (and decoded) data: ");
  for (uint8_t i = 0; i < RS_DECODED_SIZE; ++i) {
    printf("%x ", decodedData[i]);
  }
  printf("\n");

  return GS_ERR_CODE_SUCCESS;
}

static uint32_t getCurrentTime(void) {
  time_t ts = time(NULL);
  if (ts < 0 || ts > UINT32_MAX) {
    printf("Current time not a uint32_t!");
    exit(1);
  }
  return (uint32_t)ts;
}
