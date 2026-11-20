typedef struct {
    uint16_t count;
    uint16_t samples;
    uint16_t adcVIn;
    uint16_t adcVGnd;
    uint16_t adcVTM;
    uint16_t adcVT1;
    uint16_t adcVT2;
} qeynet_beacon_uart_packet_t;

// ! write comments
float qeynetAdcToVoltage(uint16_t adcValue);

float qeynetVoltSupply(qeynet_beacon_uart_packet_t* packet);

float qeynetTempMC(qeynet_beacon_uart_packet_t* packet);

float qeynetResistT1(qeynet_beacon_uart_packet_t* packet);
float qeynetResistT2(qeynet_beacon_uart_packet_t* packet);

float qeynetTempT1(qeynet_beacon_uart_packet_t* packet);
float qeynetTempT2(qeynet_beacon_uart_packet_t* packet);

/**
 * @brief Parse UART packet
 * @param packet Unparsed packet in the form of a string
 * @param packetLen The length of the packet being parsed
 * @param parsedPacket Buffer with the correct packet type
 *
 * @return OBC_ERR_CODE_SUCCESS on success, else an error code
 */
obc_error_code_t qeynetBeaconUARTParsePacket(const unsigned char* packet, size_t packetLen, qeynet_beacon_uart_packet_t* parsedPacket);
