#include "test_lm75bd.h"

#include "lm75bd.h"
#include "obc_print.h"
#include "obc_sci_io.h"

#define EXPECTED_TEMP_RANGE1 10.0f
#define EXPECTED_TEMP_RANGE2 30.0f
#define EXPECTED_THYST 75.0f
#define EXPECTED_TOS 125.0f

#define LM75BD_CONF_BUFF_SIZE 1U

void testLm75bd(void) {
  sciPrintf("Testing temp sensor...\r\n");

  obc_error_code_t errCode;
  lm75bd_config_t config;
  config.devAddr = LM75BD_OBC_I2C_ADDR;

  float temp = 0.0f;
  float tHyst = 0.0f;
  float tOS = 0.0f;

  static int testState = 0;  // # of times test has been run
  bool configTest = true;

  if (testState == 0) {
    errCode = readConfigLM75BD(&config);

    if (errCode != OBC_ERR_CODE_SUCCESS) {
      configTest = false;
      sciPrintf("Config Read Testing error: %d\r\n", errCode);
    }

    if (config.osPolarity != LM75BD_OS_POL_ACTIVE_LOW) {
      configTest = false;
      sciPrintf("Config testing error: read OS_POL value does not match default of 0 (LOW), instead: %d\r\n",
                config.osPolarity);
    }

    if (config.osOperationMode != LM75BD_OS_OP_MODE_COMP) {
      configTest = false;
      sciPrintf("Config testing error: read OS_OP_MODE value does not match default of 0 (COMP), instead: %d\r\n",
                config.osOperationMode);
    }

    if (config.devOperationMode != LM75BD_DEV_OP_MODE_NORMAL) {
      configTest = false;
      sciPrintf("Config testing error: read DEV_OP_MODE value does not match default of 0 (NORM), instead: %d\r\n",
                config.devOperationMode);
    }
  }

  if (configTest) {
    sciPrintf("Config test passed \r\n");
  }

  errCode = readTempLM75BD(LM75BD_OBC_I2C_ADDR, &temp);

  sciPrintf("Current temperature: %f\r\n", temp);
  if (errCode != OBC_ERR_CODE_SUCCESS) sciPrintf("Temp Read Testing error: %d\r\n", errCode);

  if (temp < EXPECTED_TEMP_RANGE1)
    sciPrintf("Testing error: temperature falls below expected range of 10.0 - 30.0 degrees celsius\r\n");

  else if (temp > EXPECTED_TEMP_RANGE2)
    sciPrintf("Testing error: temperature is above expected range of 10.0 - 30.0 degrees celsius\r\n");

  else
    sciPrintf("Testing success: temperature is within the expected range of 10.0 - 30.0 degrees celsius\r\n");

  errCode = readThystLM75BD(LM75BD_OBC_I2C_ADDR, &tHyst);
  if (errCode != OBC_ERR_CODE_SUCCESS) sciPrintf("Thyst Read Testing error: %d\r\n", errCode);

  if (tHyst != EXPECTED_THYST)
    sciPrintf("Testing error: read tHyst value %d does not equal expected value of 75.0 degrees celsius\r\n", tHyst);

  else
    sciPrintf("Testing success: read tHyst value equals the default tHyst value of 75.0 degrees\r\n");

  errCode = readTosLM75BD(LM75BD_OBC_I2C_ADDR, &tOS);
  if (errCode != OBC_ERR_CODE_SUCCESS) sciPrintf("tOS Read Testing error: %d\r\n", errCode);

  if (tOS != EXPECTED_TOS)
    sciPrintf("Testing error: read tOS value %f does not equal expected value of 125.0 degrees celsius\r\n", tOS);

  else
    sciPrintf("Testing success: read tOS value equals the default tOS value of 125.0 degrees\r\n");

  testState++;
}
