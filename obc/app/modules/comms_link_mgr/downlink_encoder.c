#include "downlink_encoder.h"
#include "cc1120_txrx.h"
#include "obc_board_config.h"
#include "obc_gs_ax25.h"
#include "obc_gs_commands_response.h"
#include "obc_gs_fec.h"

#include "obc_gs_telemetry_pack.h"
#include "obc_sci_io.h"
#include "telemetry_fs_utils.h"
#include "telemetry_manager.h"

#include "comms_manager.h"
#include "obc_errors.h"
#include "obc_logging.h"
#include "obc_reliance_fs.h"
#include "obc_scheduler_config.h"

#include <FreeRTOS.h>
#include <os_portmacro.h>
#include <os_queue.h>
#include <os_task.h>

#include <gio.h>
#include <stdint.h>
#include <sys_common.h>

#define COMMS_TELEM_ENCODE_QUEUE_LENGTH 2U
#define COMMS_TELEM_ENCODE_QUEUE_ITEM_SIZE sizeof(encode_event_t)  // Size of the telemetry batch ID
#define COMMS_TELEM_ENCODE_QUEUE_RX_WAIT_PERIOD portMAX_DELAY
#define COMMS_TELEM_ENCODE_QUEUE_TX_WAIT_PERIOD portMAX_DELAY

static QueueHandle_t telemEncodeQueueHandle = NULL;
static StaticQueue_t telemEncodeQueue;
static uint8_t telemEncodeQueueStack[COMMS_TELEM_ENCODE_QUEUE_LENGTH * COMMS_TELEM_ENCODE_QUEUE_ITEM_SIZE];

/**
 * @brief Sends data from a telemetry buffer to the CC1120 transmit queue
 *
 * @param telemetryDataBuffer - Pointer to the telemetry_data_t buffer
 * @param numTelemetryData - Number of telemetry data points in the buffer
 * @return obc_error_code_t
 */
static obc_error_code_t sendTelemetryBuffer(telemetry_data_t *telemetryDataBuffer, uint8_t numTelemetryData);

/**
 * @brief Reads chunks of telemetry out of a file and sends it into the CC1120
 * transmit queue
 *
 * @param telemetryBatchId - ID of the telemetry batch to send
 * @return obc_error_code_t - OBC_ERR_CODE_SUCCESS if all telemetry data was
 * sent successfully
 */
static obc_error_code_t sendTelemetryFile(uint32_t telemetryBatchId);

/**
 * @brief Get the file descriptor object for the telemetry file
 *
 * @param telemetryBatchId - ID of the telemetry batch to send
 * @param fd - Pointer to the file descriptor to be set
 * @return obc_error_code_t
 */
static obc_error_code_t getFileDescriptor(uint32_t telemetryBatchId, int32_t *fd);

/**
 * @brief Sends a byte array, applying FEC and AX.25 framing
 *
 * @param sendBuffer - An array of bytes to send of size 223B
 * @return obc_error_code_t
 */
static obc_error_code_t sendPacket(uint8_t *sendBuffer);

/**
 * @brief Either sends a single piece of telemetry or packs it into the current
 * telemetry packet
 *
 * @param singleTelem - A single piece of telemetry_data_t
 * @param telemPacket - A complete telemetry packet of size 223B
 * @param telemPacketOffset - Pointer to number of bytes already filled in
 * telemPacket
 * @return obc_error_code_t
 */
static obc_error_code_t sendOrPackNextTelemetry(telemetry_data_t *singleTelem, packed_telem_packet_t *telemPacket,
                                                size_t *telemPacketOffset);

void obcTaskInitCommsDownlinkEncoder(void) {
  if (telemEncodeQueueHandle == NULL) {
    telemEncodeQueueHandle = xQueueCreateStatic(COMMS_TELEM_ENCODE_QUEUE_LENGTH, COMMS_TELEM_ENCODE_QUEUE_ITEM_SIZE,
                                                telemEncodeQueueStack, &telemEncodeQueue);
  }
}

/**
 * @brief Sends downlink data to encoding task queue
 *
 * @param queueMsg - Includes command ID, and either a telemetry batch ID or a
 * telemetry_data_t array
 * @return obc_error_code_t - OBC_ERR_CODE_SUCCESS if the telemetry batch ID was
 * successfully sent to the queue
 */
obc_error_code_t sendToDownlinkEncodeQueue(encode_event_t *queueMsg) {
  ASSERT(telemEncodeQueueHandle != NULL);

  if (xQueueSend(telemEncodeQueueHandle, (void *)queueMsg, COMMS_TELEM_ENCODE_QUEUE_TX_WAIT_PERIOD) == pdPASS) {
    return OBC_ERR_CODE_SUCCESS;
  }

  return OBC_ERR_CODE_QUEUE_FULL;
}

void obcTaskFunctionCommsDownlinkEncoder(void *pvParameters) {
  obc_error_code_t errCode;
  uint8_t cmdResBuffer[RS_DECODED_SIZE] = {0};
  uint8_t cmdResBytesRecieved = 0;

  while (1) {
    encode_event_t queueMsg;

    // Wait for a telemetry downlink event
    if (xQueueReceive(telemEncodeQueueHandle, &queueMsg, COMMS_TELEM_ENCODE_QUEUE_RX_WAIT_PERIOD) != pdPASS) {
      // TODO: Handle this if necessary
      continue;
    }
    transmit_event_t transmitEvent = {0};
    switch (queueMsg.eventID) {
      case DOWNLINK_TELEMETRY_FILE:
        setCurrentLinkDestCallSign(GROUND_STATION_CALLSIGN, CALLSIGN_LENGTH, DEFAULT_SSID);
        LOG_IF_ERROR_CODE(sendTelemetryFile(queueMsg.telemetryBatchId));
        transmitEvent.eventID = END_DOWNLINK;
        LOG_IF_ERROR_CODE(sendToCC1120TransmitQueue(&transmitEvent));
        break;
      case DOWNLINK_DATA_BUFFER:
        setCurrentLinkDestCallSign(GROUND_STATION_CALLSIGN, CALLSIGN_LENGTH, DEFAULT_SSID);
        LOG_IF_ERROR_CODE(
            sendTelemetryBuffer(queueMsg.telemetryDataBuffer.telemData, queueMsg.telemetryDataBuffer.bufferSize));
        transmitEvent.eventID = END_DOWNLINK;
        LOG_IF_ERROR_CODE(sendToCC1120TransmitQueue(&transmitEvent));
        break;
      case DOWNLINK_CMD_RESPONSE:
        if (cmdResBytesRecieved < RS_DECODED_SIZE) {
          cmdResBuffer[cmdResBytesRecieved] = queueMsg.cmdResponseByte;
          cmdResBytesRecieved++;
        }

        if (cmdResBytesRecieved >= RS_DECODED_SIZE) {
          // Downlink this response if the array gets filled up
          setCurrentLinkDestCallSign(GROUND_STATION_CALLSIGN, CALLSIGN_LENGTH, DEFAULT_SSID);
          LOG_IF_ERROR_CODE(sendPacket(cmdResBuffer));
          transmitEvent.eventID = END_DOWNLINK;
          LOG_IF_ERROR_CODE(sendToCC1120TransmitQueue(&transmitEvent));
          cmdResBytesRecieved = 0;
          memset(cmdResBuffer, 0, RS_DECODED_SIZE);  // Not required, but done to avoid any edge cases
        }
        break;
      default:
        LOG_ERROR_CODE(OBC_ERR_CODE_INVALID_ARG);
        break;
    }
  }
}

/**
 * @brief Sends data from a telemetry buffer to the CC1120 transmit queue
 *
 * @param telemetryDataBuffer - Pointer to the telemetry_data_t buffer
 * @param numTelemetryData - Number of telemetry data points in the buffer
 * @return obc_error_code_t
 */
static obc_error_code_t sendTelemetryBuffer(telemetry_data_t *telemetryDataBuffer, uint8_t numTelemetryData) {
  obc_error_code_t errCode;

  if (telemetryDataBuffer == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  if (numTelemetryData == 0) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  // Initialize important variables related to packing and queueing the
  // telemetry to be sen
  packed_telem_packet_t telemPacket = {0};  // Holds 223B of "raw" telemetry data.
                                            // Zero initialized because telem IDs of 0 are ignored at the ground
                                            // station
  size_t telemPacketOffset = 0;             // Number of bytes filled in telemPacket

  // Loop through all telemetry data in the buffer
  for (uint8_t i = 0; i < numTelemetryData; i++) {
    RETURN_IF_ERROR_CODE(sendOrPackNextTelemetry(telemetryDataBuffer + i, &telemPacket, &telemPacketOffset));
  }

  // Send the last packet if it is not empty
  if (telemPacketOffset == 0) return OBC_ERR_CODE_SUCCESS;

  RETURN_IF_ERROR_CODE(sendPacket(telemPacket.data));

  return OBC_ERR_CODE_SUCCESS;
}

/**
 * @brief Reads chunks of telemetry out of a file and sends it into the CC1120
 * transmit queue
 *
 * @param telemetryBatchId - ID of the telemetry batch to send
 * @return obc_error_code_t - OBC_ERR_CODE_SUCCESS if all telemetry data was
 * sent successfully
 */
static obc_error_code_t sendTelemetryFile(uint32_t telemetryBatchId) {
  obc_error_code_t errCode;

  // Get the file descriptor for the telemetry file
  int32_t fd;
  RETURN_IF_ERROR_CODE(getFileDescriptor(telemetryBatchId, &fd));

  // Initialize important variables related to packing and queueing the
  // telemetry to be sent
  telemetry_data_t singleTelem;  // Holds a single piece of telemetry from getNextTelemetry()

  packed_telem_packet_t telemPacket = {0};  // Holds 223B of "raw" telemetry data.
                                            // Zero initialized because telem IDs of 0 are ignored at the ground
                                            // station
  size_t telemPacketOffset = 0;             // Number of bytes filled in telemPacket

  // Read a single piece of telemetry from the file
  while ((errCode = readNextTelemetryFromFile(fd, &singleTelem)) == OBC_ERR_CODE_SUCCESS) {
    errCode = sendOrPackNextTelemetry(&singleTelem, &telemPacket, &telemPacketOffset);
    if (errCode != OBC_ERR_CODE_SUCCESS) {
      LOG_ERROR_CODE(errCode);
      RETURN_IF_ERROR_CODE(closeTelemetryFile(fd));
      return errCode;
    }
  }

  if (errCode == OBC_ERR_CODE_REACHED_EOF) {
    LOG_DEBUG("Reached end of telemetry file");
    errCode = OBC_ERR_CODE_SUCCESS;
  } else {
    LOG_ERROR_CODE(errCode);  // If the error wasn't an EOF error, return
    RETURN_IF_ERROR_CODE(closeTelemetryFile(fd));
    return errCode;
  }

  // If there's no data left to send, return
  if (telemPacketOffset == 0) return OBC_ERR_CODE_SUCCESS;

  errCode = sendPacket(telemPacket.data);
  if (errCode != OBC_ERR_CODE_SUCCESS) {
    LOG_ERROR_CODE(errCode);
    RETURN_IF_ERROR_CODE(closeTelemetryFile(fd));
    return errCode;
  }

  // Close telemetry file
  RETURN_IF_ERROR_CODE(closeTelemetryFile(fd));

  return OBC_ERR_CODE_SUCCESS;
}

/**
 * @brief Get the file descriptor object for the telemetry file
 *
 * @param telemetryBatchId - ID of the telemetry batch to send
 * @param fd - Pointer to the file descriptor to be set
 * @return obc_error_code_t
 */
static obc_error_code_t getFileDescriptor(uint32_t telemetryBatchId, int32_t *fd) {
  obc_error_code_t errCode;

  if (fd == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  // Open the telemetry file
  RETURN_IF_ERROR_CODE(openTelemetryFileRO(telemetryBatchId, fd));

  // Print the telemetry file size for debugging
  size_t fileSize;
  errCode = getFileSize(*fd, &fileSize);
  if (errCode != OBC_ERR_CODE_SUCCESS) {
    LOG_ERROR_CODE(errCode);
    closeTelemetryFile(*fd);
    return errCode;
  }
  LOG_DEBUG("Sending telemetry file");

  // Print telemetry file name
  char fileName[TELEMETRY_FILE_PATH_MAX_LENGTH] = {0};
  errCode = constructTelemetryFilePath(telemetryBatchId, fileName, TELEMETRY_FILE_PATH_MAX_LENGTH);
  if (errCode != OBC_ERR_CODE_SUCCESS) {
    LOG_ERROR_CODE(errCode);
    closeTelemetryFile(*fd);
    return errCode;
  }

  return OBC_ERR_CODE_SUCCESS;
}

/**
 * @brief Either sends a single piece of telemetry or packs it into the current
 * telemetry packet
 *
 * @param singleTelem - A single piece of telemetry_data_t
 * @param telemPacket - A complete telemetry packet of size 223B
 * @param telemPacketOffset - Pointer to number of bytes already filled in
 * telemPacket
 * @return obc_error_code_t
 */
static obc_error_code_t sendOrPackNextTelemetry(telemetry_data_t *singleTelem, packed_telem_packet_t *telemPacket,
                                                size_t *telemPacketOffset) {
  obc_error_code_t errCode;

  uint8_t packedSingleTelem[MAX_TELEMETRY_DATA_SIZE];  // Holds a serialized
                                                       // version of the current
                                                       // piece of telemetry
  uint32_t packedSingleTelemSize = 0;                  // Size of the packed single telemetry

  // Pack the single telemetry into a uint8_t array
  if (packTelemetry(singleTelem, packedSingleTelem, sizeof(packedSingleTelem) / sizeof(uint8_t),
                    &packedSingleTelemSize) != OBC_GS_ERR_CODE_SUCCESS) {
    return OBC_ERR_CODE_FAILED_PACK;
  }

  // If the single telemetry is too large to continue adding to the telemPacket,
  // send the telemPacket
  if ((*telemPacketOffset) + packedSingleTelemSize > PACKED_TELEM_PACKET_SIZE) {
    RETURN_IF_ERROR_CODE(sendPacket(telemPacket->data));
    // Reset the packedTelem struct and offset
    *telemPacket = (packed_telem_packet_t){0};
    *telemPacketOffset = 0;
  }

  // Copy the telemetry data into the packedTelem struct
  memcpy(telemPacket->data + (*telemPacketOffset), packedSingleTelem, packedSingleTelemSize);
  *telemPacketOffset += packedSingleTelemSize;

  return OBC_ERR_CODE_SUCCESS;
}

/**
 * @brief Sends a byte array, applying FEC and AX.25 framing
 *
 * @param sendBuffer - An array of bytes to send of size 223B
 * @return obc_error_code_t
 */
static obc_error_code_t sendPacket(uint8_t *sendBuffer) {
  packed_rs_packet_t fecPkt = {0};  // Holds a 255B RS packet
  unstuffed_ax25_i_frame_t unstuffedAx25Pkt = {0};
  transmit_event_t transmitEvent = {.eventID = DOWNLINK_PACKET};

  obc_gs_error_code_t interfaceErr;
  // Apply Reed Solomon FEC
  interfaceErr = rsEncode(sendBuffer, &fecPkt);
  if (interfaceErr != OBC_GS_ERR_CODE_SUCCESS) {
    return OBC_ERR_CODE_FEC_ENCODE_FAILURE;
  }

  // Perform AX.25 framing
  interfaceErr = ax25SendIFrame((uint8_t *)&fecPkt.data, RS_ENCODED_SIZE, &unstuffedAx25Pkt);
  if (interfaceErr != OBC_GS_ERR_CODE_SUCCESS) {
    return OBC_ERR_CODE_AX25_ENCODE_FAILURE;
  }

  interfaceErr = ax25Stuff(unstuffedAx25Pkt.data, unstuffedAx25Pkt.length, transmitEvent.ax25Pkt.data,
                           &transmitEvent.ax25Pkt.length);
  if (interfaceErr != OBC_GS_ERR_CODE_SUCCESS) {
    return OBC_ERR_CODE_AX25_BIT_STUFF_FAILURE;
  }

  // Send into CC1120 transmit queue
  obc_error_code_t errCode;
  RETURN_IF_ERROR_CODE(sendToCC1120TransmitQueue(&transmitEvent));

  return OBC_ERR_CODE_SUCCESS;
}
