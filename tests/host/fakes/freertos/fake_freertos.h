/* Imported from C:\PSC\csp-rs485\tests\host (authorized subset). */
/* This import makes no assertion about upstream licensing. */
#ifndef CSP_RS485_FAKE_FREERTOS_H
#define CSP_RS485_FAKE_FREERTOS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    size_t static_task_create_calls;
    size_t static_stream_create_calls;
    size_t static_mutex_create_calls;
    size_t static_binary_semaphore_create_calls;
    size_t dynamic_allocator_calls;
    size_t mutex_take_calls;
    size_t mutex_give_calls;
    size_t binary_semaphore_take_calls;
    size_t binary_semaphore_give_calls;
    size_t binary_semaphore_wait_calls;
    size_t yield_calls;
    size_t task_notify_calls;
    size_t notify_from_isr_calls;
    size_t stream_send_from_isr_calls;
    size_t self_delete_calls;
    size_t non_null_delete_calls;
    size_t stale_delete_calls;
    size_t critical_exit_hook_calls;
    size_t task_create_hook_calls;
    size_t notify_preemption_block_calls;
    size_t task_block_calls;
    size_t mutex_wait_calls;
    size_t notify_wait_calls;
    size_t task_critical_enter_calls;
    size_t task_critical_exit_calls;
    size_t task_critical_nesting;
    size_t task_critical_max_nesting;
    uint32_t last_wait_ticks;
} fake_freertos_observations_t;

typedef void (*fake_freertos_hook_t)(void);

void fake_freertos_reset(void);
void fake_freertos_clear_observations(void);
void fake_freertos_set_isr_task_woken(bool task_woken);
void fake_freertos_get_observations(fake_freertos_observations_t *observations);
void fake_freertos_set_next_wait_hook(
    fake_freertos_hook_t hook,
    uint32_t elapsed_ticks);
void fake_freertos_set_critical_exit_hook(fake_freertos_hook_t hook);
void fake_freertos_set_critical_exit_hook_after(
    size_t exit_count,
    fake_freertos_hook_t hook);
void fake_freertos_set_delete_hook(fake_freertos_hook_t hook);
void fake_freertos_set_next_stream_receive_hook(fake_freertos_hook_t hook);
void fake_freertos_set_empty_stream_receive_hook(fake_freertos_hook_t hook);
void fake_freertos_set_task_create_hook(fake_freertos_hook_t hook);
void fake_freertos_set_task_notify_runs_task(bool runs_task);
void fake_freertos_set_mutex_wait_hook(fake_freertos_hook_t hook);
bool fake_freertos_mutex_is_available(void);
bool fake_freertos_run_task_until_blocked(void);
bool fake_freertos_task_in_critical(void);

#endif
