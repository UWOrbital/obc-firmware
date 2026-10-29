#include "obc_sci_io.h"
#include "obc_print.h"
#include "obc_spi_io.h"
#include "obc_privilege.h"
#include "sdc_diskio.h"

#include <FreeRTOS.h>
#include <os_task.h>
#include <sys_common.h>
#include <sys_dma.h>
#include <gio.h>
#include <sci.h>
#include <spi.h>
#include <stdint.h>

#define CPU_WORKER_STACK_SIZE 256U
#define SDC_WORKER_STACK_SIZE 1024U

/* Priority Hierarchy:
 * SDC_Worker (Priority 2) > CPU_Worker (Priority 1) > Idle Task (Priority 0)
 */
#define CPU_WORKER_PRIORITY (tskIDLE_PRIORITY + 1U)
#define SDC_WORKER_PRIORITY (tskIDLE_PRIORITY + 2U)

/* Sector 0 contains the MBR/Partition Table. It is safe for read benchmarking
 * because it is guaranteed to exist on any partitioned SD card.
 * DO NOT write to Sector 0, as doing so will destroy the partition table!
 */
#define SD_BENCHMARK_SECTOR 0U
#define SD_BENCHMARK_ITERATIONS 500U
#define TASK_STATS_BUFFER_SIZE 512U

static StackType_t cpuWorkerStack[CPU_WORKER_STACK_SIZE];
static StaticTask_t cpuWorkerBuffer;

static StackType_t sdcWorkerStack[SDC_WORKER_STACK_SIZE];
static StaticTask_t sdcWorkerBuffer;

/**
 * @brief CPU Worker Task: Runs whenever SDC_Worker is blocked waiting on DMA.
 * FreeRTOS's built-in runtime statistics directly track the time spent here.
 */
void vCpuWorkerTask(void *pvParameters) {
  (void)pvParameters;
  while (1) {
    /* literally just eats cpu, idk whether this loop gets optimized out or smth*/
  }
}

/**
 * @brief SDC Worker Task: Executes SD card sector reads via DMA, then
 * queries and prints FreeRTOS runtime statistics.
 */
void vSdcWorkerTask(void *pvParameters) {
  (void)pvParameters;
  prvRaisePrivilege();

  sciPrintf("\r\n========================================\r\n");
  sciPrintf("  SD Card DMA & CPU Free Time Benchmark \r\n");
  sciPrintf("========================================\r\n");

  /* Initialize the SD Card (configures SPI DMA and hardware) */
  DSTATUS stat = disk_initialize(0);
  if (stat & STA_NOINIT) {
    sciPrintf("ERROR: Failed to initialize SD card (DSTATUS: 0x%02X)\r\n", stat);
    while (1) {
      vTaskDelay(portMAX_DELAY);
    }
  }
  sciPrintf("SD Card initialized successfully.\r\n");

  /* Execute SD sector reads via DMA */
  uint8_t sectorBuffer[512] = {0};
  sciPrintf("Reading %u sectors via DMA...\r\n", SD_BENCHMARK_ITERATIONS);

  TickType_t startTime = xTaskGetTickCount();

  // read sector 0 since its just the MBR partition info
  for (uint32_t i = 0; i < SD_BENCHMARK_ITERATIONS; i++) {
    DRESULT res = disk_read(0, sectorBuffer, SD_BENCHMARK_SECTOR, 1);
    if (res != RES_OK) {
      sciPrintf("SD read failed at sector %u (result: %d)\r\n", SD_BENCHMARK_SECTOR, res);
      break;
    }
  }

  /* TODO: Add disk_write benchmark once safe write sector is verified.
  DOUBLE CHECK WHAT THIS CONFLICTS WITH AND HOW ITS MEANT TO BE USED IN THE FUTURE
  IDK WHAT THE FILESYSTEM WILL BE USED FOR SO ASK THIS
   */

  TickType_t durationMs = pdTICKS_TO_MS(xTaskGetTickCount() - startTime);

  /* Query FreeRTOS native runtime statistics */
  char statsBuffer[TASK_STATS_BUFFER_SIZE] = {0};
  vTaskGetRunTimeStats(statsBuffer);

  sciPrintf("\r\n--- Benchmark Completed in %u ms ---\r\n", durationMs);
  sciPrintf("\r\n================ Task Run-Time Statistics ================\r\n");
  sciPrintf("Task            Abs Time (RTI Ticks)   %% CPU Time\r\n");
  sciPrintf("----------------------------------------------------------\r\n");
  sciPrintf("%s", statsBuffer);
  sciPrintf("==========================================================\r\n");
  sciPrintf("Note: 'CPU_Worker' %% CPU indicates the spare processing time\r\n");
  sciPrintf("      available for other tasks while DMA handled the transfer.\r\n\r\n");

  /* Park the task when finished */
  while (1) {
    vTaskDelay(portMAX_DELAY);
  }
}

int main(void) {
  /* Initialize MCU peripherals */
  gioInit();
  sciInit();
  spiInit();
  dmaEnable();

  _enable_interrupt_();

  /* Initialize communication mutexes and SCI printing */
  initSciPrint();
  initSpiMutex();

  /* Create tasks statically */
  xTaskCreateStatic(vCpuWorkerTask, "CPU_Worker", CPU_WORKER_STACK_SIZE, NULL, CPU_WORKER_PRIORITY, cpuWorkerStack,
                    &cpuWorkerBuffer);

  xTaskCreateStatic(vSdcWorkerTask, "SDC_Worker", SDC_WORKER_STACK_SIZE, NULL, SDC_WORKER_PRIORITY, sdcWorkerStack,
                    &sdcWorkerBuffer);

  /* Start FreeRTOS scheduler */
  vTaskStartScheduler();

  while (1);

  return 0;
}
