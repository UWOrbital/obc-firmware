#include "bl_utils.h"
#include "bl_uart.h"
#include "obc_errors.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "obc_logging.h"
#include "bl_config.h"
#include "bl_errors.h"
#include "bl_time.h"
#include "flash.h"
#include "flash_errors.h"
#if defined(DEBUG) && !defined(OBC_REVISION_2)
#include <gio.h>
#endif

/* LINKER EXPORTED SYMBOLS */
extern uint32_t __ramFuncsLoadStart__;
extern uint32_t __ramFuncsSize__;

extern uint32_t __ramFuncsRunStart__;
extern uint32_t __ramFuncsRunEnd__;

/* DEFINES */
// These values were chosen so that the UART transfers and flash writes are quick, but don't
// use too much RAM
#define BL_ECC_FIX_CHUNK_SIZE 128U  // Bytes
#define LAST_SECTOR_START_ADDR blFlashSectorStartAddr(15U)
#define WAIT_FOREVER UINT32_MAX
#define MAX_PACKET_SIZE 223
#define EXTENDED_APP_JUMP_TIMEOUT 2000
#define DEFAULT_APP_JUMP_TIMEOUT 2000
#define LED_DELAY_MS 500

static uint8_t recvBuffer[MAX_PACKET_SIZE] = {0};

/* PUBLIC FUNCTIONS */
int main(void) {
  obc_error_code_t errCode = OBC_ERR_CODE_SUCCESS;

  blUartInit();
  blInitTick();
  gioInit();

  // F021 API and the functions that use it must be executed from RAM since they
  // can't execute from the same flash bank being modified
  memcpy(&__ramFuncsRunStart__, &__ramFuncsLoadStart__, (uint32_t)&__ramFuncsSize__);

  flash_error_code_t interfaceErr = flashFapiInitBank(RM46_FLASH_BANK);

  if (interfaceErr != FLASH_ERR_CODE_SUCCESS) {
    char blUartWriteBuffer[BL_MAX_MSG_SIZE] = {0};
    int32_t blUartWriteBufferLen =
        snprintf(blUartWriteBuffer, BL_MAX_MSG_SIZE, "Failed to init flash, BL error code: %d\r\n", errCode);
    if (blUartWriteBufferLen < 0) {
      blUartWriteBytes(strlen("Error with processing message buffer length\r\n"),
                       (uint8_t *)"Error with processing message buffer length\r\n");
    } else {
      blUartWriteBytes(blUartWriteBufferLen, (uint8_t *)blUartWriteBuffer);
    }
  }

  uint32_t jumpToAppTimeout = blGetCurrentTick() + DEFAULT_APP_JUMP_TIMEOUT;
  uint32_t ledTimeout = blGetCurrentTick() + LED_DELAY_MS;
  while (1) {
    if (blGetCurrentTick() > ledTimeout) {
#if defined(DEBUG) && !defined(OBC_REVISION_2)
      gioToggleBit(STATE_MGR_DEBUG_LED_GIO_PORT, STATE_MGR_DEBUG_LED_GIO_BIT);
#endif
      ledTimeout = blGetCurrentTick() + LED_DELAY_MS;
    }

    if (blUartReadBytes(recvBuffer, MAX_PACKET_SIZE, 100) == OBC_ERR_CODE_SUCCESS) {
      LOG_IF_ERROR_CODE(blRunCommand(recvBuffer));
      jumpToAppTimeout = blGetCurrentTick() + EXTENDED_APP_JUMP_TIMEOUT;
    }

    if (blGetCurrentTick() > jumpToAppTimeout) {
      LOG_IF_ERROR_CODE(blJumpToApp());
    }
  }
  memset(recvBuffer, 0, MAX_PACKET_SIZE);
}
