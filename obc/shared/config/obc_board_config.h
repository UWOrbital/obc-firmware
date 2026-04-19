#include <spi.h>
#include <sci.h>
#include <gio.h>

/* Board macros for registers, ports, CS pins, data formats, etc. */

#ifdef RM46_LAUNCHPAD
// Serial config
#define UART_PRINT_REG scilinREG
#define UART_READ_REG scilinREG
#define UART_VN100_REG sciREG
#define UART_BL_REG scilinREG

// Fram SPI config
#define FRAM_spiREG spiREG3
#define FRAM_spiPORT spiPORT3
#define FRAM_CS 1UL
#define FRAM_spiFMT SPI_FMT_1

// SD Card SPI config
#define SDC_SPI_PORT spiPORT3
#define SDC_SPI_REG spiREG3
#define SDC_SPI_CS 0UL
#define SDC_SPI_DATA_FORMAT SPI_FMT_0

// Camera SPI configs
#define CAM_SPI_PORT spiPORT3
#define CAM_SPI_REG spiREG3
#define CAM_SPI_DATA_FORMAT SPI_FMT_2
// SPI3 CS[2] and CS[3] are used as I2C pins
// Reuse Some CS pins
#define CAM_CS_1 1UL
#define CAM_CS_2 2UL

// State Manager DEBUG LED
#define STATE_MGR_DEBUG_LED_GIO_PORT gioPORTB
#define STATE_MGR_DEBUG_LED_GIO_BIT 1

// Comms pin numbers for ISR
#define CC1120_RX_THR_PKT_gioPORTB_PIN 3U     // cc1120 GIO0
#define CC1120_PKT_SYNC_RXTX_gioPORTA_PIN 6U  // cc1120 GIO2
#define CC1120_TX_THR_PKT_gioPORTA_PIN 7U     // cc1120 GIO3
#define CC1120_SYNC_EVENT_PIN CC1120_PKT_SYNC_RXTX_gioPORTA_PIN

#define DS3232_INT_PORT gioPORTA
#define DS3232_INT_PIN 0U

// RFFM6404 pin numbers
#define RFFM6404_TR_PIN_NUM 6U
#define RFFM6404_EN_PIN_NUM 7U
#define RFFM6404_BYP_PIN_NUM 2U

// RFFM6404 pin ports
#define RFFM6404_TR_PIN_PORT gioPORTA
#define RFFM6404_EN_PIN_PORT gioPORTA
#define RFFM6404_BYP_PIN_PORT gioPORTB

#elif defined(OBC_REVISION_1)
// Serial config
#define UART_PRINT_REG sciREG
#define UART_READ_REG sciREG
#define UART_VN100_REG scilinREG
#define UART_BL_REG sciREG

// Fram SPI config
#define FRAM_spiREG spiREG1
#define FRAM_spiPORT spiPORT1
#define FRAM_CS 1UL
#define FRAM_spiFMT SPI_FMT_0

// SD Card SPI config
#define SDC_SPI_PORT spiPORT1
#define SDC_SPI_REG spiREG1
#define SDC_SPI_CS 0UL
#define SDC_SPI_DATA_FORMAT SPI_FMT_0

// Camera SPI configs
#define CAM_SPI_PORT spiPORT3
#define CAM_SPI_REG spiREG3
#define CAM_SPI_DATA_FORMAT SPI_FMT_2
#define CAM_CS_1 1UL
#define CAM_CS_2 2UL

// State Manager DEBUG LED
#define STATE_MGR_DEBUG_LED_GIO_PORT gioPORTA
#define STATE_MGR_DEBUG_LED_GIO_BIT 5

#define DS3232_INT_PORT gioPORTA
#define DS3232_INT_PIN 0U

// Comms pin numbers for ISR
#define CC1120_RX_THR_PKT_gioPORTB_PIN 3U     // cc1120 GIO0
#define CC1120_PKT_SYNC_RXTX_gioPORTA_PIN 6U  // cc1120 GIO2
#define CC1120_TX_THR_PKT_gioPORTA_PIN 7U     // cc1120 GIO3
#define CC1120_SYNC_EVENT_PIN CC1120_PKT_SYNC_RXTX_gioPORTA_PIN

// RFFM6404 pin numbers
#define RFFM6404_TR_PIN_NUM 6U
#define RFFM6404_EN_PIN_NUM 7U
#define RFFM6404_BYP_PIN_NUM 2U

// RFFM6404 pin ports
#define RFFM6404_TR_PIN_PORT gioPORTA
#define RFFM6404_EN_PIN_PORT gioPORTA
#define RFFM6404_BYP_PIN_PORT gioPORTB

#elif defined(OBC_REVISION_2)
// Serial config
#define UART_PRINT_REG sciREG
#define UART_READ_REG sciREG
#define UART_VN100_REG scilinREG
#define UART_BL_REG sciREG

// Fram SPI config
#define FRAM_spiREG spiREG5
#define FRAM_spiPORT spiPORT5
#define FRAM_CS 0UL
#define FRAM_spiFMT SPI_FMT_0

// SD Card SPI config
#define SDC_SPI_PORT spiPORT3
#define SDC_SPI_REG spiREG3
#define SDC_SPI_CS 0UL
#define SDC_SPI_DATA_FORMAT SPI_FMT_0

// Camera SPI configs
#define CAM_SPI_PORT spiPORT1
#define CAM_SPI_REG spiREG1
#define CAM_SPI_DATA_FORMAT SPI_FMT_2
#define CAM_CS_1 0UL
#define CAM_CS_2 1UL

// DS3232 GPIO config
#define DS3232_INT_PORT gioPORTA
#define DS3232_INT_PIN 0U

// CC1120 GPIO and HET config
#define CC1120_RX_THR_PKT_gioPORTB_PIN 3U     // cc1120 GIO0
#define CC1120_PKT_SYNC_RXTX_gioPORTA_PIN 6U  // cc1120 GIO2
#define CC1120_TX_THR_PKT_gioPORTA_PIN 7U     // cc1120 GIO3
#define CC1120_SYNC_EVENT_PIN CC1120_PKT_SYNC_RXTX_gioPORTA_PIN

// RFFM6404 pin numbers
#define RFFM6404_TR_PIN_NUM 6U
#define RFFM6404_EN_PIN_NUM 7U
#define RFFM6404_BYP_PIN_NUM 2U

// RFFM6404 pin ports
#define RFFM6404_TR_PIN_PORT gioPORTA
#define RFFM6404_EN_PIN_PORT gioPORTA
#define RFFM6404_BYP_PIN_PORT gioPORTB

// TPL5010 pin numbers
#define TPL5010_WAKE_PIN 0
#define TPL5010_DONE_PIN 1
#define TPL5010_GIO_PORT gioPORTB
#else
#error Board configuration not defined
#endif
