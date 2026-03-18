#include "obc_gs_telemetry_unpack.h"
#include "obc_gs_telemetry_data.h"
#include "obc_gs_telemetry_id.h"
#include "data_unpack_utils.h"
#include "obc_gs_errors.h"

#include <stddef.h>
#include <stdint.h>

/* Declare all unpack functions for telemetry data */

static void unpackLM75BDTemp(const uint8_t *buffer, uint32_t *offset, telemetry_data_t *data);
static void unpackCC1120Temp(const uint8_t *buffer, uint32_t *offset, telemetry_data_t *data);
static void unpackRTCTemp(const uint8_t *buffer, uint32_t *offset, telemetry_data_t *data);
static void unpackObcState(const uint8_t *buffer, uint32_t *offset, telemetry_data_t *data);
static void unpackPong(const uint8_t *buffer, uint32_t *offset, telemetry_data_t *data);

typedef void (*telemetry_unpack_func_t)(const uint8_t *, uint32_t *, telemetry_data_t *);

static const telemetry_unpack_func_t telemUnpackFns[] = {
    [TELEM_LM75BD_TEMP] = unpackLM75BDTemp,
    [TELEM_CC1120_TEMP] = unpackCC1120Temp,
    [TELEM_RTC_TEMP] = unpackRTCTemp,
    [TELEM_OBC_STATE] = unpackObcState,
    [TELEM_PONG] = unpackPong,
};

#define NUM_UNPACK_FNS (sizeof(telemUnpackFns) / sizeof(telemUnpackFns[0]))

obc_gs_error_code_t unpackTelemetry(const uint8_t *buffer, uint32_t *offset, telemetry_data_t *data) {
  if (data == NULL || buffer == NULL || offset == NULL) {
    return OBC_GS_ERR_CODE_INVALID_ARG;
  }

  // Unpack the telemetry ID
  telemetry_data_id_t id = (telemetry_data_id_t)unpackUint8(buffer, offset);

  if (id >= NUM_UNPACK_FNS) {
    return OBC_GS_ERR_CODE_INVALID_ARG;
  }

  if (telemUnpackFns[id] == NULL) {
    return OBC_GS_ERR_CODE_INVALID_ARG;
  }

  data->id = id;

  // Unpack the telemetry timestamp
  data->timestamp = unpackUint32(buffer, offset);

  // Unpack the telemetry data
  telemUnpackFns[id](buffer, offset, data);

  return OBC_GS_ERR_CODE_SUCCESS;
}

static void unpackLM75BDTemp(const uint8_t *buffer, uint32_t *offset, telemetry_data_t *data) {
  data->obcTemp = unpackFloat(buffer, offset);
}

static void unpackCC1120Temp(const uint8_t *buffer, uint32_t *offset, telemetry_data_t *data) {
  data->obcTemp = unpackFloat(buffer, offset);
}

static void unpackRTCTemp(const uint8_t *buffer, uint32_t *offset, telemetry_data_t *data) {
  data->obcTemp = unpackFloat(buffer, offset);
}

static void unpackObcState(const uint8_t *buffer, uint32_t *offset, telemetry_data_t *data) {
  data->obcState = unpackUint8(buffer, offset);
}

static void unpackPong(const uint8_t *buffer, uint32_t *offset, telemetry_data_t *data) {
  // Nothing to unpack
}
