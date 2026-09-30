#pragma once
#include "FreeRTOS.h"
#include <stddef.h>
typedef struct test_queue *QueueHandle_t;
QueueHandle_t xQueueCreate(unsigned, size_t);
BaseType_t xQueueSend(QueueHandle_t, const void *, uint32_t);
BaseType_t xQueueReceive(QueueHandle_t, void *, uint32_t);
void vQueueDelete(QueueHandle_t);
