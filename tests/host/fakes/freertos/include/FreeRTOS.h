/* Imported from C:\PSC\csp-rs485\tests\host (authorized subset). */
/* This import makes no assertion about upstream licensing. */
#ifndef FREERTOS_H
#define FREERTOS_H

#include <stdint.h>

typedef int BaseType_t;
typedef unsigned int UBaseType_t;
typedef uint32_t TickType_t;
typedef uint32_t StackType_t;

typedef struct {
    TickType_t time_on_entering;
} TimeOut_t;

#define pdFALSE ((BaseType_t) 0)
#define pdTRUE ((BaseType_t) 1)
#define pdPASS pdTRUE
#define pdFAIL pdFALSE
#define portMAX_DELAY UINT32_MAX
#define configTICK_RATE_HZ 100U
#define configMAX_PRIORITIES 8U
#define pdMS_TO_TICKS(milliseconds) \
    ((TickType_t) ((((TickType_t) (milliseconds)) * configTICK_RATE_HZ) \
        / UINT32_C(1000)))

void fake_freertos_yield_from_isr(BaseType_t task_woken);
void fake_freertos_task_enter_critical(void);
void fake_freertos_task_exit_critical(void);
UBaseType_t fake_freertos_task_enter_critical_from_isr(void);
void fake_freertos_task_exit_critical_from_isr(UBaseType_t saved_state);

#define portYIELD_FROM_ISR(task_woken) fake_freertos_yield_from_isr(task_woken)
#define taskENTER_CRITICAL() fake_freertos_task_enter_critical()
#define taskEXIT_CRITICAL() fake_freertos_task_exit_critical()
#define taskENTER_CRITICAL_FROM_ISR() fake_freertos_task_enter_critical_from_isr()
#define taskEXIT_CRITICAL_FROM_ISR(saved_state) \
    fake_freertos_task_exit_critical_from_isr(saved_state)

#endif
