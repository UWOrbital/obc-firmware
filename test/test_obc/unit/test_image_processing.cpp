#include "image_processing.h"

#include "obc_errors.h"
#include <gtest/gtest.h>

#include <stdint.h>
#include <stdlib.h>

TEST(TestObcImageProcessing, findBrightestPixelInPacket) {
  uint8_t data[640 * 480] = {0};
  image_t image = {.width = 640, .height = 480, .data = data};

  image_t topPacket = {640, 240, image.data};
  image_t bottomPacket = {640, 240, image.data + 640 * 240};

  // set image data to random values < 254, set two pixels to 254 and 255
  srand(123456789);
  for (uint32_t i = 0; i < image.width * image.height; i++) {
    image.data[i] = rand() % 254;
  }
  image.data[20 + 40 * image.width] = 254;
  image.data[560 + 400 * image.width] = 255;

  /**
   * we expect the brightest pixel in the top packet to be at (20, 40) with brightness 254
   * and the brightest pixel in the bottom packet to be at (560, 400) with brightness 255
   * overall, the brightest pixel should be at (560, 400) with brightness 255
   */
  uint16_t brightestX = 0;
  uint16_t brightestY = 0;
  uint8_t brightness = 0;
  ASSERT_EQ(findBrightestPixelInPacket(&topPacket, &brightestX, &brightestY, &brightness, 0), OBC_ERR_CODE_SUCCESS);
  EXPECT_EQ(brightestX, 20);
  EXPECT_EQ(brightestY, 40);
  EXPECT_EQ(brightness, 254);
  ASSERT_EQ(findBrightestPixelInPacket(&bottomPacket, &brightestX, &brightestY, &brightness, 240),
            OBC_ERR_CODE_SUCCESS);
  EXPECT_EQ(brightestX, 560);
  EXPECT_EQ(brightestY, 400);
  EXPECT_EQ(brightness, 255);

  // all pixels are off, so the brightest pixel should be at (0, 0) with brightness 0
  memset(image.data, 0, image.width * image.height);
  brightestX = 0;
  brightestY = 0;
  brightness = 0;
  ASSERT_EQ(findBrightestPixelInPacket(&image, &brightestX, &brightestY, &brightness, 0), OBC_ERR_CODE_SUCCESS);
  EXPECT_EQ(brightestX, 0);
  EXPECT_EQ(brightestY, 0);
  EXPECT_EQ(brightness, 0);

  // all pixels are on, so the brightest pixel should be at (0, 0) with brightness 255
  memset(image.data, 255, image.width * image.height);
  brightestX = 0;
  brightestY = 0;
  brightness = 0;
  ASSERT_EQ(findBrightestPixelInPacket(&image, &brightestX, &brightestY, &brightness, 0), OBC_ERR_CODE_SUCCESS);
  EXPECT_EQ(brightestX, 0);
  EXPECT_EQ(brightestY, 0);
  EXPECT_EQ(brightness, 255);

  // should return with OBC_ERR_CODE_INVALID_ARG if any of the arguments are null
  EXPECT_EQ(findBrightestPixelInPacket(nullptr, &brightestX, &brightestY, &brightness, 0), OBC_ERR_CODE_INVALID_ARG);
  EXPECT_EQ(findBrightestPixelInPacket(&image, nullptr, &brightestY, &brightness, 0), OBC_ERR_CODE_INVALID_ARG);
  EXPECT_EQ(findBrightestPixelInPacket(&image, &brightestX, nullptr, &brightness, 0), OBC_ERR_CODE_INVALID_ARG);
  EXPECT_EQ(findBrightestPixelInPacket(&image, &brightestX, &brightestY, nullptr, 0), OBC_ERR_CODE_INVALID_ARG);
}
