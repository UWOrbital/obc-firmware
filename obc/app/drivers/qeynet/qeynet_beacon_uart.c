#include "qeynet_beacon_uart.h"
#include "obc_errors.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#define QEYNET_DEFAULT_BAUDRATE 115200U
#define QEYNET_BEACON_DEFAULT_PACKET_SIZE 57U // 55 + CR LF

// For thermistor temperature calculations
#define T_0 25.0f
#define R_0 10.0f
#define B 3248.0f

float qeynetAdcToVoltage(uint16_t adcValue) {
  return (((float)adcValue / 4095.0) * 1.5);
}

float qeynetVoltSupply(const qeynet_beacon_uart_packet_t* packet) {
  return 33.33 * qeynetAdcToVoltage(packet->adcVIn);
}

float qeynetTempMC(const qeynet_beacon_uart_packet_t* packet) {
  return (qeynetAdcToVoltage(packet->adcVTM) - 0.835) / 0.0028;
}

float qeynetResistT1(const qeynet_beacon_uart_packet_t* packet) {
  return qeynetAdcToVoltage(packet->adcVT1) * 10.0 /
         (1.25 - qeynetAdcToVoltage(packet->adcVT1));
}
float qeynetResistT2(const qeynet_beacon_uart_packet_t* packet) {
  return qeynetAdcToVoltage(packet->adcVT2) * 10.0 /
         (1.25 - qeynetAdcToVoltage(packet->adcVT2));
}

float qeynetTempT1(const qeynet_beacon_uart_packet_t* packet) {
  return 1.0 / (
           1.0 / (T_0 + 273.15) +
           1.0 / B *
           log(qeynetResistT1(packet) / R_0)
         ) - 273.15;
}
float qeynetTempT2(const qeynet_beacon_uart_packet_t* packet) {
  return 1.0 / (
           1.0 / (T_0 + 273.15) +
           1.0 / B *
           log(qeynetResistT2(packet) / R_0)
         ) - 273.15;
}

/**
 * @brief Convert a hexadecimal character to its 4-bit integer value
 * @param hexChar The hexadecimal character
 * @param out Where to output the integer value
 * @return True if hexChar is a valid hexadecimal character, otherwise false
 */
static inline bool hexCharToInt16(char hexChar, uint16_t* out) {
  if ('0' <= hexChar && hexChar <= '9') {
    *out = hexChar - '0';
    return true;
  }
  if ('A' <= hexChar && hexChar <= 'F') {
    *out = hexChar - 'A' + 10;
    return true;
  }
  return false;
}

/**
 * @brief Verify that a sequence of characters is at a position in buffer
 * @param buffer Pointer to the packet
 * @param offset Initial offset in the buffer, will increment by strlen(expected) bytes afterwards
 * @return True if the sequence of characters is there, false otherwise
 */
static inline bool validateBuffer(const char* buffer, uint32_t* offset, const char* expected) {
  int expectedLen = strlen(expected);
  int cmp = memcmp(buffer + *offset, expected, expectedLen);
  if (cmp != 0) { return false; }

  (*offset) += expectedLen;
  return true;
}

/**
 * @brief Unpack hexadecimal buffer at offset into a uint32_t value
 * @param buffer Pointer to the packet
 * @param offset Initial offset in the buffer, will increment by 4 bytes afterwards
 * @param out 
 * @return The corresponding uint32_t value
 */
static inline bool unpackInt16(const char* buffer, uint32_t* offset, uint16_t* out) {
  uint16_t temp = 0, value = 0;
  if (!hexCharToInt16(buffer[*offset], &temp)) {
    return false;
  }
  value |= temp << 12;
  if (!hexCharToInt16(buffer[*offset + 1], &temp)) {
    return false;
  }
  value |= temp << 8;
  if (!hexCharToInt16(buffer[*offset + 2], &temp)) {
    return false;
  }
  value |= temp << 4;
  if (!hexCharToInt16(buffer[*offset + 3], &temp)) {
    return false;
  }
  value |= temp;

  (*offset) += 4;
  (*out) = value;
  return true;
}

/**
 * @brief Read hexadecimal field of Qeynet beacon packet, including preceding label and terminator
 * @param buffer Pointer to the packet
 * @param offset Initial offset in the buffer, will increment by strlen(label) + 4 + strlen(terminator) bytes afterwards
 * @param label The characters expected to precede the hexadecimal value
 * @param out Where to output the hexadecimal value read
 * @param terminator The characters expected to follow the hexadecimal value
 * @return True if successful, false otherwise
 */
static inline bool parseField(const char* buffer, uint32_t* offset, const char* label, uint16_t* out, const char* terminator) {
  if (!validateBuffer(buffer, offset, label)) {
    return false;
  }
  uint16_t temp;
  if (!unpackInt16(buffer, offset, &temp)) {
    return false;
  }
  if (!validateBuffer(buffer, offset, terminator)) {
    return false;
  }

  *out = temp;
  return true;
}

obc_error_code_t qeynetBeaconUARTParsePacket(const char* packet, size_t packetLen, qeynet_beacon_uart_packet_t* parsedPacket) {
  if (packet == NULL || packetLen != QEYNET_BEACON_DEFAULT_PACKET_SIZE || parsedPacket == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  uint32_t offset = 0;
  qeynet_beacon_uart_packet_t temp;

  if (!parseField(packet, &offset, "CN:", &temp.count, ",") ||
      !parseField(packet, &offset, "SA:", &temp.samples, ",") ||
      !parseField(packet, &offset, "VI:", &temp.adcVIn, ",") ||
      !parseField(packet, &offset, "GD:", &temp.adcVGnd, ",") ||
      !parseField(packet, &offset, "TM:", &temp.adcVTM, ",") ||
      !parseField(packet, &offset, "T1:", &temp.adcVT1, ",") ||
      !parseField(packet, &offset, "T2:", &temp.adcVT2, "\r\n")) {
    return OBC_ERR_CODE_INVALID_ARG;
  }
  *parsedPacket = temp;

  return OBC_ERR_CODE_SUCCESS;
}
