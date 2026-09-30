#if ENABLE_TASK_STATS_COLLECTOR == 1
#include "task_stats_collector.h"
#include "obc_scheduler_config.h"
#include "obc_print.h"
#include "obc_privilege.h"
#include "obc_logging.h"

#define TASK_STATS_BUFFER_SIZE 1000U
#define UART_MUTEX_BLOCK_TIME portMAX_DELAY

static char taskTableHeaderStr[] =
    "***********************************************\r\nTask           State   Prio    Stack    "
    "Num\r\n***********************************************\r\n";

void obcTaskInitStatsCollector(void) {}

void obcTaskFunctionStatsCollector(void *pvParameters) {
  obc_error_code_t errCode;
  prvRaisePrivilege();
  while (1) {
    vTaskDelay(pdMS_TO_TICKS(15000));
    char taskStatsString[TASK_STATS_BUFFER_SIZE] = {0};
    char taskStatsBuffer[TASK_STATS_BUFFER_SIZE] = {0};

    vTaskList(taskStatsString);

    vTaskGetRunTimeStats(taskStatsBuffer);
    LOG_IF_ERROR_CODE(sciPrintText((unsigned char *)taskStatsBuffer, TASK_STATS_BUFFER_SIZE, UART_MUTEX_BLOCK_TIME));
    LOG_IF_ERROR_CODE(
        sciPrintText((unsigned char *)taskTableHeaderStr, strlen(taskTableHeaderStr), UART_MUTEX_BLOCK_TIME));
    LOG_IF_ERROR_CODE(sciPrintText((unsigned char *)taskStatsString, TASK_STATS_BUFFER_SIZE, UART_MUTEX_BLOCK_TIME));
  }
}
#endif
