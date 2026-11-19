#include "obc_errors.h"
#include "test_mgr.h"

obc_error_code_t cc1120OnboardTest();

obc_error_code_t commsMgrOnboardTest();

const onboard_test_callback_t onboardTests[NUM_MODULES] = {
    [cc1120] = cc1120OnboardTest,
    [comms_mgr] = commsMgrOnboardTest,
};

// Implement test functions below

// Each function should either
// a) Send a request to the queue of a peripheral, and send a test res after its done
// b) Call the peripheral directly, and send a test res after its done.

obc_error_code_t cc1120OnboardTest() { return OBC_ERR_CODE_SUCCESS; };

obc_error_code_t commsMgrOnboardTest() { return OBC_ERR_CODE_SUCCESS; };
