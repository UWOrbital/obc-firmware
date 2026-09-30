#include "alarm_handler.h"
#include "cc1120_txrx.h"
#include "gio.h"
#include "hal_stdtypes.h"
#include "obc_board_config.h"
#include "tpl5010.h"

void gioNotification(gioPORT_t *port, uint32 bit) {
  if (port == gioPORTA) {
    switch (bit) {
      case DS3232_INT_PIN:
        alarmInterruptCallback();
        break;
      // See section 3.4.1.1
      // Triggered on falling edge so TX FIFO can be written to once the signal that RTX FIFO is above
      // TXRX_INTERRUPT_THRESHOLD is deasserted to avoid a FIFO
      case CC1120_TX_THR_PKT_gioPORTA_PIN:
        txFifoReadyCallback();
        break;
      // See section 3.4.1.1
      // triggered on falling edge once TX FIFO has been completely emptied
      case CC1120_PKT_SYNC_RXTX_gioPORTA_PIN:
        txFifoEmptyCallback();
        break;
    }
  } else if (port == gioPORTB) {
    switch (bit) {
      // See section 3.4.1.1
      // Triggered on rising edge so RX FIFO can be read once the signal that RX FIFO is above TXRX_INTERRUPT_THRESHOLD
      // is asserted to avoid a FIFO overflow
      case CC1120_RX_THR_PKT_gioPORTB_PIN:
        rxFifoReadyCallback();
        break;

#ifdef OBC_REVISION_2
      case TPL5010_WAKE_PIN:
        feedHardwareWatchdog();
        break;
#endif
    }
  }
}
