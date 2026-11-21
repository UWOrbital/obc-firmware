#include "obc_errors.h"

#include <stdint.h>
#include <string.h>

typedef struct {
    uint16_t count;
    uint16_t samples;
    uint16_t adcVIn;
    uint16_t adcVGnd;
    uint16_t adcVTM;
    uint16_t adcVT1;
    uint16_t adcVT2;
} qeynet_beacon_uart_packet_t;

#ifdef __cplusplus
extern "C" {
#endif

/**const 
 * @brief Convert adc value from qeynet uart packet to a decimal voltage
 * @param adcValue The adc value to convert
 * @return The decimal voltage
 */
float qeynetAdcToVoltage(uint16_t adcValue);

/**
 * @brief Calculate supply voltage from qeynet uart packet
 * @param packet The packet to calculate from
 * @return The supply voltage
 */
float qeynetVoltSupply(const qeynet_beacon_uart_packet_t* packet);

/**
 * @brief Calculate microcontroller temperature from qeynet uart packet
 * @param packet The packet to calculate from
 * @return The microcontroller temperature
 */
float qeynetTempMC(const qeynet_beacon_uart_packet_t* packet);

/**
 * @brief Calculate thermistor 1 resistance from qeynet uart packet
 * @param packet The packet to calculate from
 * @return The thermistor 1 resistance
 */
float qeynetResistT1(const qeynet_beacon_uart_packet_t* packet);
/**
 * @brief Calculate thermistor 2 resistance from qeynet uart packet
 * @param packet The packet to calculate from
 * @return The thermistor 2 resistance
 */
float qeynetResistT2(const qeynet_beacon_uart_packet_t* packet);

/**
 * @brief Calculate thermistor 1 temperature from qeynet uart packet
 * @param packet The packet to calculate from
 * @return The thermistor 1 temperature
 */
float qeynetTempT1(const qeynet_beacon_uart_packet_t* packet);
/**
 * @brief Calculate thermistor 2 temperature from qeynet uart packet
 * @param packet The packet to calculate from
 * @return The thermistor 2 temperature
 */
float qeynetTempT2(const qeynet_beacon_uart_packet_t* packet);

/**
 * @brief Parse UART packet
 * @param packet Unparsed packet in the form of a string
 * @param packetLen The length of the packet being parsed
 * @param parsedPacket Buffer with the correct packet type
 *
 * @return OBC_ERR_CODE_SUCCESS on success, else an error code
 */
obc_error_code_t qeynetBeaconUARTParsePacket(const char* packet, size_t packetLen, qeynet_beacon_uart_packet_t* parsedPacket);

#ifdef __cplusplus
}
#endif
