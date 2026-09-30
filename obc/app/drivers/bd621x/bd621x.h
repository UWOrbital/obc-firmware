#pragma once

#include "het.h"
#include "obc_errors.h"

/**
* @struct DC_motor_t
* @brief Struct used to hold the motor's pwm modules and which HET driver its running on
* @param hetBase: pointer to HET RAM
    hetRAM1: HET1 RAM pointer
    hetRAM2: HET2 RAM pointer
* @param finPwm: pwm driver that controls FIN (Forward pin of motor driver)
* @param rinPwm: pwm driver that controls RIN (Reverse pin of motor driver)
    pwm0: Pwm 0
    pwm1: Pwm 1
    pwm2: Pwm 2
    pwm3: Pwm 3
    pwm4: Pwm 4
    pwm5: Pwm 5
    pwm6: Pwm 6
    pwm7: Pwm 7
* @param  maxSpeed: The torque produced at 100% duty cycle. (m/s)
**/
typedef struct {
  hetRAMBASE_t* hetBase;
  uint32_t finPwm, rinPwm;
  float maxSpeed;
} DC_motor_t;

/**
 * @brief Starts the PWM channels.
 * @param motor: pointer to motor struct
 * @return Returns OBC_ERR_CODE_SUCCESS if successful, OBC_ERR_CODE_INVALID_ARG if motor is a pointer to NULL
 * or DC_motor_t struct parameters are invalid.
 **/
obc_error_code_t startMotor(const DC_motor_t* motor);

/**
* @brief Sets the motor at the specified speed and period.
* @param motor: pointer to motor struct.
* @param speed: (m/s) The signed speed value at which the motor should be run. Set to maxiumum speed for the motor
if it greater than that in magnitude. Same units as maxSpeed.
* @param period: The PWM period in us.
* @return Returns OBC_ERR_CODE_SUCCESS if successful, OBC_ERR_CODE_INVALID_ARG if motor is a pointer to NULL
 * or DC_motor_t struct parameters are invalid.
*/
obc_error_code_t setMotorSpeed(const DC_motor_t* motor, float speed, float64 period);

/**
 * @brief Sets PWM signal to 100, setting the motor to HIGH until signal is changed.
 * @param DC_Motor: pointer to motor struct.
 * @return Returns OBC_ERR_CODE_SUCCESS if successful, OBC_ERR_CODE_INVALID_ARG if motor is a pointer to NULL
 * or DC_motor_t struct parameters are invalid.
 **/
obc_error_code_t brakeMotor(const DC_motor_t* motor);

/**
 * @brief Sets the motor to idle mode.
 * @param motor: pointer to motor struct.
 * @return Returns OBC_ERR_CODE_SUCCESS if successful, OBC_ERR_CODE_INVALID_ARG if motor is a pointer to NULL
 * or DC_motor_t struct parameters are invalid.
 **/
obc_error_code_t idleMotor(const DC_motor_t* motor);
