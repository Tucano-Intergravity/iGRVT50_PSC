#include "csp_rs485_internal.h"

#include <FreeRTOS.h>
#include <semphr.h>
#include <stream_buffer.h>
#include <task.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdatomic.h>
#include <stdint.h>

#include <csp_rs485_profile.h>

#define CSP_RS485_NOTIFY_RX UINT32_C(1)
#define CSP_RS485_NOTIFY_FAULT UINT32_C(2)
#define CSP_RS485_NOTIFY_STOP UINT32_C(4)
#define CSP_RS485_NOTIFY_ACTIVATE UINT32_C(8)
#define CSP_RS485_NOTIFY_ALL \
    (CSP_RS485_NOTIFY_RX | CSP_RS485_NOTIFY_FAULT | CSP_RS485_NOTIFY_STOP \
        | CSP_RS485_NOTIFY_ACTIVATE)
#define CSP_RS485_STREAM_STORAGE_SIZE (CSP_RS485_STREAM_BUFFER_SIZE + 1U)
#define CSP_RS485_GAP_BOUNDARY_CAPACITY 4U

static StaticTask_t task_storage;
static StackType_t task_stack[CSP_RS485_TASK_STACK_CAPACITY_WORDS];
static StaticStreamBuffer_t stream_storage;
static uint8_t stream_bytes[CSP_RS485_STREAM_STORAGE_SIZE];
static StaticSemaphore_t tx_mutex_storage;
static StaticSemaphore_t stop_ack_storage;
static _Atomic(TaskHandle_t) task_handle;
static StreamBufferHandle_t rx_stream;
static SemaphoreHandle_t tx_mutex;
static SemaphoreHandle_t stop_ack;
static TickType_t recovery_retry_ticks;
static atomic_bool runtime_active;
static atomic_bool producer_active;
static atomic_bool consumer_active;
static bool retry_ready;
static uint8_t rx_chunk[CSP_RS485_DMA_MAX_EVENT_BYTES];
static _Atomic uint32_t produced_sequence;
static uint32_t gap_boundaries[CSP_RS485_GAP_BOUNDARY_CAPACITY];
static atomic_size_t gap_write_index;
static atomic_size_t gap_read_index;
static atomic_bool gap_at_stream_tail;
static atomic_bool gap_saturated;
static uint32_t consumed_sequence;
static bool discarding_stream;

_Static_assert(
    CSP_RS485_STREAM_STORAGE_SIZE == (CSP_RS485_STREAM_BUFFER_SIZE + 1U),
    "FreeRTOS stream buffer storage includes one sentinel byte");

static bool wait_for_notification(TickType_t ticks_to_wait, uint32_t *bits);

static void runtime_task(void *argument)
{
    (void) argument;

    for (;;) {
        (void) csp_rs485_freertos_run_once();
        if (!atomic_load_explicit(&runtime_active, memory_order_acquire)) {
            /*
             * Acknowledge only after run_once() has returned to this safe
             * point. The lifecycle owner then deletes this task by handle;
             * self-delete would defer static TCB cleanup to the idle task and
             * make an immediate deinit/reinit reuse the storage too early.
             */
            (void) xSemaphoreGive(stop_ack);
            for (;;) {
                uint32_t ignored = 0U;
                (void) wait_for_notification(portMAX_DELAY, &ignored);
            }
        }
    }
}

static bool wait_for_notification(TickType_t ticks_to_wait, uint32_t *bits)
{
    return xTaskNotifyWait(
               0U,
               CSP_RS485_NOTIFY_ALL,
               bits,
               ticks_to_wait)
        == pdTRUE;
}

bool csp_rs485_freertos_start(const csp_rs485_link_config_t *config)
{
    const TickType_t retry_ticks = (config == NULL)
        ? 0U
        : pdMS_TO_TICKS(config->recovery_retry_ms);
    if ((config == NULL)
        || (config->task_priority >= (uint32_t) configMAX_PRIORITIES)
        || (config->task_stack_words == 0U)
        || (config->task_stack_words > CSP_RS485_TASK_STACK_CAPACITY_WORDS)
        || (retry_ticks == 0U)) {
        return false;
    }

    rx_stream = xStreamBufferCreateStatic(
        sizeof(stream_bytes),
        1U,
        stream_bytes,
        &stream_storage);
    tx_mutex = xSemaphoreCreateMutexStatic(&tx_mutex_storage);
    stop_ack = xSemaphoreCreateBinaryStatic(&stop_ack_storage);
    if ((rx_stream == NULL) || (tx_mutex == NULL) || (stop_ack == NULL)) {
        rx_stream = NULL;
        tx_mutex = NULL;
        stop_ack = NULL;
        return false;
    }

    recovery_retry_ticks = retry_ticks;
    atomic_store_explicit(&runtime_active, true, memory_order_release);
    atomic_store_explicit(&producer_active, false, memory_order_release);
    atomic_store_explicit(&consumer_active, false, memory_order_release);
    retry_ready = false;
    consumed_sequence = 0U;
    atomic_store_explicit(&produced_sequence, 0U, memory_order_relaxed);
    atomic_store_explicit(
        &gap_write_index,
        0U,
        memory_order_relaxed);
    atomic_store_explicit(
        &gap_read_index,
        0U,
        memory_order_relaxed);
    atomic_store_explicit(&gap_at_stream_tail, false, memory_order_relaxed);
    atomic_store_explicit(&gap_saturated, false, memory_order_relaxed);
    discarding_stream = false;
    TaskHandle_t const created_task = xTaskCreateStatic(
        runtime_task,
        "csp-rs485",
        config->task_stack_words,
        NULL,
        config->task_priority,
        task_stack,
        &task_storage);
    if (created_task == NULL) {
        atomic_store_explicit(&runtime_active, false, memory_order_release);
        rx_stream = NULL;
        tx_mutex = NULL;
        stop_ack = NULL;
        return false;
    }

    atomic_store_explicit(&task_handle, created_task, memory_order_release);
    atomic_store_explicit(&producer_active, true, memory_order_release);

    return true;
}

void csp_rs485_freertos_activate(void)
{
    TaskHandle_t const handle = atomic_load_explicit(
        &task_handle,
        memory_order_acquire);
    if (handle != NULL) {
        atomic_store_explicit(&consumer_active, true, memory_order_release);
        (void) xTaskNotify(handle, CSP_RS485_NOTIFY_ACTIVATE, eSetBits);
    }
}

void csp_rs485_freertos_quiesce_producer(void)
{
    atomic_store_explicit(&producer_active, false, memory_order_release);
}

void csp_rs485_freertos_stop(void)
{
    TaskHandle_t const handle = atomic_exchange_explicit(
        &task_handle,
        NULL,
        memory_order_acq_rel);
    if (handle == NULL) {
        return;
    }

    atomic_store_explicit(&runtime_active, false, memory_order_release);
    atomic_store_explicit(&consumer_active, false, memory_order_release);
    (void) xTaskNotify(handle, CSP_RS485_NOTIFY_STOP, eSetBits);
    (void) xSemaphoreTake(stop_ack, portMAX_DELAY);
    vTaskDelete(handle);
    rx_stream = NULL;
    tx_mutex = NULL;
    stop_ack = NULL;
    retry_ready = false;
}

bool csp_rs485_freertos_take_tx_mutex(void)
{
    return (tx_mutex != NULL)
        && (xSemaphoreTake(tx_mutex, portMAX_DELAY) == pdTRUE);
}

void csp_rs485_freertos_give_tx_mutex(void)
{
    if (tx_mutex != NULL) {
        (void) xSemaphoreGive(tx_mutex);
    }
}

static bool record_gap_boundary_from_isr(uint32_t boundary)
{
    if (atomic_exchange_explicit(
            &gap_at_stream_tail,
            true,
            memory_order_acq_rel)) {
        return false;
    }

    const size_t write_index = atomic_load_explicit(
        &gap_write_index,
        memory_order_relaxed);
    const size_t read_index = atomic_load_explicit(
        &gap_read_index,
        memory_order_acquire);
    if ((write_index - read_index) < CSP_RS485_GAP_BOUNDARY_CAPACITY) {
        gap_boundaries[write_index % CSP_RS485_GAP_BOUNDARY_CAPACITY] =
            boundary;
        atomic_store_explicit(
            &gap_write_index,
            write_index + 1U,
            memory_order_release);
    } else {
        atomic_store_explicit(
            &gap_saturated,
            true,
            memory_order_release);
    }
    return true;
}

void csp_rs485_freertos_rx_from_isr(const uint8_t *bytes, size_t length)
{
    TaskHandle_t const handle = atomic_load_explicit(
        &task_handle,
        memory_order_acquire);
    if ((!atomic_load_explicit(&producer_active, memory_order_acquire))
        || (handle == NULL)
        || (rx_stream == NULL)
        || (length == 0U)
        || ((bytes == NULL) && (length > 0U))) {
        return;
    }

    BaseType_t task_woken = pdFALSE;
    const size_t accepted = xStreamBufferSendFromISR(
        rx_stream,
        bytes,
        length,
        &task_woken);
    const size_t available = xStreamBufferBytesAvailable(rx_stream);
    const uint32_t accepted_count = (uint32_t) accepted;
    const uint32_t accepted_end = atomic_fetch_add_explicit(
        &produced_sequence,
        accepted_count,
        memory_order_acq_rel) + accepted_count;
    if (accepted > 0U) {
        atomic_store_explicit(
            &gap_at_stream_tail,
            false,
            memory_order_release);
    }
    bool discontinuity = false;
    if (accepted != length) {
        discontinuity = record_gap_boundary_from_isr(accepted_end);
    }
    csp_rs485_link_record_rx_stream_from_isr(
        length - accepted,
        available,
        discontinuity);
    if (accepted > 0U) {
        (void) xTaskNotifyFromISR(
            handle,
            CSP_RS485_NOTIFY_RX,
            eSetBits,
            &task_woken);
    }
    if (task_woken != pdFALSE) {
        portYIELD_FROM_ISR(task_woken);
    }
}

void csp_rs485_freertos_mark_rx_discontinuity_from_isr(void)
{
    TaskHandle_t const handle = atomic_load_explicit(
        &task_handle,
        memory_order_acquire);
    if ((!atomic_load_explicit(&producer_active, memory_order_acquire))
        || (handle == NULL)
        || (rx_stream == NULL)) {
        return;
    }

    const uint32_t boundary = atomic_load_explicit(
        &produced_sequence,
        memory_order_acquire);
    const bool discontinuity = record_gap_boundary_from_isr(boundary);
    csp_rs485_link_record_rx_stream_from_isr(0U, 0U, discontinuity);

    BaseType_t task_woken = pdFALSE;
    (void) xTaskNotifyFromISR(
        handle,
        CSP_RS485_NOTIFY_RX,
        eSetBits,
        &task_woken);
    if (task_woken != pdFALSE) {
        portYIELD_FROM_ISR(task_woken);
    }
}

void csp_rs485_freertos_notify_fault(void)
{
    TaskHandle_t const handle = atomic_load_explicit(
        &task_handle,
        memory_order_acquire);
    if (handle != NULL) {
        (void) xTaskNotify(handle, CSP_RS485_NOTIFY_FAULT, eSetBits);
    }
}

void csp_rs485_freertos_notify_fault_from_isr(void)
{
    TaskHandle_t const handle = atomic_load_explicit(
        &task_handle,
        memory_order_acquire);
    if (handle == NULL) {
        return;
    }

    BaseType_t task_woken = pdFALSE;
    (void) xTaskNotifyFromISR(
        handle,
        CSP_RS485_NOTIFY_FAULT,
        eSetBits,
        &task_woken);
    if (task_woken != pdFALSE) {
        portYIELD_FROM_ISR(task_woken);
    }
}

void csp_rs485_freertos_request_stop(void)
{
    TaskHandle_t const handle = atomic_load_explicit(
        &task_handle,
        memory_order_acquire);
    if (handle != NULL) {
        (void) xTaskNotify(handle, CSP_RS485_NOTIFY_STOP, eSetBits);
    }
}

static void consume_stream_chunk(const uint8_t *bytes, size_t length)
{
    if (atomic_exchange_explicit(
            &gap_saturated,
            false,
            memory_order_acq_rel)) {
        csp_rs485_link_reset_rx_parser();
        discarding_stream = true;
        atomic_store_explicit(
            &gap_read_index,
            atomic_load_explicit(&gap_write_index, memory_order_acquire),
            memory_order_release);
    }
    if (discarding_stream) {
        consumed_sequence += (uint32_t) length;
        return;
    }

    size_t position = 0U;

    for (;;) {
        const size_t read_index = atomic_load_explicit(
            &gap_read_index,
            memory_order_relaxed);
        const size_t write_index = atomic_load_explicit(
            &gap_write_index,
            memory_order_acquire);
        if (read_index != write_index) {
            const uint32_t boundary = gap_boundaries[
                read_index % CSP_RS485_GAP_BOUNDARY_CAPACITY];
            if ((uint32_t) (boundary - consumed_sequence) == 0U) {
                csp_rs485_link_reset_rx_parser();
                atomic_store_explicit(
                    &gap_read_index,
                    read_index + 1U,
                    memory_order_release);
                continue;
            }
        }
        if (position == length) {
            break;
        }

        size_t chunk = length - position;
        if (read_index != write_index) {
            const uint32_t boundary = gap_boundaries[
                read_index % CSP_RS485_GAP_BOUNDARY_CAPACITY];
            const uint32_t before_boundary =
                boundary - consumed_sequence;
            if (chunk > (size_t) before_boundary) {
                chunk = (size_t) before_boundary;
            }
        }

        csp_rs485_link_consume_rx_bytes(&bytes[position], chunk);
        position += chunk;
        consumed_sequence += (uint32_t) chunk;
    }
}

static bool wait_for_retry_interval(void)
{
    TickType_t remaining = recovery_retry_ticks;
    TimeOut_t timeout;
    vTaskSetTimeOutState(&timeout);

    for (;;) {
        uint32_t notifications = 0U;
        (void) wait_for_notification(remaining, &notifications);
        if ((notifications & CSP_RS485_NOTIFY_STOP) != 0U) {
            atomic_store_explicit(
                &runtime_active,
                false,
                memory_order_release);
            return false;
        }
        if (xTaskCheckForTimeOut(&timeout, &remaining) == pdTRUE) {
            return true;
        }
    }
}

size_t csp_rs485_freertos_run_once(void)
{
    if ((!atomic_load_explicit(&runtime_active, memory_order_acquire))
        || (rx_stream == NULL)) {
        return 0U;
    }

    uint32_t notifications = 0U;
    bool notified = false;
    const bool retry_due = retry_ready;
    if (retry_due) {
        retry_ready = false;
        notified = wait_for_notification(0U, &notifications);
    } else {
        notified = wait_for_notification(portMAX_DELAY, &notifications);
    }
    if ((notifications & CSP_RS485_NOTIFY_STOP) != 0U) {
        atomic_store_explicit(&runtime_active, false, memory_order_release);
        return 0U;
    }

    if (!atomic_load_explicit(&consumer_active, memory_order_acquire)) {
        return 0U;
    }

    csp_rs485_link_process_isr_faults();

    if ((!notified) && (!retry_due)) {
        return 0U;
    }

    size_t total = 0U;
    for (;;) {
        const size_t received = xStreamBufferReceive(
            rx_stream,
            rx_chunk,
            sizeof(rx_chunk),
            0U);
        if (received == 0U) {
            consume_stream_chunk(rx_chunk, 0U);
            if (discarding_stream) {
                bool discard_complete = false;
                taskENTER_CRITICAL();
                if (xStreamBufferBytesAvailable(rx_stream) == 0U) {
                    atomic_store_explicit(
                        &gap_read_index,
                        atomic_load_explicit(
                            &gap_write_index,
                            memory_order_acquire),
                        memory_order_release);
                    discarding_stream = false;
                    discard_complete = true;
                }
                taskEXIT_CRITICAL();
                if (!discard_complete) {
                    continue;
                }
            }
            break;
        }

        consume_stream_chunk(rx_chunk, received);
        total += received;
    }

    csp_rs485_health_t health;
    csp_rs485_link_get_health(&health);
    if (health.state == CSP_RS485_LINK_RECOVERING) {
        const csp_rs485_recovery_result_t result =
            csp_rs485_link_recovery_step();
        if (result == CSP_RS485_RECOVERY_RETRY_AFTER_WAIT) {
            if (wait_for_retry_interval()) {
                retry_ready = true;
            }
        }
    }

    return total;
}

#ifdef CSP_RS485_HOST_TEST
void csp_rs485_freertos_test_set_rx_sequence(size_t sequence)
{
    const uint32_t sequence32 = (uint32_t) sequence;
    atomic_store_explicit(
        &produced_sequence,
        sequence32,
        memory_order_relaxed);
    consumed_sequence = sequence32;
}
#endif
