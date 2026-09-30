#pragma once

#include <stdint.h>

#include "obc_errors.h"
#include "vn100_binary_parsing.h"

/* To access the user manual for VN-100 click here --> https://geo-matching.com/media/migrationpiwnum.pdf */

/**
 * @brief Initiates VN100 with the default parameters for baudrate, polling rate, etc
 * Defaults:
 * User Tag: NULL
 * Serial Baud Rate: 115200
 * Async Data Output Frequency: 10 Hz
 * Async Data Output Type: INS_LLA
 * Syncronization control: 3,0,0,0,6,1,0,100000000,0
 *
 */
void initVn100(void);

/**
 * @brief Reset the VN-100 to its factory defaults. After this function is called you must restart ASCII or Binary
 * outputs
 *
 * @return OBC_ERR_CODE_SUCCESS on success, else an error code
 */

obc_error_code_t vn100ResetModule(void);

/**
 * @brief Set the baudrate for the VN-100 peripheral
 * @param baudrate The desired baudrate to be set
 * @note The default baudrate is 115200 Hz
 *
 * @return OBC_ERR_CODE_SUCCESS on success, else an error code
 */
obc_error_code_t vn100SetBaudrate(uint32_t baudrate);

/**
 * @brief Set the output rate for the ASYNC outputs
 * @param outputRate The desired baudrate to be set
 * @note The default factory output rate is 40Hz. But initialized to 10Hz in initVn100()
 *        Acceptable Output rates: 1Hz, 2Hz, 4Hz, 5Hz, 10Hz, 20Hz, 25Hz, 40Hz, 50Hz, 100Hz, 200Hz
 *
 * @return OBC_ERR_CODE_SUCCESS on success, else an error code
 */
obc_error_code_t vn100SetOutputRate(uint32_t outputRateHz);

/**
 * @brief pause asyncronous outputs from the VN-100
 *
 * @return OBC_ERR_CODE_SUCCESS on success, else an error code
 */
obc_error_code_t vn100PauseAsync(void);

/**
 * @brief resume asyncronous outputs from the VN-100
 *
 * @return OBC_ERR_CODE_SUCCESS on success, else an error code
 */
obc_error_code_t vn100ResumeAsync(void);

/**
 * @brief Function begins asynchronous binary outputs on VN_100 Serial port 2 (TTL)
 *  Outputs:
 *  Yaw, Pitch, Roll
 *  Angular Rates
 *  Acceleration
 *  Magnetometer
 *  Temperature
 *  Pressure
 *
 * @return OBC_ERR_CODE_SUCCESS on success, else an error code
 */
obc_error_code_t vn100StartBinaryOutputs(void);

/**
 * @brief Stops asyncronous binary ouputs from serial port 2, this should be used in the event of changing the
 *        configuration of serial port 2 to output serial ASCII. Simply call startBinaryOutputs() to
 *        reconfigure the serial port to output binary packets.
 *
 * @return OBC_ERR_CODE_SUCCESS on success, else an error code
 */
obc_error_code_t vn100StopBinaryOutputs(void);

/**
 * @brief Reads all incoming asyncronous binary output data.
 *
 * @param parsedPacket Pointer to the packet to store data in.
 * @return OBC_ERR_CODE_SUCCESS on success, else an error code
 */
obc_error_code_t vn100ReadBinaryOutputs(vn100_binary_packet_t* parsedPacket);
