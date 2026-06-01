#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "obc_errors.h"
#include <stdint.h>
#include <stdbool.h>

// Port 0 Pin Locations
#define TCA6424A_PIN_00 0x00
#define TCA6424A_PIN_01 0x01
#define TCA6424A_PIN_02 0x02
#define TCA6424A_PIN_03 0x03
#define TCA6424A_PIN_04 0x04
#define TCA6424A_PIN_05 0x05
#define TCA6424A_PIN_06 0x06
#define TCA6424A_PIN_07 0x07

// Port 1 Pin Locations
#define TCA6424A_PIN_10 0x10
#define TCA6424A_PIN_11 0x11
#define TCA6424A_PIN_12 0x12
#define TCA6424A_PIN_13 0x13
#define TCA6424A_PIN_14 0x14
#define TCA6424A_PIN_15 0x15
#define TCA6424A_PIN_16 0x16
#define TCA6424A_PIN_17 0x17

// Port 2 Pin Locations
#define TCA6424A_PIN_20 0x20
#define TCA6424A_PIN_21 0x21
#define TCA6424A_PIN_22 0x22
#define TCA6424A_PIN_23 0x23
#define TCA6424A_PIN_24 0x24
#define TCA6424A_PIN_25 0x25
#define TCA6424A_PIN_26 0x26
#define TCA6424A_PIN_27 0x27

// Register Addresses (We use port 0 as starting point and then do arithmetic to move around to port 1 and 2)
#define TCA6424A_INPUT_PORT_ZERO_ADDR 0x00
#define TCA6424A_OUTPUT_PORT_ZERO_ADDR 0x04
#define TCA6424A_CONFIGURATION_PORT_ZERO_ADDR 0x0C

// Each port is mapped to 8 different pins, therefore giving us 24 total IO options from the expander.
#define MAX_PORT_INDEX 2
#define MAX_PIN_INDEX 7

// I2C Transfer timeout
#define TCA6424A_I2C_TRANSFER_TIMEOUT_TICKS pdMS_TO_TICKS(100)

typedef enum { TCA6424A_IO_OUTPUT, TCA6424A_IO_INPUT } tca6424a_IO_t;

typedef enum { TCA6424A_LEVEL_LOW, TCA6424A_LEVEL_HIGH } tca6424a_level_t;

/**
 * @struct tca6424a_t
 * @brief TCA6424A Device Handle.
 */
typedef struct {
  uint8_t i2cAddress;
} tca6424a_t;

/**
 * @brief Configures a specific pin as an Input or Output.
 * @param device Pointer to the TCA6424A device handle.
 * @param pinLocation The specific pin macro (e.g. TCA6424A_PIN_04).
 * @param IO The desired direction of IO (Input/Output)
 * @return OBC_ERR_CODE_SUCCESS if successful, error code otherwise.
 */
obc_error_code_t tca6424aSetIO(const tca6424a_t *device, uint8_t pinLocation, tca6424a_IO_t IO);

/**
 * @brief Sets an output pin to be high or low.
 * @param device Pointer to the TCA6424A device handle.
 * @param pinLocation The specific pin macro (e.g. TCA6424A_PIN_04).
 * @param level The desired level (High or Low).
 * @return OBC_ERR_CODE_SUCCESS if successful, error code otherwise.
 */
obc_error_code_t tca6424aWritePin(const tca6424a_t *device, uint8_t pinLocation, tca6424a_level_t level);

/**
 * @brief Gets the current state of an input pin (whether its high or low).
 * @param device Pointer to the TCA6424A device handle.
 * @param pinLocation The specific pin macro (e.g. TCA6424A_PIN_04).
 * @param level Out pointer to store the read level (High or Low).
 * @return OBC_ERR_CODE_SUCCESS if successful, error code otherwise.
 */
obc_error_code_t tca6424aReadPin(const tca6424a_t *device, uint8_t pinLocation, tca6424a_level_t *level);

#ifdef __cplusplus
}
#endif
