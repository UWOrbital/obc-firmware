#include "qeynet_beacon_uart.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#define QEYNET_DEFAULT_BAUDRATE 115200U
#define QEYNET_BEACON_DEFAULT_PACKET_SIZE 55U

// For thermistor temperature calculations
#define T_0 25.0
#define R_0 10.0
#define B 3248.0

float qeynetAdcToVoltage(uint16_t adcValue) {
  return (((float)adcValue / 4095.0) * 1.5);
}

float qeynetVoltSupply(qeynet_beacon_uart_packet_t* packet) {
  return 33.33 * qeynetAdcToVoltage(packet->adcVIn);
}

float qeynetTempMC(qeynet_beacon_uart_packet_t* packet) {
  return (qeynetAdcToVoltage(packet->adcVTM) - 0.835) / 0.0028;
}

float qeynetResistT1(qeynet_beacon_uart_packet_t* packet) {
  return qeynetAdcToVoltage(packet->adcVT1) * 10.0 /
         (1.25 - qeynetAdcToVoltage(packet->adcVT1));
}
float qeynetResistT2(qeynet_beacon_uart_packet_t* packet) {
  return qeynetAdcToVoltage(packet->adcVoltT2) * 10.0 /
         (1.25 - qeynetAdcToVoltage(packet->adcVT2));
}

float qeynetTempT1(qeynet_beacon_uart_packet_t* packet) {
  return 1.0 / (
           1.0 / (T_0 + 273.15) +
           1.0 / B *
           log(qeynetResistT1(packet) / R_0)
         ) - 273.15;
}
float qeynetTempT2(qeynet_beacon_uart_packet_t* packet) {
  return 1.0 / (
           1.0 / (T_0 + 273.15) +
           1.0 / B *
           log(qeynetResistT2(packet) / R_0)
         ) - 273.15;
}

static inline uint16_t hexCharToInt16(unsigned char hexChar) { // ! copy or char pointer?
  if ('0' <= hexChar && hexChar <= '9') {
    return hexChar - '0';
  }
  return hexChar - 'A' + 10;
  // ! is it safe to not do the last check if A <= hexChar <= F?
  // ! probably not fix later
}

/**
 * @brief Verify that a sequence of characters is at a position in buffer
 * @param buffer Pointer to the packet
 * @param offset Initial offset in the buffer, will increment by strlen(expected) bytes afterwards
 * @return True if the sequence of characters is there, false otherwise
 */
static inline bool validateBuffer(const unsigned char* buffer, uint32_t* offset, const unsigned char* expected) {
  int expectedLen = strlen(expected);
  int cmp = memcmp(buffer + *offset, expected, expectedLen);
  if (cmp != 0) { return false; }
  (*offset) += expectedLen;
  return true;
}

/**
 * @brief Unpack buffer at offset into a uint32_t value
 * @param buffer pointer to the packet
 * @param offset Initial offset in the buffer, will increment by 4 bytes afterwards
 * @return The corresponding uint32_t value
 */
static inline uint16_t unpackInt16(const unsigned char* buffer, uint32_t* offset) {
  uint16_t value = (hexCharToInt16(buffer[*offset]) << 12) | (hexCharToInt16(buffer[*offset + 1]) << 8) |
                   (hexCharToInt16(buffer[*offset + 2]) << 4) | (hexCharToInt16(buffer[*offset + 3]));
  (*offset) += 4;
  return value;
}

// ! write comment
static inline bool parseField(const unsigned char* buffer, uint32_t* offset, const unsigned char* label, uint16_t* out, const unsigned char* terminator) {
  if (!validateBuffer(buffer, offset, label)) {
    return false;
  }
  *out = unpackInt16(buffer, offset);
  if (!validateBuffer(buffer, offset, terminator)) {
    return false;
  }
  return true;
}

obc_error_code_t qeynetBeaconUARTParsePacket(const unsigned char* packet, size_t packetLen, qeynet_beacon_uart_packet_t* parsedPacket) {
  if (packet == NULL || packetLen != QEYNET_BEACON_DEFAULT_PACKET_SIZE || parsedPacket == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  uint32_t offset = 0;
  qeynet_beacon_uart_packet_t temp;

  if (!parseField(packet, offset, "CN:", &temp.count, ",") ||
      !parseField(packet, offset, "SA:", &temp.samples, ",") ||
      !parseField(packet, offset, "VI:", &temp.adcVIn, ",") ||
      !parseField(packet, offset, "GD:", &temp.adcVGnd, ",") ||
      !parseField(packet, offset, "TM:", &temp.adcVTM, ",") ||
      !parseField(packet, offset, "T1:", &temp.adcVT1, ",") ||
      !parseField(packet, offset, "T2:", &temp.adcVT2, "\r\n")) {
    return OBC_ERR_CODE_INVALID_ARG;
  }
  *parsedPacket = temp;

  return OBC_ERR_CODE_SUCCESS;
}
