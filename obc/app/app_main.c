#include "obc_logging.h"
#include "obc_sci_io.h"
#include "obc_i2c_io.h"
#include "obc_spi_io.h"
#include "obc_reset.h"
#include "obc_scheduler_config.h"
#include "state_mgr.h"
#include "flash.h"
#include "bl_config.h"
#include "obc_privilege.h"

#include <FreeRTOS.h>
#include <os_task.h>

#include <sys_common.h>
#include <sys_core.h>
#include <gio.h>
#include <sci.h>
#include <i2c.h>
#include <spi.h>
#include <can.h>
#include <het.h>

// Debugging
#define STATE_MGR_DEBUG_LED_GIO_PORT gioPORTB
#define STATE_MGR_DEBUG_LED_GIO_BIT 1

/* LINKER EXPORTED SYMBOLS */
extern uint32_t __ramFuncsLoadStart__;
extern uint32_t __ramFuncsSize__;

extern uint32_t __ramFuncsRunStart__;
extern uint32_t __ramFuncsRunEnd__;

// This is the stack canary. It should never be overwritten.

void *__stack_chk_guard = (void *)0xDEADBEEF;

uint32_t __stack_chk_guard_init(void);

void __stack_chk_fail(void) { resetSystem(RESET_REASON_STACK_CHECK_FAIL); }

int main(void) {
  // Run hardware initialization code
  gioInit();
  sciInit();
  i2cInit();
  spiInit();
  canInit();
  hetInit();
  _enable_interrupt_();

  // TODO implement error handling for if the flash init doesn't work.

  // Initialize bus mutexes
  initSciMutex();
  initI2CMutex();
  initSpiMutex();

  // The state_mgr is the only task running initially.
  obcSchedulerInitTask(OBC_SCHEDULER_CONFIG_ID_STATE_MGR);
  obcSchedulerCreateTask(OBC_SCHEDULER_CONFIG_ID_STATE_MGR);

  // Initialize flash
  memcpy(&__ramFuncsRunStart__, &__ramFuncsLoadStart__, (uint32_t)&__ramFuncsSize__);

  // Give privilege in the RTOS
  BaseType_t xRunningPrivileged = prvRaisePrivilege();

  _disable_interrupt_();
  flashFapiInitBank(RM46_FLASH_BANK);
  _enable_interrupt_();

  // Remove privilege from RTOS
  portRESET_PRIVILEGE(xRunningPrivileged);

  vTaskStartScheduler();
}
