#pragma once

#include "obc_errors.h"

#include <stddef.h>
#include <stdint.h>

/**
 * @brief A struct to store an image
 * @param width The width of the image
 * @param height The height of the image
 * @param data The image data
 */
typedef struct image_t {
  uint16_t width;
  uint16_t height;
  uint8_t *data;
} image_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Find the brightest pixel in an image packet
 * @param packet The image packet to search through
 * @param x The x coordinate of the brightest pixel
 * @param y The y coordinate of the brightest pixel
 * @param brightness The brightness of the brightest pixel
 * @param packetStartY The y coordinate of the top left corner of the packet, the width of the packet is assumed to be
 *                     the width of the image
 */
obc_error_code_t findBrightestPixelInPacket(image_t *packet, uint16_t *x, uint16_t *y, uint8_t *brightness,
                                            uint16_t packetStartY);

#ifdef __cplusplus
}
#endif
