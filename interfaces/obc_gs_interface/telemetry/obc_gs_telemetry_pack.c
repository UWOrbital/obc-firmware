#include "obc_gs_telemetry_pack.h"
#include "obc_gs_telemetry_data.h"
#include "obc_gs_telemetry_id.h"
#include "data_pack_utils.h"
#include "obc_gs_errors.h"

#include <stddef.h>
#include <stdint.h>

/* Declare all pack functions for telemetry data */
static void packLM75BDTemp(const telemetry_data_t *data, uint8_t *buffer, uint32_t *offset);
static void packCC1120Temp(const telemetry_data_t *data, uint8_t *buffer, uint32_t *offset);
static void packRTCTemp(const telemetry_data_t *data, uint8_t *buffer, uint32_t *offset);
static void packObcState(const telemetry_data_t *data, uint8_t *buffer, uint32_t *offset);
static void packPong(const telemetry_data_t *data, uint8_t *buffer, uint32_t *offset);

typedef void (*telemetry_pack_func_t)(const telemetry_data_t *, uint8_t *, uint32_t *);

static const telemetry_pack_func_t telemPackFns[] = {
    [TELEM_LM75BD_TEMP] = packLM75BDTemp,
    [TELEM_CC1120_TEMP] = packCC1120Temp,
    [TELEM_RTC_TEMP] = packRTCTemp,
    [TELEM_OBC_STATE] = packObcState,
    [TELEM_PONG] = packPong,
};

obc_gs_error_code_t packTelemetry(const telemetry_data_t *data, uint8_t *buffer, size_t len, uint32_t *numPacked) {
  if (data == NULL || buffer == NULL || numPacked == NULL) {
    return OBC_GS_ERR_CODE_INVALID_ARG;
  }

  if (len < MAX_TELEMETRY_DATA_SIZE) {
    return OBC_GS_ERR_CODE_BUFF_TOO_SMALL;
  }

  if (telemPackFns[data->id] == NULL) {
    return OBC_GS_ERR_CODE_INVALID_ARG;
  }

  uint32_t offset = 0;

  // Pack the telemetry ID
  packUint8(data->id, buffer, &offset);

  // Pack the timestamp
  packUint32(data->timestamp, buffer, &offset);

  // Pack the telemetry parameters
  telemPackFns[data->id](data, buffer, &offset);

  *numPacked = offset;

  return OBC_GS_ERR_CODE_SUCCESS;
}

static void packLM75BDTemp(const telemetry_data_t *data, uint8_t *buffer, uint32_t *offset) {
  packFloat(data->obcTemp, buffer, offset);
}

static void packCC1120Temp(const telemetry_data_t *data, uint8_t *buffer, uint32_t *offset) {
  packFloat(data->obcTemp, buffer, offset);
}

static void packRTCTemp(const telemetry_data_t *data, uint8_t *buffer, uint32_t *offset) {
  packFloat(data->obcTemp, buffer, offset);
}

static void packObcState(const telemetry_data_t *data, uint8_t *buffer, uint32_t *offset) {
  packUint8(data->obcState, buffer, offset);
}

static void packPong(const telemetry_data_t *data, uint8_t *buffer, uint32_t *offset) {
  // Nothing to pack
}
