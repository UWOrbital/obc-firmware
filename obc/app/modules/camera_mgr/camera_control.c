#include "camera_control.h"

#include <spi.h>

obc_error_code_t selectCamera(camera_id_t cameraID) {
  // TODO: Validate I2C mux driver code and properly integrate with control code
  return OBC_ERR_CODE_SUCCESS;
}

// TODO: Find proper config settings and capture sequence
// Following config step is ripped straight from arduino arducam example files, unsure how
// Camera is generating images, just super under exposed, brightness super low
obc_error_code_t camConfigureSensor(void) {
  obc_error_code_t errCode;
  // Reset camera
  RETURN_IF_ERROR_CODE(ov5642Reset());
  // Setup Preview resolution
  applyCamPreviewConfig();
  vTaskDelay(pdMS_TO_TICKS(2));

  // Switch to JPEG capture
  applyCamCaptureConfig();
  // Switch to lowest JPEG resolution
  applyCamResolutionConfig();

  vTaskDelay(pdMS_TO_TICKS(1));
  // Vertical flip
  RETURN_IF_ERROR_CODE(ov5642SetVerticalFlip(true));
  // Pixel binning
  // RETURN_IF_ERROR_CODE(camWriteSensorReg16_8(0x3621, 0x10));
  // Image horizontal control
  RETURN_IF_ERROR_CODE(ov5642SetHorizontalStart(432));
  // Image compression
  RETURN_IF_ERROR_CODE(ov5642SetQuantizationScale(0x08));
  // Lens correction
  RETURN_IF_ERROR_CODE(ov5642SetLencBrvScale(0x0C));
  // Image processor setup
  RETURN_IF_ERROR_CODE(ov5642SetLencCorrection(true));

  return errCode;
}
