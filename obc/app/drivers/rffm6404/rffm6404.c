#include "gio.h"
#include "max5360.h"
#include "obc_board_config.h"
#include "obc_errors.h"
#include "obc_logging.h"

/* See Page 4 of the RFFM6404 datasheet for truth table explaining necessary pin values for each mode */

/**
 * @brief powers on the RFFE chip in receive mode
 *
 * @return obc_err_code_t - whether or not the chip was successfully turned on
 */
obc_error_code_t rffm6404ActivateRx(void) {
  obc_error_code_t errCode;
  RETURN_IF_ERROR_CODE(max5360PowerOff());
  gioSetBit(RFFM6404_TR_PIN_PORT, RFFM6404_TR_PIN_NUM, 0);
  gioSetBit(RFFM6404_EN_PIN_PORT, RFFM6404_EN_PIN_NUM, 1);
  gioSetBit(RFFM6404_BYP_PIN_PORT, RFFM6404_BYP_PIN_NUM, 0);
  return OBC_ERR_CODE_SUCCESS;
}

/**
 * @brief powers on the RFFE chip in transmit mode
 *
 * @param voltagePowerControl voltage value to use for VAPC to control power
 *
 * @return obc_err_code_t - whether or not the chip was successfully turned on
 */
obc_error_code_t rffm6404ActivateTx(float voltagePowerControl) {
  obc_error_code_t errCode;
  RETURN_IF_ERROR_CODE(max5360WriteVoltage(voltagePowerControl));
  gioSetBit(RFFM6404_TR_PIN_PORT, RFFM6404_TR_PIN_NUM, 1);
  gioSetBit(RFFM6404_EN_PIN_PORT, RFFM6404_EN_PIN_NUM, 1);
  gioSetBit(RFFM6404_BYP_PIN_PORT, RFFM6404_BYP_PIN_NUM, 0);
  return OBC_ERR_CODE_SUCCESS;
}

/**
 * @brief from receive mode, activate bypass mode to turn of LNA
 *
 * @return obc_err_code_t - whether or not the operation was successful
 */
obc_error_code_t rffm6404ActivateRecvByp(void) {
  obc_error_code_t errCode;
  RETURN_IF_ERROR_CODE(max5360PowerOff());
  gioSetBit(RFFM6404_TR_PIN_PORT, RFFM6404_TR_PIN_NUM, 0);
  gioSetBit(RFFM6404_EN_PIN_PORT, RFFM6404_EN_PIN_NUM, 1);
  gioSetBit(RFFM6404_BYP_PIN_PORT, RFFM6404_BYP_PIN_NUM, 1);
  return OBC_ERR_CODE_SUCCESS;
}

/**
 * @brief powers off the RFFE chip
 *
 * @return obc_err_code_t - whether or not the power off was successful
 */
obc_error_code_t rffm6404PowerOff(void) {
  obc_error_code_t errCode;
  RETURN_IF_ERROR_CODE(max5360PowerOff());
  gioSetBit(RFFM6404_EN_PIN_PORT, RFFM6404_EN_PIN_NUM, 0);
  return OBC_ERR_CODE_SUCCESS;
}
