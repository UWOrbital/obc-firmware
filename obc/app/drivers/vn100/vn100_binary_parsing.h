#pragma once

#include <stdint.h>
#include <stdlib.h>

#include "obc_errors.h"

/* -------------------------------------- Packet structure Byte Sizes -------------------------- */
#define VN100_BINARY_HEADER_SIZE 4U
#define VN100_BINARY_CRC_SIZE 2U
#define VN100_BINARY_PAYLOAD_SIZE 56U  // Size excluding header and CRC
#define VN100_BINARY_PACKET_SIZE \
  VN100_BINARY_HEADER_SIZE + VN100_BINARY_PAYLOAD_SIZE + VN100_BINARY_CRC_SIZE  // Total Size

typedef struct {
  float yaw;
  float pitch;
  float roll;
  float gyroX;
  float gyroY;
  float gyroZ;
  float accelX;
  float accelY;
  float accelZ;
  float magX;
  float magY;
  float magZ;
  float temp;
  float pres;
} vn100_binary_packet_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Parse the packets into their respective packet types
 * @param packet Unparsed packet in the form of a string
 * @param packetLen The length of the packet being parsed
 * @param parsedPacket Buffer with the correct packet type
 *
 * @return OBC_ERR_CODE_SUCCESS on success, else an error code
 */
obc_error_code_t vn100ParsePacket(const unsigned char* packet, size_t packetLen, vn100_binary_packet_t* parsedPacket);

#ifdef __cplusplus
}
#endif
