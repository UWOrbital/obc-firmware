#include "image_processing.h"

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
                                            uint16_t packetStartY) {
  if (packet == NULL || x == NULL || y == NULL || brightness == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }
  *brightness = 0;
  for (uint16_t i = 0; i < packet->height; i++) {
    for (uint16_t j = 0; j < packet->width; j++) {
      uint8_t pixel = packet->data[(i * packet->width) + j];
      if (pixel > *brightness) {
        *brightness = pixel;
        *x = j;
        *y = i + packetStartY;
      }
    }
  }
  return OBC_ERR_CODE_SUCCESS;
}
