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
  // Vertical flip. Note this is a no-op in practice: startImageCapture() re-applies the JPEG
  // table (0x3818 = 0xc8) before every capture, so the flip bit is cleared again before any
  // frame is grabbed. Every frame captured so far has been un-flipped.
  RETURN_IF_ERROR_CODE(ov5642SetVerticalFlip(true));
  // Pixel binning
  // RETURN_IF_ERROR_CODE(camWriteSensorReg16_8(0x3621, 0x10));
  // Image horizontal control
  RETURN_IF_ERROR_CODE(ov5642SetHorizontalStart(432));
  // Image compression
  RETURN_IF_ERROR_CODE(ov5642SetQuantizationScale(0x08));
  // Lens correction: disabled. ov5642SetLencBrvScale() targeted 0x3800/0x3801 (timing HS)
  // instead of 0x5888/0x5889, so this call was clobbering the horizontal start set just above.
  // The function is fixed now, but leaving the call out keeps the sensor state byte-identical
  // to previously captured frames while the colour fault is being isolated.
  // RETURN_IF_ERROR_CODE(ov5642SetLencBrvScale(0x0C));
  // Image processor setup
  RETURN_IF_ERROR_CODE(ov5642SetLencCorrection(true));

  // Let AEC and AWB converge before the first capture. The sensor free-runs once configured;
  // the arduchip only gates what reaches the FIFO. At ~3.75 fps in this mode this is ~11 frames.
  vTaskDelay(pdMS_TO_TICKS(3000));

  return errCode;
}
