/* Imported from C:\PSC\csp-rs485\tests\host (authorized subset). */
/* This import makes no assertion about upstream licensing. */
#include "fake_freertos.h"

#include <FreeRTOS.h>
#include <semphr.h>
#include <stream_buffer.h>
#include <task.h>

#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <string.h>

static fake_freertos_observations_t observations;
static TaskHandle_t current_task;
static bool isr_task_woken;
static TaskFunction_t current_task_function;
static fake_freertos_hook_t next_wait_hook;
static TickType_t next_wait_elapsed_ticks;
static fake_freertos_hook_t critical_exit_hook;
static size_t critical_exit_hook_after;
static fake_freertos_hook_t delete_hook;
static fake_freertos_hook_t next_stream_receive_hook;
static fake_freertos_hook_t empty_stream_receive_hook;
static fake_freertos_hook_t task_create_hook;
static fake_freertos_hook_t mutex_wait_hook;
static SemaphoreHandle_t current_mutex;
static SemaphoreHandle_t current_binary_semaphore;
static TickType_t fake_ticks;
static UBaseType_t critical_nesting;
static bool task_execution_active;
static bool task_notify_runs_task;
static jmp_buf task_run_jump;

enum {
    FAKE_TASK_RUN_BLOCKED = 1,
    FAKE_TASK_RUN_DELETED = 2,
};

void fake_freertos_reset(void)
{
    memset(&observations, 0, sizeof(observations));
    current_task = NULL;
    isr_task_woken = false;
    current_task_function = NULL;
    next_wait_hook = NULL;
    next_wait_elapsed_ticks = 0U;
    critical_exit_hook = NULL;
    critical_exit_hook_after = 0U;
    delete_hook = NULL;
    next_stream_receive_hook = NULL;
    empty_stream_receive_hook = NULL;
    task_create_hook = NULL;
    mutex_wait_hook = NULL;
    current_mutex = NULL;
    current_binary_semaphore = NULL;
    fake_ticks = 0U;
    critical_nesting = 0U;
    task_execution_active = false;
    task_notify_runs_task = false;
}

void fake_freertos_clear_observations(void)
{
    memset(&observations, 0, sizeof(observations));
}

void fake_freertos_set_isr_task_woken(bool task_woken)
{
    isr_task_woken = task_woken;
}

void fake_freertos_get_observations(fake_freertos_observations_t *result)
{
    if (result != NULL) {
        *result = observations;
    }
}

void fake_freertos_set_next_wait_hook(
    fake_freertos_hook_t hook,
    uint32_t elapsed_ticks)
{
    next_wait_hook = hook;
    next_wait_elapsed_ticks = elapsed_ticks;
}

void fake_freertos_set_critical_exit_hook(fake_freertos_hook_t hook)
{
    critical_exit_hook = hook;
}

void fake_freertos_set_critical_exit_hook_after(
    size_t exit_count,
    fake_freertos_hook_t hook)
{
    critical_exit_hook_after = exit_count;
    critical_exit_hook = hook;
}

void fake_freertos_set_delete_hook(fake_freertos_hook_t hook)
{
    delete_hook = hook;
}

void fake_freertos_set_next_stream_receive_hook(fake_freertos_hook_t hook)
{
    next_stream_receive_hook = hook;
}

void fake_freertos_set_empty_stream_receive_hook(fake_freertos_hook_t hook)
{
    empty_stream_receive_hook = hook;
}

void fake_freertos_set_task_create_hook(fake_freertos_hook_t hook)
{
    task_create_hook = hook;
}

void fake_freertos_set_task_notify_runs_task(bool runs_task)
{
    task_notify_runs_task = runs_task;
}

void fake_freertos_set_mutex_wait_hook(fake_freertos_hook_t hook)
{
    mutex_wait_hook = hook;
}

bool fake_freertos_mutex_is_available(void)
{
    return (current_mutex != NULL)
        && (current_mutex->available != pdFALSE);
}

bool fake_freertos_run_task_until_blocked(void)
{
    if (current_task_function == NULL) {
        return false;
    }

    task_execution_active = true;
    const int run_result = setjmp(task_run_jump);
    if (run_result == 0) {
        current_task_function(NULL);
        task_execution_active = false;
        return false;
    }

    task_execution_active = false;
    return run_result == FAKE_TASK_RUN_BLOCKED;
}

void fake_freertos_yield_from_isr(BaseType_t task_woken)
{
    if (task_woken != pdFALSE) {
        ++observations.yield_calls;
    }
}

void fake_freertos_task_enter_critical(void)
{
    ++critical_nesting;
}

void fake_freertos_task_exit_critical(void)
{
    if (critical_nesting > 0U) {
        --critical_nesting;
    }
    if ((critical_nesting == 0U) && (critical_exit_hook != NULL)
        && (critical_exit_hook_after > 0U)) {
        --critical_exit_hook_after;
    }
    if ((critical_nesting == 0U) && (critical_exit_hook != NULL)
        && (critical_exit_hook_after == 0U)) {
        fake_freertos_hook_t hook = critical_exit_hook;
        critical_exit_hook = NULL;
        ++observations.critical_exit_hook_calls;
        hook();
    }
}

UBaseType_t fake_freertos_task_enter_critical_from_isr(void)
{
    const UBaseType_t saved_state = critical_nesting;
    ++critical_nesting;
    return saved_state;
}

void fake_freertos_task_exit_critical_from_isr(UBaseType_t saved_state)
{
    critical_nesting = saved_state;
}

TaskHandle_t xTaskCreateStatic(TaskFunction_t task, const char *name,
    uint32_t stack_depth, void *argument, UBaseType_t priority,
    StackType_t *stack_buffer, StaticTask_t *task_buffer)
{
    (void) task;
    (void) name;
    (void) stack_depth;
    (void) argument;
    (void) priority;
    (void) stack_buffer;
    ++observations.static_task_create_calls;
    if (task_buffer == NULL) {
        return NULL;
    }
    if ((task_buffer == current_task)
        && (task_buffer->deleted == pdFALSE)) {
        return NULL;
    }
    memset(task_buffer, 0, sizeof(*task_buffer));
    current_task = task_buffer;
    current_task_function = task;
    if (task_create_hook != NULL) {
        fake_freertos_hook_t hook = task_create_hook;
        task_create_hook = NULL;
        ++observations.task_create_hook_calls;
        hook();
    }
    return task_buffer;
}

BaseType_t xTaskNotifyWait(uint32_t bits_to_clear_on_entry,
    uint32_t bits_to_clear_on_exit, uint32_t *notification_value,
    TickType_t ticks_to_wait)
{
    ++observations.notify_wait_calls;
    observations.last_wait_ticks = ticks_to_wait;
    if (current_task == NULL) {
        return pdFALSE;
    }
    current_task->notification_bits &= ~bits_to_clear_on_entry;
    const uint32_t value = current_task->notification_bits;
    if (notification_value != NULL) {
        *notification_value = value;
    }
    current_task->notification_bits &= ~bits_to_clear_on_exit;
    if (value != 0U) {
        return pdTRUE;
    }

    if (next_wait_hook != NULL) {
        fake_freertos_hook_t hook = next_wait_hook;
        TickType_t elapsed_ticks = next_wait_elapsed_ticks;
        next_wait_hook = NULL;
        next_wait_elapsed_ticks = 0U;
        if (elapsed_ticks > ticks_to_wait) {
            elapsed_ticks = ticks_to_wait;
        }
        fake_ticks += elapsed_ticks;
        hook();
        const uint32_t hooked_value = current_task->notification_bits;
        if (notification_value != NULL) {
            *notification_value = hooked_value;
        }
        current_task->notification_bits &= ~bits_to_clear_on_exit;
        return (hooked_value == 0U) ? pdFALSE : pdTRUE;
    }

    if ((ticks_to_wait == portMAX_DELAY) && task_execution_active) {
        ++observations.task_block_calls;
        longjmp(task_run_jump, FAKE_TASK_RUN_BLOCKED);
    }

    if (ticks_to_wait != portMAX_DELAY) {
        fake_ticks += ticks_to_wait;
    }
    return pdFALSE;
}

BaseType_t xTaskNotifyFromISR(TaskHandle_t task, uint32_t value,
    eNotifyAction action, BaseType_t *higher_priority_task_woken)
{
    (void) action;
    ++observations.notify_from_isr_calls;
    if (task == NULL) {
        return pdFALSE;
    }
    task->notification_bits |= value;
    if (higher_priority_task_woken != NULL) {
        *higher_priority_task_woken = isr_task_woken ? pdTRUE : pdFALSE;
    }
    return pdTRUE;
}

BaseType_t xTaskNotify(TaskHandle_t task, uint32_t value,
    eNotifyAction action)
{
    (void) action;
    ++observations.task_notify_calls;
    if (task == NULL) {
        return pdFALSE;
    }
    task->notification_bits |= value;
    if (task_notify_runs_task && fake_freertos_run_task_until_blocked()) {
        ++observations.notify_preemption_block_calls;
    }
    return pdTRUE;
}

void vTaskDelete(TaskHandle_t task)
{
    if ((task == NULL) && (current_task != NULL)) {
        ++observations.self_delete_calls;
        current_task->deleted = pdTRUE;
        if (task_execution_active) {
            longjmp(task_run_jump, FAKE_TASK_RUN_DELETED);
        }
        return;
    }

    ++observations.non_null_delete_calls;
    if ((task != NULL) && (task->deleted != pdFALSE)) {
        ++observations.stale_delete_calls;
    }
    if (delete_hook != NULL) {
        fake_freertos_hook_t hook = delete_hook;
        delete_hook = NULL;
        hook();
    }
    if (task != NULL) {
        task->deleted = pdTRUE;
    }
}

void vTaskSetTimeOutState(TimeOut_t *timeout)
{
    if (timeout != NULL) {
        timeout->time_on_entering = fake_ticks;
    }
}

BaseType_t xTaskCheckForTimeOut(
    TimeOut_t *timeout,
    TickType_t *ticks_to_wait)
{
    if ((timeout == NULL) || (ticks_to_wait == NULL)) {
        return pdTRUE;
    }
    const TickType_t elapsed = fake_ticks - timeout->time_on_entering;
    if (elapsed >= *ticks_to_wait) {
        return pdTRUE;
    }
    *ticks_to_wait -= elapsed;
    timeout->time_on_entering = fake_ticks;
    return pdFALSE;
}

StreamBufferHandle_t xStreamBufferCreateStatic(size_t buffer_size_bytes,
    size_t trigger_level_bytes, uint8_t *storage_area,
    StaticStreamBuffer_t *stream_buffer)
{
    (void) trigger_level_bytes;
    ++observations.static_stream_create_calls;
    if ((stream_buffer == NULL) || (storage_area == NULL)
        || (buffer_size_bytes < 2U)) {
        return NULL;
    }
    stream_buffer->storage = storage_area;
    stream_buffer->storage_size = buffer_size_bytes;
    stream_buffer->read_index = 0U;
    stream_buffer->write_index = 0U;
    stream_buffer->count = 0U;
    return stream_buffer;
}

size_t xStreamBufferSendFromISR(StreamBufferHandle_t stream_buffer,
    const void *data, size_t data_length_bytes,
    BaseType_t *higher_priority_task_woken)
{
    const uint8_t *input = data;
    ++observations.stream_send_from_isr_calls;
    if ((stream_buffer == NULL) || ((input == NULL) && (data_length_bytes > 0U))) {
        return 0U;
    }
    const size_t capacity = stream_buffer->storage_size - 1U;
    size_t accepted = data_length_bytes;
    if (accepted > (capacity - stream_buffer->count)) {
        accepted = capacity - stream_buffer->count;
    }
    for (size_t index = 0U; index < accepted; ++index) {
        stream_buffer->storage[stream_buffer->write_index] = input[index];
        stream_buffer->write_index =
            (stream_buffer->write_index + 1U) % stream_buffer->storage_size;
    }
    stream_buffer->count += accepted;
    if (higher_priority_task_woken != NULL) {
        *higher_priority_task_woken = isr_task_woken ? pdTRUE : pdFALSE;
    }
    return accepted;
}

size_t xStreamBufferReceive(StreamBufferHandle_t stream_buffer, void *data,
    size_t buffer_length_bytes, TickType_t ticks_to_wait)
{
    (void) ticks_to_wait;
    uint8_t *output = data;
    if ((stream_buffer == NULL) || ((output == NULL) && (buffer_length_bytes > 0U))) {
        return 0U;
    }
    size_t received = stream_buffer->count;
    if (received > buffer_length_bytes) {
        received = buffer_length_bytes;
    }
    for (size_t index = 0U; index < received; ++index) {
        output[index] = stream_buffer->storage[stream_buffer->read_index];
        stream_buffer->read_index =
            (stream_buffer->read_index + 1U) % stream_buffer->storage_size;
    }
    stream_buffer->count -= received;
    if (next_stream_receive_hook != NULL) {
        fake_freertos_hook_t hook = next_stream_receive_hook;
        next_stream_receive_hook = NULL;
        hook();
    }
    if ((received == 0U) && (empty_stream_receive_hook != NULL)) {
        fake_freertos_hook_t hook = empty_stream_receive_hook;
        empty_stream_receive_hook = NULL;
        hook();
    }
    return received;
}

size_t xStreamBufferBytesAvailable(StreamBufferHandle_t stream_buffer)
{
    return (stream_buffer == NULL) ? 0U : stream_buffer->count;
}

SemaphoreHandle_t xSemaphoreCreateMutexStatic(StaticSemaphore_t *buffer)
{
    ++observations.static_mutex_create_calls;
    if (buffer == NULL) {
        return NULL;
    }
    buffer->available = pdTRUE;
    current_mutex = buffer;
    return buffer;
}

SemaphoreHandle_t xSemaphoreCreateBinaryStatic(StaticSemaphore_t *buffer)
{
    ++observations.static_binary_semaphore_create_calls;
    if (buffer == NULL) {
        return NULL;
    }
    buffer->available = pdFALSE;
    current_binary_semaphore = buffer;
    return buffer;
}

BaseType_t xSemaphoreTake(SemaphoreHandle_t mutex, TickType_t ticks_to_wait)
{
    (void) ticks_to_wait;
    if ((current_binary_semaphore != NULL)
        && (mutex == current_binary_semaphore)) {
        ++observations.binary_semaphore_take_calls;
        if (mutex == NULL) {
            return pdFALSE;
        }
        if (mutex->available == pdFALSE) {
            ++observations.binary_semaphore_wait_calls;
            if ((ticks_to_wait != 0U)
                && (current_task != NULL)
                && (current_task->deleted == pdFALSE)) {
                (void) fake_freertos_run_task_until_blocked();
            }
        }
        if (mutex->available == pdFALSE) {
            return pdFALSE;
        }
        mutex->available = pdFALSE;
        return pdTRUE;
    }

    ++observations.mutex_take_calls;
    if (mutex == NULL) {
        return pdFALSE;
    }
    if ((mutex->available == pdFALSE) && (mutex_wait_hook != NULL)) {
        fake_freertos_hook_t hook = mutex_wait_hook;
        mutex_wait_hook = NULL;
        ++observations.mutex_wait_calls;
        hook();
    }
    if (mutex->available == pdFALSE) {
        return pdFALSE;
    }
    mutex->available = pdFALSE;
    return pdTRUE;
}

BaseType_t xSemaphoreGive(SemaphoreHandle_t mutex)
{
    if ((current_binary_semaphore != NULL)
        && (mutex == current_binary_semaphore)) {
        ++observations.binary_semaphore_give_calls;
        if (mutex == NULL) {
            return pdFALSE;
        }
        mutex->available = pdTRUE;
        return pdTRUE;
    }

    ++observations.mutex_give_calls;
    if (mutex == NULL) {
        return pdFALSE;
    }
    mutex->available = pdTRUE;
    return pdTRUE;
}
