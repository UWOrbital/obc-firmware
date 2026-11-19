#pragma once

#include "obc_errors.h"

/*
 *
 * State manager which reads states from app flags in flash, and decides the proper handler.
 *
 */
obc_error_code_t blStateMgr(void);
