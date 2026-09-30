#pragma once

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Compute the CRC32 of a buffer.
 *
 * @param prevCrc32 The previous CRC32 value, or 0 to start a new CRC32 computation.
 * @param buffer The buffer to compute the CRC32 of.
 * @param len The length of the buffer.
 * @return uint32_t The CRC32 of the buffer.
 */
uint32_t computeCrc32(const uint32_t prevCrc32, const uint8_t* buffer, size_t len);
