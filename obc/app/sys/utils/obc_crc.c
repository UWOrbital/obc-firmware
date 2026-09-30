#include "obc_crc.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// redutils.h requires the standard types above; keep it after those headers.
// clang-format off
#include <redutils.h>
// clang-format on

uint32_t computeCrc32(const uint32_t prevCrc32, const uint8_t* buffer, size_t len) {
  return RedCrc32Update(prevCrc32, buffer, len);
}
