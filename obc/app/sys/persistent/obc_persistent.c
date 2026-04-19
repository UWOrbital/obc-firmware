#include "obc_persistent.h"

#include "fm25v20a.h"
#include "obc_assert.h"
#include "obc_errors.h"
#include "obc_logging.h"
#include "obc_crc.h"

#include <string.h>

/* Config */
static const obc_persist_config_t obcPersistConfig[] = {
    [OBC_PERSIST_SECTION_ID_OBC_TIME] = {.sectionStartAddr = OBC_PERSIST_ADDR_OF(obcTime),
                                         .sectionSize = sizeof(obc_time_persist_t),
                                         .sectionVersion = OBC_PERSISTENT_VERSION,
                                         .dataSize = sizeof(obc_time_persist_data_t),
                                         .sectionCount = OBC_PERSISTENT_MIN_SUBINDEX},
    [OBC_PERSIST_SECTION_ID_ALARM_MGR] = {.sectionStartAddr = OBC_PERSIST_ADDR_OF(alarmMgr),
                                          .sectionSize = sizeof(alarm_mgr_persist_t),
                                          .sectionVersion = OBC_PERSISTENT_VERSION,
                                          .dataSize = sizeof(alarm_mgr_persist_data_t),
                                          .sectionCount = OBC_PERSISTENT_MAX_SUBINDEX_ALARM},
};

STATIC_ASSERT(sizeof(obc_persist_t) <= FRAM_MAX_ADDRESS, "obc_persist_t exceeds available FRAM space");

/* Private function declarations */

/**
 * @brief Gets the config based on the section id
 * @return obc_persist_config_t* Pointer to the config or NULL if sectionId is invalid
 */
static const obc_persist_config_t *getOBCPersistConfig(obc_persist_section_id_t sectionId);

/* Public function definitions */

obc_error_code_t getPersistentData(obc_persist_section_id_t sectionId, void *buff, size_t buffLen) {
  return getPersistentDataByIndex(sectionId, 0, buff, buffLen);
}

obc_error_code_t setPersistentData(obc_persist_section_id_t sectionId, const void *buff, size_t buffLen) {
  return setPersistentDataByIndex(sectionId, 0, buff, buffLen);
}

obc_error_code_t getPersistentDataByIndex(obc_persist_section_id_t sectionId, size_t index, void *buff,
                                          size_t buffLen) {
  obc_error_code_t errCode;
  uint8_t *buffPtr = (uint8_t *)buff;
  if (buffPtr == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  // Validate against config
  const obc_persist_config_t *config = getOBCPersistConfig(sectionId);
  if (config == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }
  if (index >= config->sectionCount) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  // Buffer will only hold the data not the header
  if (buffLen < config->dataSize) {
    return OBC_ERR_CODE_BUFF_TOO_SMALL;
  }
  uint32_t sectionStartAddrByIndex = config->sectionStartAddr + index * config->sectionSize;

  // Reads header, sectionSize holds the header as well
  obc_persist_section_header_t header = {0};
  RETURN_IF_ERROR_CODE(framRead(sectionStartAddrByIndex, (uint8_t *)&header, sizeof(obc_persist_section_header_t)));

  if (header.sectionSize != config->sectionSize) {
    return OBC_ERR_CODE_PERSISTENT_CORRUPTED;
  }

  // Use the dataSize to prevent accidentally reading data past the section
  RETURN_IF_ERROR_CODE(
      framRead(sectionStartAddrByIndex + sizeof(obc_persist_section_header_t), buffPtr, config->dataSize));

  // Compute CRC32 of data and check it
  uint32_t crc32 = computeCrc32(0, buffPtr, config->dataSize);
  if (header.crc32 != crc32) {
    return OBC_ERR_CODE_PERSISTENT_CORRUPTED;
  }

  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t setPersistentDataByIndex(obc_persist_section_id_t sectionId, size_t index, const void *buff,
                                          size_t buffLen) {
  obc_error_code_t errCode;
  const uint8_t *buffPtr = (const uint8_t *)buff;

  if (buffPtr == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  // Validate against config
  const obc_persist_config_t *config = getOBCPersistConfig(sectionId);
  if (config == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }
  if (index >= config->sectionCount) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  // Buffer will only hold the data not the header
  if (buffLen < config->dataSize) {
    return OBC_ERR_CODE_BUFF_TOO_SMALL;
  }

  // Create header
  obc_persist_section_header_t header = {0};
  header.sectionSize = config->sectionSize;
  header.sectionVersion = config->sectionVersion;
  // Use the dataSize as that's what we're writing
  header.crc32 = computeCrc32(0, buffPtr, config->dataSize);

  // Write header and data
  uint32_t sectionStartAddrByIndex = config->sectionStartAddr + index * config->sectionSize;
  RETURN_IF_ERROR_CODE(framWrite(sectionStartAddrByIndex, (uint8_t *)&header, sizeof(obc_persist_section_header_t)));

  // Use the dataSize to prevent accidentally overriding data past the section
  RETURN_IF_ERROR_CODE(
      framWrite(sectionStartAddrByIndex + sizeof(obc_persist_section_header_t), buffPtr, config->dataSize));

  return OBC_ERR_CODE_SUCCESS;
}

/* Private functions */

static const obc_persist_config_t *getOBCPersistConfig(obc_persist_section_id_t sectionId) {
  if (sectionId >= OBC_PERSIST_SECTION_ID_COUNT) return NULL;

  return &obcPersistConfig[sectionId];
}
