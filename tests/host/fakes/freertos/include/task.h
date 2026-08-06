/* Imported from C:\PSC\csp-rs485\tests\host (authorized subset). */
/* This import makes no assertion about upstream licensing. */
#ifndef TASK_H
#define TASK_H

#include <FreeRTOS.h>

typedef struct {
    uint32_t notification_bits;
    BaseType_t deleted;
} StaticTask_t;

typedef StaticTask_t *TaskHandle_t;
typedef void (*TaskFunction_t)(void *argument);

typedef enum {
    eSetBits = 0,
} eNotifyAction;

TaskHandle_t xTaskCreateStatic(TaskFunction_t task, const char *name,
    uint32_t stack_depth, void *argument, UBaseType_t priority,
    StackType_t *stack_buffer, StaticTask_t *task_buffer);
BaseType_t xTaskNotifyWait(uint32_t bits_to_clear_on_entry,
    uint32_t bits_to_clear_on_exit, uint32_t *notification_value,
    TickType_t ticks_to_wait);
BaseType_t xTaskNotifyFromISR(TaskHandle_t task, uint32_t value,
    eNotifyAction action, BaseType_t *higher_priority_task_woken);
BaseType_t xTaskNotify(TaskHandle_t task, uint32_t value,
    eNotifyAction action);
void vTaskDelete(TaskHandle_t task);
void vTaskSetTimeOutState(TimeOut_t *timeout);
BaseType_t xTaskCheckForTimeOut(
    TimeOut_t *timeout,
    TickType_t *ticks_to_wait);

#endif
