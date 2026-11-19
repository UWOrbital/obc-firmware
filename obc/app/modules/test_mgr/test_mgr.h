#include "obc_errors.h"

#define TEST_RES_QUEUE_LENGTH 25UL
#define TEST_RES_QUEUE_ITEM_SIZE sizeof(test_res_t)  // Replace this with the correct data type

typedef obc_error_code_t (*onboard_test_callback_t)();

typedef enum {
  cc1120,
  comms_mgr,
  NUM_MODULES,
} module_t;

typedef struct {
  module_t moduleId;
  obc_error_code_t errCode;
} test_res_t;
