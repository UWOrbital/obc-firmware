#include <FreeRTOS.h>
#include <os_task.h>

#include "console.h"
#include "posix_example.h"

xTaskHandle dummyTaskHandle;

void main(void) {
  // Initialize the mutex used to protect the console.
  initConsole();

  // Create a dummy task.
  BaseType_t xReturned;
  xReturned = xTaskCreate(vDummyTask, "dummyTaskA", 1024, NULL, 1, &dummyTaskHandle);

  if (xReturned == pdPASS) {
    // Start the scheduler if the task was created successfully.
    vTaskStartScheduler();
  } else {
    printConsole("Failed to create dummy task\n");
  }
}
