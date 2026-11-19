#pragma once
#include "FreeRTOS.h"
typedef void *QueueHandle_t;
typedef struct {
  unsigned unused;
} StaticQueue_t;
QueueHandle_t xQueueCreateStatic(UBaseType_t count, UBaseType_t size, uint8_t *storage, StaticQueue_t *queue);
BaseType_t xQueueSend(QueueHandle_t queue, const void *event, TickType_t timeout);
BaseType_t xQueueSendFromISR(QueueHandle_t queue, const void *event, BaseType_t *awoken);
BaseType_t xQueueReceive(QueueHandle_t queue, void *event, TickType_t timeout);
