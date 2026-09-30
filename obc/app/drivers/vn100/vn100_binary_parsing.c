#include "vn100_binary_parsing.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data_unpack_utils.h"
#include "obc_errors.h"
#include "obc_gs_crc.h"

/* ------------------------------------------- Packet Error Checking ------------------------------------*/
#define DEFAULT_SYNC_BYTE 0xFA
#define SYNC_HEADER_LENGTH 1U
#define PAYLOAD_OFFSET VN100_BINARY_HEADER_SIZE
#define VALID_CHECKSUM_RETURN 0U
#define VN100_ERROR_HEADER "$VNERR,"
#define ERROR_BUFFER_OVERFLOW 255U
#define VN100_ERROR_TO_OBC_ERROR(err) \
  (obc_error_code_t)((err) + 300) /* VN100 error codes have been mapped to be OBC error codes */

static inline uint8_t singleDigitAsciiToInt(char digit) { return (uint8_t)(digit - '0'); }

/**
 * @brief Check the first pyte of the packet.
 * @param buffer pointer to the packet
 * @return If it is not 0xFA, return false (0) otherwise true (1).
 */
static inline bool isSyncByteValid(const unsigned char* buffer) {
  if (buffer[0] == DEFAULT_SYNC_BYTE) {
    return true;
  }
  return false;
}

/**
 * @brief Unpack buffer into a uin32_t value
 * @param buffer pointer to the packet
 * @param offset Initial offset in the buffer, will increment by 4 bytes afterwards
 * @return The corresponding uint32_t value
 */
static inline uint32_t unpackInt32LittleEndian(const uint8_t* buffer, uint32_t* offset) {
  uint32_t value = ((uint32_t)buffer[*offset + 3] << 24) | ((uint32_t)buffer[*offset + 2] << 16) |
                   ((uint32_t)buffer[*offset + 1] << 8) | ((uint32_t)buffer[*offset]);
  (*offset) += 4;
  return value;
}

/**
 * @brief Unpack buffer into a float value
 * @param buffer pointer to the packet
 * @param offset Initial offset in the buffer, will increment by 4 bytes afterwards
 * @return The corresponding float value
 */
static inline float unpackFloatLittleEndian(const uint8_t* buffer, uint32_t* offset) {
  float val = 0.0;

  uint32_t tmp = unpackInt32LittleEndian(buffer, offset);
  memcpy(&val, &tmp, sizeof(val));

  return val;
}

/**
 * @brief Extracts the corresponding error code
 * @param buffer pointer to the packet
 * @return A VN_100 error code
 */
static inline obc_error_code_t extractErrorCode(const unsigned char* buffer) {
  uint8_t vn100Error = buffer[strlen(VN100_ERROR_HEADER)];

  /* Specifically check for ERROR_BUFFER_OVERFLOW because it cannot be directly mapped to an OBC error using the macro
     as it would exceed the space allocated for ADCS errors. See section 3.7 for more details on the VN100 error codes.
   */
  if (vn100Error == ERROR_BUFFER_OVERFLOW) {
    return OBC_ERR_CODE_VN100_ERROR_BUFFER_OVERFLOW;
  }
  obc_error_code_t errorCode = VN100_ERROR_TO_OBC_ERROR(singleDigitAsciiToInt(vn100Error));
  return errorCode;
}

obc_error_code_t vn100ParsePacket(const unsigned char* packet, size_t packetLen, vn100_binary_packet_t* parsedPacket) {
  if (packet == NULL || packetLen != VN100_BINARY_PACKET_SIZE || parsedPacket == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  if (strncmp((const char*)packet, VN100_ERROR_HEADER, strlen(VN100_ERROR_HEADER)) == 0) {
    return extractErrorCode(packet);
  }

  if (!isSyncByteValid(packet)) {
    return OBC_ERR_CODE_VN100_INVALID_SYNC_BYTE;
  }

  /* The CRC is selected such that if you compute the 16-bit CRC starting with the group byte
  and include the CRC itself, a valid packet will result in 0x0000 computed
  by the running CRC calculation over the entire packet. */

  uint16_t vn100Crc = calculateCrc16Ccitt(&packet[1], VN100_BINARY_PACKET_SIZE - 1U);

  uint32_t offset = PAYLOAD_OFFSET;

  if (vn100Crc == VALID_CHECKSUM_RETURN) {
    parsedPacket->yaw = unpackFloatLittleEndian(packet, &offset);
    parsedPacket->pitch = unpackFloatLittleEndian(packet, &offset);
    parsedPacket->roll = unpackFloatLittleEndian(packet, &offset);

    parsedPacket->gyroX = unpackFloatLittleEndian(packet, &offset);
    parsedPacket->gyroY = unpackFloatLittleEndian(packet, &offset);
    parsedPacket->gyroZ = unpackFloatLittleEndian(packet, &offset);

    parsedPacket->accelX = unpackFloatLittleEndian(packet, &offset);
    parsedPacket->accelY = unpackFloatLittleEndian(packet, &offset);
    parsedPacket->accelZ = unpackFloatLittleEndian(packet, &offset);

    parsedPacket->magX = unpackFloatLittleEndian(packet, &offset);
    parsedPacket->magY = unpackFloatLittleEndian(packet, &offset);
    parsedPacket->magZ = unpackFloatLittleEndian(packet, &offset);

    parsedPacket->temp = unpackFloatLittleEndian(packet, &offset);
    parsedPacket->pres = unpackFloatLittleEndian(packet, &offset);

    return OBC_ERR_CODE_SUCCESS;
  }

  return OBC_ERR_CODE_VN100_INVALID_CHECKSUM;
}
