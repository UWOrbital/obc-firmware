#include "fm25v20a.h"

#include "obc_errors.h"
#include "obc_assert.h"
#include "obc_persistent.h"

#include <stdint.h>
#include <string.h>

// Callback pointers make the persistent layout larger on 64-bit test hosts.
#define MOCK_FRAM_MAX_SIZE sizeof(obc_persist_t)
static uint8_t memory[MOCK_FRAM_MAX_SIZE] = {0};

STATIC_ASSERT(MOCK_FRAM_MAX_SIZE <= FRAM_MAX_ADDRESS, "Mock FRAM exceeds available FRAM space");

obc_error_code_t framRead(uint32_t addr, uint8_t *buffer, size_t nBytes) {
  if (buffer == NULL) return OBC_ERR_CODE_INVALID_ARG;

  if (addr + nBytes > MOCK_FRAM_MAX_SIZE) return OBC_ERR_CODE_BUFF_OVERFLOW;

  memcpy(buffer, memory + addr, nBytes);
  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t framWrite(uint32_t addr, const uint8_t *data, size_t nBytes) {
  if (data == NULL) return OBC_ERR_CODE_INVALID_ARG;

  if (addr + nBytes > MOCK_FRAM_MAX_SIZE) return OBC_ERR_CODE_BUFF_OVERFLOW;

  memcpy(memory + addr, data, nBytes);
  return OBC_ERR_CODE_SUCCESS;
}
