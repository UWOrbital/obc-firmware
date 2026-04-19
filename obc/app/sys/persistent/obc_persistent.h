#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "obc_errors.h"

/*---------------------------------------------------------------------------*/
/* GUIDE FOR ADDING A NEW PERSISTENT SECTION:
 * 1. Add required configs
 *     - Create an ID in the obc_persist_section_id_t enum,
 *       it should be OBC_PERSIST_SECTION_ID_<MODULE_NAME>
 *     - If the number of sections is greater than 1, create a count macro for the
 *       section in the config section of the obc_persistent.h file. It should be
 *       OBC_PERSISTENT_MAX_<MODULE_NAME>_COUNT
 * 2. Add a struct for the data to be stored in the section
 *      - This struct should contain all the data that needs to be stored
 *      - The entire section is protected with a single CRC so the entire
 *        section must be read/written even if you only care about 1 field.
 *        Ex: If you store an array in the section, you must read/write the
 *        entire array even if you only want to change 1 element.
 *      - The struct name should be <module>_persist_data_t
 * 3. Add a struct for the section itself (header + data)
 *      - The struct name should be <module>_persist_t
 *      - This is the layout of the section in persistent storage
 * 4. Add the section struct (header + data) to the obc_persist_t struct
 *      - The struct element name should be <module> (e.g. obcTime)
 *      - obc_persist_t is the layout of the entire persistent storage
 *      - You can declare a buffer of identical sections in this struct.
 *        This is useful for storing arrays that need to be individually
 *        accessible. If you are doing so, use the count macro created in step 1.
 * 5. Add a config struct to the obcPersistConfig[] in the obc_persistent.c file
 *     - The sectionStartAddr should be the address of the section in FRAM, use the
 *       OBC_PERSIST_ADDR_OF macro with the section name (e.g. OBC_PERSIST_ADDR_OF(obcTime))
 *     - The sectionSize should be the size of the section in FRAM, use the sizeof() macro
 *       with the section struct name (e.g. sizeof(obc_time_persist_t))
 *     - The sectionVersion should be the current version number of obc_persist, defined by
 *       the macro OBC_PERSISTENT_VERSION
 *     - The dataSize should be the size of the data in the section, use the sizeof() macro
 *       with the data struct name (e.g. sizeof(obc_time_persist_data_t)). This is used to
 *       verify that the buffer passed to the get/set functions is large enough.
 *     - The sectionCount should be OBC_PERSISTENT_MIN_SUBINDEX (equivalent to 1) unless,
 *       the section is storing an array of identical sections. In this case,
 *       use the macro that was defined in the obc_persistent.h file under step 1.
 *---------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*/
/* Maximum sub index for each section */
#define OBC_PERSISTENT_MIN_SUBINDEX 1U
#define OBC_PERSISTENT_MAX_SUBINDEX_ALARM 24U

/* Current version of obc_persistent to store in section headers */
#define OBC_PERSISTENT_VERSION 1

/*---------------------------------------------------------------------------*/
/**
 * @brief Header struct to be placed at the start of each persistent section
 *
 */
typedef struct {
  uint32_t sectionSize;
  uint32_t sectionVersion;
  uint32_t crc32;
} obc_persist_section_header_t;

/*---------------------------------------------------------------------------*/

/* SECTION AND DATA STRUCTS */

/* obc_time module */
typedef struct {
  uint32_t unixTime;
} obc_time_persist_data_t;

typedef struct {
  obc_persist_section_header_t header;
  obc_time_persist_data_t data;
} obc_time_persist_t;

/* alarm_mgr module */

// Command ID definition for persistent
typedef enum {
  CMD_END_OF_FRAME_PERSIST = 0x00,
  CMD_EXEC_OBC_RESET_PERSIST,
  CMD_RTC_SYNC_PERSIST,
  CMD_DOWNLINK_LOGS_NEXT_PASS_PERSIST,
  CMD_MICRO_SD_FORMAT_PERSIST,
  CMD_PING_PERSIST,
  CMD_DOWNLINK_TELEM_PERSIST,
  NUM_CMD_CALLBACKS_PERSIST
} cmd_callback_id_persist_t;

// Command data structures for persistent
typedef struct {
  uint32_t unixTime;
} rtc_sync_cmd_data_persist_t;
typedef struct {
  uint8_t logLevel;
} downlink_logs_next_pass_cmd_data_persist_t;

// Command message definition for persistent
typedef struct {
  union {
    rtc_sync_cmd_data_persist_t rtcSync;
    downlink_logs_next_pass_cmd_data_persist_t downlinkLogsNextPass;
  };
  uint32_t timestamp;
  bool isTimeTagged;
  cmd_callback_id_persist_t id;
} cmd_msg_persist_t;

// Alarm handler callback definition for persistent
typedef union {
  obc_error_code_t (*defaultCallback)(void);
  obc_error_code_t (*cmdCallback)(cmd_msg_persist_t *);
} alarm_handler_callback_def_persist_t;

typedef enum {
  ALARM_TYPE_DEFAULT_PERSIST,
  ALARM_TYPE_TIME_TAGGED_CMD_PERSIST,
} alarm_type_persist_t;

typedef struct {
  uint32_t unixTime;
  alarm_type_persist_t type;
  alarm_handler_callback_def_persist_t callbackDef;
  union {
    cmd_msg_persist_t cmdMsg;
  };
} alarm_mgr_persist_data_t;

typedef struct {
  obc_persist_section_header_t header;
  alarm_mgr_persist_data_t data;
} alarm_mgr_persist_t;

/*---------------------------------------------------------------------------*/

/**
 * @brief Layout of the persistent storage
 */

typedef struct {
  obc_time_persist_t obcTime;
  alarm_mgr_persist_t alarmMgr[OBC_PERSISTENT_MAX_SUBINDEX_ALARM];
} obc_persist_t;

#define OBC_PERSIST_ADDR_OF(data) (0x0 + offsetof(obc_persist_t, data))

/*---------------------------------------------------------------------------*/
/* CONFIG */

typedef enum {
  OBC_PERSIST_SECTION_ID_OBC_TIME = 0,
  OBC_PERSIST_SECTION_ID_ALARM_MGR,

  OBC_PERSIST_SECTION_ID_COUNT  // Must always be last
} obc_persist_section_id_t;

typedef struct {
  size_t sectionStartAddr;  // Includes the header
  size_t sectionSize;       // Includes the header
  uint32_t sectionVersion;
  size_t dataSize;  // Size of the data in the section (Does not include the header)
  size_t sectionCount;
} obc_persist_config_t;

/*---------------------------------------------------------------------------*/

/**
 * @brief Get a persistent section from FRAM by the sectionId and verify its header data.
 * This function is used when the section is not an array of identical sections and is equivalent
 * to calling getPersistentDataByIndex with index = 0.
 *
 * @warning This function does not manage concurrent accesses of the same section
 *
 * @param sectionId The sectionId of the section to get
 * @param buff Buffer to store the section (header not included)
 * @param buffLen Length of the buffer (Must be at least sectionId's dataSize bytes long)
 *
 * @return obc_error_code_t OBC_ERR_CODE_SUCCESS if successful, otherwise an error code
 */
obc_error_code_t getPersistentData(obc_persist_section_id_t sectionId, void *buff, size_t buffLen);

/**
 * @brief Set a persistent section in FRAM by the sectionId and write its header data
 * This function is used when the section is not an array of identical sections and is equivalent
 * to calling setPersistentDataByIndex with index = 0.
 *
 * @warning This function does not manage concurrent accesses of the same section
 *
 * @param sectionId The sectionId of the section to set
 * @param buff Buffer containing the section data (header not included)
 * @param buffLen Length of the buffer (Must be at least sectionId's dataSize bytes long)
 *
 * @return obc_error_code_t OBC_ERR_CODE_SUCCESS if successful, otherwise an error code
 */
obc_error_code_t setPersistentData(obc_persist_section_id_t sectionId, const void *buff, size_t buffLen);

/**
 * @brief Get a persistent section from FRAM by the sectionId and index and verify its header data. This function is
 * used when the section is an array of identical sections.
 *
 * @warning This function does not manage concurrent accesses of the same section
 *
 * @param sectionId The sectionId of the section to get
 * @param index The index of the section to get
 * @param buff Buffer to store the section data (header not included)
 * @param buffLen Length of the buffer (Must be at least sectionId's dataSize bytes long)
 *
 * @return obc_error_code_t OBC_ERR_CODE_SUCCESS if successful, otherwise an error code
 */
obc_error_code_t getPersistentDataByIndex(obc_persist_section_id_t sectionId, size_t index, void *buff, size_t buffLen);

/**
 * @brief Set a persistent section in FRAM by the sectionId and index and write its header data. This function is
 * used when the section is an array of identical sections.
 *
 * @warning This function does not manage concurrent accesses of the same section
 *
 * @param sectionId The sectionId of the section to set
 * @param index The index of the section to get
 * @param buff Buffer containing the section data (header not included)
 * @param buffLen Length of the buffer (Must be at least sectionId's dataSize bytes long)
 *
 * @return obc_error_code_t OBC_ERR_CODE_SUCCESS if successful, otherwise an error code
 */
obc_error_code_t setPersistentDataByIndex(obc_persist_section_id_t sectionId, size_t index, const void *buff,
                                          size_t buffLen);

#ifdef __cplusplus
}
#endif
