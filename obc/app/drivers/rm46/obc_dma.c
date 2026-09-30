#include "obc_dma.h"

#include <sys_dma.h>

#include "obc_spi_dma.h"

void dmaGroupANotification(dmaInterrupt_t inttype, uint32 channel) {
  switch (inttype) {
    case FTC:
      break;
    case LFS:
      break;
    case BTC:
      switch (channel) {
        case DMA_SPI_1_RX_CHANNEL:
          dmaSpi1FinishedCallback();
          break;
        case DMA_SPI_3_RX_CHANNEL:
          dmaSpi3FinishedCallback();
          break;
      }
    case HBC:
  }
}
