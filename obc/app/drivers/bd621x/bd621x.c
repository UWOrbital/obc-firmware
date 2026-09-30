#include "bd621x.h"

#include <stdlib.h>

#include "het.h"
#include "obc_errors.h"
#include "obc_logging.h"

// Datasheet reference: https://fscdn.rohm.com/en/products/databook/datasheet/ic/motor/dc/bd621x-e.pdf

#define MOTOR_DRIVE_HIGH 100          // 100% duty cycle, indiicating high signal.
#define MOTOR_DRIVE_LOW 0             // 0% duty cycle, indiicating low signal.
#define MOTOR_SPEED_LOWER_BOUND 0.01  // (m/s) Arbitrarily chosen lower bound for motor speed.
#define MOTOR_MAX_FREQUENCY 100000    // (Hz) Max frequency allowed for the brushless motor,  Pg. 11 in the datasheet.
#define MOTOR_MIN_FREQUENCY 20000     // (Hz) Min frequency allowed for the brushless motor, Pg. 11 in the datasheet.

#define MOTOR_MAX_DUTY 100  // The maximum duty cycle allowed in percent.

/**
 * @brief Returns a bool indicating validity of the pwm channel.
 * @param pwm: The uint32_t value for the pwm channel, also defined as a macro in het.h
 * @return Returns bool 1 if the Pwm channel is valid, otherwise 0.
 **/
static bool isValidPwm(uint32_t pwm);

/**
 * @brief Returns a bool indicating validity of the hetRam struct pointer.
 * @param hetRam: A pointer to a hetRAMBASE_t struct, also defined as a macro in reg_het.h
 * @return Returns bool 1 if the Pwm channel is valid, otherwise 0.
 **/
static bool isValidHetBase(hetRAMBASE_t* hetRam);

/**
 * @brief Returns a bool indicating validity of the all motor parameters.
 * @param motor: Pointer to the motor struct.
 * @return Returns bool 1 if all motor parameters are valid, otherwise 0.
 **/
static bool isValidMotorParameters(const DC_motor_t* motor);

/**
 * @brief Sets the motor at the specified duty cycle and period.
 * @param motor: pointer to motor struct.
 * @param speed: (m/s) The duty cycle give as a int32_t casted downt to uint32 with considerations for sign.
 * @param period: The PWM period in us.
 * @return Returns OBC_ERR_CODE_SUCCESS if successful, OBC_ERR_CODE_INVALID_ARG if motor is a pointer to NULL
 * or DC_motor_t struct parameters are invalid.
 **/
static obc_error_code_t setMotorPwm(const DC_motor_t* motor, int32_t duty, float64 period);

obc_error_code_t startMotor(const DC_motor_t* motor) {
  if (motor == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  if (!isValidMotorParameters(motor)) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  pwmStart(motor->hetBase, motor->finPwm);
  pwmStart(motor->hetBase, motor->rinPwm);
  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t setMotorSpeed(const DC_motor_t* motor, float speed, float64 period) {
  if (motor == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  if (!isValidMotorParameters(motor)) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  if (motor->maxSpeed < MOTOR_SPEED_LOWER_BOUND) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  float64 frequency = 1 / period;
  if ((frequency < MOTOR_MIN_FREQUENCY) || (frequency > MOTOR_MAX_FREQUENCY)) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  int32_t duty = (int32_t)((speed / motor->maxSpeed) * 100);

  if (abs(duty) > MOTOR_MAX_DUTY) {
    return OBC_ERR_CODE_INVALID_ARG;
  }
  obc_error_code_t errCode;
  RETURN_IF_ERROR_CODE(setMotorPwm(motor, duty, period));
  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t brakeMotor(const DC_motor_t* motor) {
  if (motor == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  if (!isValidMotorParameters(motor)) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  pwmSetDuty(motor->hetBase, motor->finPwm, MOTOR_DRIVE_HIGH);
  pwmSetDuty(motor->hetBase, motor->rinPwm, MOTOR_DRIVE_HIGH);
  return OBC_ERR_CODE_SUCCESS;
}

obc_error_code_t idleMotor(const DC_motor_t* motor) {
  if (motor == NULL) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  if (!isValidMotorParameters(motor)) {
    return OBC_ERR_CODE_INVALID_ARG;
  }

  pwmStop(motor->hetBase, motor->finPwm);
  pwmStop(motor->hetBase, motor->rinPwm);
  return OBC_ERR_CODE_SUCCESS;
}

static obc_error_code_t setMotorPwm(const DC_motor_t* motor, int32_t duty, float64 period) {
  hetSIGNAL_t signal = {.duty = abs(duty), .period = period};

  if (duty >= 0) {
    pwmSetSignal(motor->hetBase, motor->finPwm, signal);
    pwmSetDuty(motor->hetBase, motor->rinPwm, MOTOR_DRIVE_LOW);
  } else {
    pwmSetDuty(motor->hetBase, motor->finPwm, MOTOR_DRIVE_LOW);
    pwmSetSignal(motor->hetBase, motor->rinPwm, signal);
  }
  return OBC_ERR_CODE_SUCCESS;
}

static bool isValidPwm(uint32_t pwm) { return (pwm <= pwm7); }
static bool isValidHetBase(hetRAMBASE_t* hetRam) { return (hetRam == hetRAM1 || hetRam == hetRAM2); }

static bool isValidMotorParameters(const DC_motor_t* motor) {
  return (isValidPwm(motor->finPwm) && isValidPwm(motor->rinPwm) && isValidHetBase(motor->hetBase));
}
