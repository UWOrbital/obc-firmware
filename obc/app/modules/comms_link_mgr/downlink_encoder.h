#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "obc_errors.h"
#include "comms_manager.h"

typedef enum { DOWNLINK_TELEMETRY_FILE, DOWNLINK_DATA_BUFFER, DOWNLINK_CMD_RESPONSE } encode_event_id_t;

typedef struct {
  telemetry_data_t telemData[MAX_DOWNLINK_TELEM_BUFFER_SIZE];
  uint8_t bufferSize;
} telemetry_data_buffer_t;

typedef struct {
  encode_event_id_t eventID;
  union {
    uint32_t telemetryBatchId;
    telemetry_data_buffer_t telemetryDataBuffer;
    uint8_t cmdResponseByte;
  };
} encode_event_t;

/**
 * @brief Sends downlink data to encoding task queue
 *
 * @param queueMsg - Includes command ID, and either a telemetry batch ID or a telemetry_data_t array
 * @return obc_error_code_t - OBC_ERR_CODE_SUCCESS if the telemetry batch ID was successfully sent to the queue
 */
obc_error_code_t sendToDownlinkEncodeQueue(encode_event_t *queueMsg);
