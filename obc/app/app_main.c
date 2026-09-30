#include <FreeRTOS.h>
#include <can.h>
#include <gio.h>
#include <het.h>
#include <i2c.h>
#include <os_task.h>
#include <sci.h>
#include <spi.h>
#include <sys_common.h>
#include <sys_core.h>

#include "obc_i2c_io.h"
#include "obc_logging.h"
#include "obc_reset.h"
#include "obc_scheduler_config.h"
#include "obc_sci_io.h"
#include "obc_spi_io.h"
#include "state_mgr.h"

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

  // Initialize bus mutexes
  initSciMutex();
  initI2CMutex();
  initSpiMutex();

  // The state_mgr is the only task running initially.
  obcSchedulerInitTask(OBC_SCHEDULER_CONFIG_ID_STATE_MGR);
  obcSchedulerCreateTask(OBC_SCHEDULER_CONFIG_ID_STATE_MGR);

  vTaskStartScheduler();
}
