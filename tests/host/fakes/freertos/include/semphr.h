/* Imported from C:\PSC\csp-rs485\tests\host (authorized subset). */
/* This import makes no assertion about upstream licensing. */
#ifndef SEMPHR_H
#define SEMPHR_H

#include <FreeRTOS.h>

typedef struct {
    BaseType_t available;
} StaticSemaphore_t;

typedef StaticSemaphore_t *SemaphoreHandle_t;

SemaphoreHandle_t xSemaphoreCreateMutexStatic(StaticSemaphore_t *buffer);
SemaphoreHandle_t xSemaphoreCreateBinaryStatic(StaticSemaphore_t *buffer);
BaseType_t xSemaphoreTake(SemaphoreHandle_t mutex, TickType_t ticks_to_wait);
BaseType_t xSemaphoreGive(SemaphoreHandle_t mutex);

#endif
