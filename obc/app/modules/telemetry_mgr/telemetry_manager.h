#pragma once

#include <stddef.h>
#include <stdint.h>

#include "obc_errors.h"
#include "obc_gs_telemetry_data.h"

/**
 * @brief	Adds a telemetry data point to the telemetry queue
 * @param	data Pointer to the telemetry data point to add
 * @return  obc_error_code_t OBC_ERR_CODE_SUCCESS if the data was added to the queue, error code otherwise
 */
obc_error_code_t addTelemetryData(telemetry_data_t *data);

obc_error_code_t setTelemetryManagerDownlinkReady(void);
