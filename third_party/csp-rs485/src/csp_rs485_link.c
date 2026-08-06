#include "csp_rs485_internal.h"

#include <FreeRTOS.h>
#include <task.h>

#include <csp/csp_buffer.h>
#include <csp/csp_crc32.h>
#include <csp/csp_error.h>
#include <csp/csp_iflist.h>
#include <csp/interfaces/csp_if_kiss.h>

#include <csp_rs485_link.h>
#include <csp_rs485_profile.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdatomic.h>
#include <stdint.h>
#include <string.h>

#define CSP_RS485_INTERFACE_NAME "RS485"

static int csp_rs485_next_hop(
    const csp_route_t *route,
    csp_packet_t *packet);

static csp_rs485_link_context_t link;
static csp_rs485_health_t published_health;
static csp_kiss_interface_data_t kiss;
static uint8_t tx_frame[CSP_RS485_TX_FRAME_MAX];
static atomic_bool rx_discontinuity_pending;
static atomic_bool shutdown_pending;
static atomic_uint_fast32_t operational_state;
static atomic_uint_fast32_t pending_isr_uart_faults;
static atomic_uint_fast32_t pending_isr_dma_faults;
static atomic_uint_fast32_t pending_isr_last_fault;
static atomic_uint_fast32_t isr_fault_sequence;
static atomic_uint_fast32_t isr_stream_sequence;
static atomic_uint_fast32_t isr_stream_dropped_bytes;
static atomic_uint_fast32_t isr_stream_high_watermark;
static atomic_uint_fast32_t isr_stream_discontinuities;
static csp_iface_t interface = {
    .name = CSP_RS485_INTERFACE_NAME,
    .interface_data = &kiss,
    .nexthop = csp_rs485_next_hop,
    .mtu = CSP_RS485_INTERFACE_MTU,
};

void csp_rs485_link_health_task_lock(void)
{
    taskENTER_CRITICAL();
}

void csp_rs485_link_health_task_unlock(void)
{
    taskEXIT_CRITICAL();
}

static csp_rs485_link_state_t operational_state_get(void)
{
    for (;;) {
        if (atomic_load_explicit(&shutdown_pending, memory_order_acquire)) {
            return CSP_RS485_LINK_STOPPED;
        }
        const uint_fast32_t sequence_before = atomic_load_explicit(
            &isr_fault_sequence,
            memory_order_acquire);
        if ((sequence_before & UINT32_C(1)) != 0U) {
            continue;
        }
        const csp_rs485_link_state_t state =
            (csp_rs485_link_state_t) atomic_load_explicit(
                &operational_state,
                memory_order_acquire);
        const bool fault_pending =
            (atomic_load_explicit(
                &pending_isr_uart_faults,
                memory_order_relaxed)
                != 0U)
            || (atomic_load_explicit(
                &pending_isr_dma_faults,
                memory_order_relaxed)
                != 0U)
            || (atomic_load_explicit(
                &pending_isr_last_fault,
                memory_order_relaxed)
                != CSP_RS485_FAULT_NONE);
        const uint_fast32_t sequence_after = atomic_load_explicit(
            &isr_fault_sequence,
            memory_order_acquire);
        if (sequence_before == sequence_after) {
            return fault_pending
                ? CSP_RS485_LINK_RECOVERING
                : state;
        }
    }
}

static void operational_state_set(csp_rs485_link_state_t state)
{
    if (atomic_load_explicit(&shutdown_pending, memory_order_acquire)) {
        state = CSP_RS485_LINK_STOPPED;
    }
    atomic_store_explicit(
        &operational_state,
        (uint_fast32_t) state,
        memory_order_release);
}

static void sync_operational_state(void)
{
    csp_rs485_link_health_task_lock();
    const csp_rs485_link_state_t state = link.health.state;
    published_health = link.health;
    csp_rs485_link_health_task_unlock();
    operational_state_set(state);
}

static uint32_t frame_wire_time_ms(size_t frame_length)
{
    const uint64_t numerator =
        ((uint64_t) frame_length
            * (uint64_t) CSP_RS485_BITS_PER_WIRE_BYTE
            * UINT64_C(1000))
        + (uint64_t) CSP_RS485_BAUD_RATE
        - UINT64_C(1);

    return (uint32_t) (numerator / (uint64_t) CSP_RS485_BAUD_RATE);
}

static bool config_is_valid(const csp_rs485_link_config_t *config)
{
    if ((config == NULL)
        || (config->port_ops == NULL)
        || (config->port_ops->initialize == NULL)
        || (config->port_ops->arm_receive == NULL)
        || (config->port_ops->enable_irqs == NULL)
        || (config->port_ops->disable_and_clear_irqs == NULL)
        || (config->port_ops->abort_receive == NULL)
        || (config->port_ops->deinitialize == NULL)
        || (config->port_ops->force_receive_mode == NULL)
        || (config->port_ops->reset_rx_position == NULL)
        || (config->port_ops->transmit_frame == NULL)
        || (config->task_priority >= (uint32_t) configMAX_PRIORITIES)
        || (config->task_stack_words == 0U)
        || (config->task_stack_words > CSP_RS485_TASK_STACK_CAPACITY_WORDS)
        || (pdMS_TO_TICKS(config->recovery_retry_ms) == 0U)
        || (csp_buffer_data_size() != CSP_RS485_CSP_BUFFER_DATA_SIZE)) {
        return false;
    }

    const uint32_t maximum_wire_time =
        frame_wire_time_ms(CSP_RS485_TX_FRAME_MAX);
    return config->tx_margin_ms <= (UINT32_MAX - maximum_wire_time);
}

static int register_interface(void)
{
    csp_iface_t *listed =
        csp_iflist_get_by_name(CSP_RS485_INTERFACE_NAME);

    if (listed == &interface) {
        return CSP_ERR_NONE;
    }
    if (listed != NULL) {
        return CSP_ERR_ALREADY;
    }

    return csp_iflist_add(&interface);
}

static void initialize_kiss_state(void)
{
    kiss.max_rx_length =
        CSP_RS485_CSP_HEADER_SIZE + CSP_RS485_CSP_BUFFER_DATA_SIZE;
    kiss.tx_func = NULL;
    kiss.rx_mode = KISS_MODE_NOT_STARTED;
    kiss.rx_length = 0U;
    kiss.rx_first = false;
    kiss.rx_packet = NULL;
}

void csp_rs485_kiss_reset(csp_kiss_interface_data_t *kiss_state)
{
    if (kiss_state == NULL) {
        return;
    }

    if (kiss_state->rx_packet != NULL) {
        csp_buffer_free(kiss_state->rx_packet);
        kiss_state->rx_packet = NULL;
    }

    kiss_state->rx_mode = KISS_MODE_NOT_STARTED;
    kiss_state->rx_length = 0U;
    kiss_state->rx_first = false;
}

int csp_rs485_link_init(const csp_rs485_link_config_t *config)
{
    if (!config_is_valid(config)) {
        return CSP_ERR_INVAL;
    }
    if (link.health.state != CSP_RS485_LINK_STOPPED) {
        return CSP_ERR_ALREADY;
    }
    atomic_store_explicit(&shutdown_pending, false, memory_order_release);

    interface.mtu =
        (uint16_t) (csp_buffer_data_size() - CSP_RS485_CSP_CRC_SIZE);
    const int register_result = register_interface();
    if (register_result != CSP_ERR_NONE) {
        return register_result;
    }

    initialize_kiss_state();
    atomic_store_explicit(
        &rx_discontinuity_pending,
        false,
        memory_order_relaxed);
    atomic_store_explicit(&pending_isr_uart_faults, 0U, memory_order_relaxed);
    atomic_store_explicit(&pending_isr_dma_faults, 0U, memory_order_relaxed);
    atomic_store_explicit(
        &pending_isr_last_fault,
        CSP_RS485_FAULT_NONE,
        memory_order_relaxed);
    atomic_store_explicit(&isr_fault_sequence, 0U, memory_order_relaxed);
    atomic_store_explicit(&isr_stream_sequence, 0U, memory_order_relaxed);
    atomic_store_explicit(
        &isr_stream_dropped_bytes,
        0U,
        memory_order_relaxed);
    atomic_store_explicit(
        &isr_stream_high_watermark,
        0U,
        memory_order_relaxed);
    atomic_store_explicit(
        &isr_stream_discontinuities,
        0U,
        memory_order_relaxed);
    link.config = *config;
    memset(&link.health, 0, sizeof(link.health));
    link.health.state = CSP_RS485_LINK_STOPPED;
    published_health = link.health;
    operational_state_set(CSP_RS485_LINK_STOPPED);

    if (!csp_rs485_freertos_start(config)) {
        return CSP_ERR_DRIVER;
    }
    (void) csp_rs485_supervisor_start(&link);
    sync_operational_state();

    /*
     * The software lifecycle is established even when the first hardware
     * start fails. RECOVERING blocks TX, and activation lets the owner task
     * retry through the same bounded timer-driven path used at runtime.
     */
    csp_rs485_freertos_activate();
    return CSP_ERR_NONE;
}

void csp_rs485_link_deinit(void)
{
    atomic_store_explicit(&shutdown_pending, true, memory_order_release);
    operational_state_set(CSP_RS485_LINK_STOPPED);
    csp_rs485_freertos_quiesce_producer();
    const bool mutex_locked = csp_rs485_freertos_take_tx_mutex();
    csp_rs485_supervisor_stop(&link);
    sync_operational_state();
    if (mutex_locked) {
        csp_rs485_freertos_give_tx_mutex();
    }
    csp_rs485_freertos_stop();
    csp_rs485_kiss_reset(&kiss);
    atomic_store_explicit(
        &rx_discontinuity_pending,
        false,
        memory_order_relaxed);
    memset(&link.config, 0, sizeof(link.config));
}

csp_iface_t *csp_rs485_link_get_interface(void)
{
    return &interface;
}

void csp_rs485_link_get_health(csp_rs485_health_t *health)
{
    if (health != NULL) {
        for (;;) {
            const uint_fast32_t sequence_before = atomic_load_explicit(
                &isr_stream_sequence,
                memory_order_acquire);
            if ((sequence_before & UINT32_C(1)) != 0U) {
                continue;
            }
            csp_rs485_link_health_task_lock();
            csp_rs485_health_t snapshot = published_health;
            snapshot.stream_dropped_bytes += (uint32_t) atomic_load_explicit(
                &isr_stream_dropped_bytes,
                memory_order_relaxed);
            const uint32_t isr_high_watermark = (uint32_t) atomic_load_explicit(
                &isr_stream_high_watermark,
                memory_order_relaxed);
            if (isr_high_watermark > snapshot.stream_high_watermark) {
                snapshot.stream_high_watermark = isr_high_watermark;
            }
            snapshot.stream_discontinuities += (uint32_t) atomic_load_explicit(
                &isr_stream_discontinuities,
                memory_order_relaxed);
            const uint_fast32_t sequence_after = atomic_load_explicit(
                &isr_stream_sequence,
                memory_order_acquire);
            csp_rs485_link_health_task_unlock();
            if (sequence_before == sequence_after) {
                *health = snapshot;
                return;
            }
        }
    }
}

void csp_rs485_link_report_fault(csp_rs485_fault_t fault)
{
    if (fault == CSP_RS485_FAULT_NONE) {
        return;
    }

    csp_rs485_link_health_task_lock();
    link.health.last_error = fault;
    if (fault == CSP_RS485_FAULT_UART) {
        ++link.health.uart_errors;
    } else if (fault == CSP_RS485_FAULT_DMA) {
        ++link.health.dma_errors;
    }

    if (link.health.state == CSP_RS485_LINK_RUNNING) {
        link.health.state = CSP_RS485_LINK_RECOVERING;
    }
    published_health = link.health;
    operational_state_set(link.health.state);
    csp_rs485_link_health_task_unlock();
    csp_rs485_freertos_notify_fault();
}

void csp_rs485_link_report_fault_from_isr(csp_rs485_fault_t fault)
{
    if (fault == CSP_RS485_FAULT_NONE) {
        return;
    }

    (void) atomic_fetch_add_explicit(
        &isr_fault_sequence,
        1U,
        memory_order_acq_rel);
    if (fault == CSP_RS485_FAULT_UART) {
        (void) atomic_fetch_add_explicit(
            &pending_isr_uart_faults,
            1U,
            memory_order_relaxed);
    } else if (fault == CSP_RS485_FAULT_DMA) {
        (void) atomic_fetch_add_explicit(
            &pending_isr_dma_faults,
            1U,
            memory_order_relaxed);
    }
    atomic_store_explicit(
        &pending_isr_last_fault,
        (uint_fast32_t) fault,
        memory_order_release);
    uint_fast32_t expected = (uint_fast32_t) CSP_RS485_LINK_RUNNING;
    (void) atomic_compare_exchange_strong_explicit(
        &operational_state,
        &expected,
        (uint_fast32_t) CSP_RS485_LINK_RECOVERING,
        memory_order_acq_rel,
        memory_order_acquire);
    (void) atomic_fetch_add_explicit(
        &isr_fault_sequence,
        1U,
        memory_order_release);
    csp_rs485_freertos_notify_fault_from_isr();
}

void csp_rs485_link_process_isr_faults(void)
{
    const uint_fast32_t uart_faults = atomic_exchange_explicit(
        &pending_isr_uart_faults,
        0U,
        memory_order_acq_rel);
    const uint_fast32_t dma_faults = atomic_exchange_explicit(
        &pending_isr_dma_faults,
        0U,
        memory_order_acq_rel);
    const csp_rs485_fault_t last_fault = (csp_rs485_fault_t)
        atomic_exchange_explicit(
            &pending_isr_last_fault,
            CSP_RS485_FAULT_NONE,
            memory_order_acq_rel);
    if ((uart_faults == 0U)
        && (dma_faults == 0U)
        && (last_fault == CSP_RS485_FAULT_NONE)) {
        return;
    }

    csp_rs485_link_health_task_lock();
    link.health.uart_errors += (uint32_t) uart_faults;
    link.health.dma_errors += (uint32_t) dma_faults;
    if (last_fault != CSP_RS485_FAULT_NONE) {
        link.health.last_error = last_fault;
    }
    if (link.health.state == CSP_RS485_LINK_RUNNING) {
        link.health.state = CSP_RS485_LINK_RECOVERING;
    }
    published_health = link.health;
    operational_state_set(link.health.state);
    csp_rs485_link_health_task_unlock();
}

void csp_rs485_link_mark_rx_discontinuity(void)
{
    const csp_rs485_link_state_t state = operational_state_get();
    if (state == CSP_RS485_LINK_STOPPED) {
        return;
    }

    atomic_store_explicit(
        &rx_discontinuity_pending,
        true,
        memory_order_release);
    csp_rs485_link_health_task_lock();
    ++link.health.stream_discontinuities;
    published_health = link.health;
    csp_rs485_link_health_task_unlock();
}

void csp_rs485_link_mark_rx_discontinuity_once(void)
{
    const csp_rs485_link_state_t state = operational_state_get();
    if (state == CSP_RS485_LINK_STOPPED) {
        return;
    }

    const bool already_pending = atomic_exchange_explicit(
        &rx_discontinuity_pending,
        true,
        memory_order_acq_rel);
    if (!already_pending) {
        csp_rs485_link_health_task_lock();
        ++link.health.stream_discontinuities;
        published_health = link.health;
        csp_rs485_link_health_task_unlock();
    }
}

void csp_rs485_link_mark_rx_discontinuity_from_isr(void)
{
    if (operational_state_get() != CSP_RS485_LINK_RUNNING) {
        return;
    }

    csp_rs485_freertos_mark_rx_discontinuity_from_isr();
}

void csp_rs485_link_record_rx_stream_from_isr(
    size_t dropped_bytes,
    size_t high_watermark,
    bool discontinuity)
{
    (void) atomic_fetch_add_explicit(
        &isr_stream_sequence,
        1U,
        memory_order_acq_rel);
    (void) atomic_fetch_add_explicit(
        &isr_stream_dropped_bytes,
        (uint_fast32_t) dropped_bytes,
        memory_order_relaxed);
    uint_fast32_t observed_high_watermark = atomic_load_explicit(
        &isr_stream_high_watermark,
        memory_order_relaxed);
    while ((high_watermark > observed_high_watermark)
        && (!atomic_compare_exchange_weak_explicit(
                &isr_stream_high_watermark,
                &observed_high_watermark,
                (uint_fast32_t) high_watermark,
                memory_order_relaxed,
                memory_order_relaxed))) {
    }
    if (discontinuity) {
        (void) atomic_fetch_add_explicit(
            &isr_stream_discontinuities,
            1U,
            memory_order_relaxed);
    }
    (void) atomic_fetch_add_explicit(
        &isr_stream_sequence,
        1U,
        memory_order_release);
}

void csp_rs485_link_reset_rx_parser(void)
{
    csp_rs485_kiss_reset(&kiss);
}

void csp_rs485_link_consume_rx_bytes(
    const uint8_t *bytes,
    size_t length)
{
    const csp_rs485_link_state_t state = operational_state_get();
    if ((state != CSP_RS485_LINK_RUNNING)
        || ((bytes == NULL) && (length > 0U))) {
        return;
    }

    const bool had_discontinuity =
        atomic_exchange_explicit(
            &rx_discontinuity_pending,
            false,
            memory_order_acq_rel);
    if (had_discontinuity) {
        csp_rs485_kiss_reset(&kiss);
    }

    const uint32_t errors_before = interface.rx_error;
    csp_kiss_rx(&interface, bytes, length, NULL);
    csp_rs485_link_health_task_lock();
    link.health.protocol_errors += interface.rx_error - errors_before;
    published_health = link.health;
    csp_rs485_link_health_task_unlock();
}

csp_rs485_recovery_result_t csp_rs485_link_recovery_step(void)
{
    if (atomic_load_explicit(&shutdown_pending, memory_order_acquire)
        || ((csp_rs485_link_state_t) atomic_load_explicit(
            &operational_state,
            memory_order_acquire)
            == CSP_RS485_LINK_STOPPED)) {
        return CSP_RS485_RECOVERY_CANCELLED;
    }
    if (!csp_rs485_freertos_take_tx_mutex()) {
        return CSP_RS485_RECOVERY_RETRY_AFTER_WAIT;
    }

    csp_rs485_recovery_result_t result = CSP_RS485_RECOVERY_CANCELLED;
    const csp_rs485_link_state_t base_state =
        (csp_rs485_link_state_t) atomic_load_explicit(
            &operational_state,
            memory_order_acquire);
    if ((!atomic_load_explicit(&shutdown_pending, memory_order_acquire))
        && (base_state != CSP_RS485_LINK_STOPPED)) {
        result = csp_rs485_supervisor_recovery_step(&link);
        sync_operational_state();
    }
    csp_rs485_freertos_give_tx_mutex();
    return result;
}

static void record_transmit_failure(csp_rs485_port_result_t result)
{
    csp_rs485_link_health_task_lock();
    ++link.health.tx_failures;

    if (result == CSP_RS485_PORT_TIMEOUT) {
        ++link.health.tx_timeouts;
        link.health.last_error = CSP_RS485_FAULT_TX_TIMEOUT;
        if (link.health.state == CSP_RS485_LINK_RUNNING) {
            link.health.state = CSP_RS485_LINK_RECOVERING;
        }
    } else if (result == CSP_RS485_PORT_STATE_ERROR) {
        link.health.last_error = CSP_RS485_FAULT_TX_STATE;
        if (link.health.state == CSP_RS485_LINK_RUNNING) {
            link.health.state = CSP_RS485_LINK_RECOVERING;
        }
    }
    operational_state_set(link.health.state);
    published_health = link.health;
    csp_rs485_link_health_task_unlock();
    if ((result == CSP_RS485_PORT_TIMEOUT)
        || (result == CSP_RS485_PORT_STATE_ERROR)) {
        csp_rs485_freertos_notify_fault();
    }
}

static int csp_rs485_next_hop(
    const csp_route_t *route,
    csp_packet_t *packet)
{
    (void) route;

    const csp_rs485_link_state_t state = operational_state_get();
    if ((packet == NULL)
        || (state != CSP_RS485_LINK_RUNNING)
        || (packet->length > interface.mtu)) {
        return CSP_ERR_TX;
    }

    if (!csp_rs485_freertos_take_tx_mutex()) {
        csp_rs485_link_health_task_lock();
        ++link.health.tx_failures;
        published_health = link.health;
        csp_rs485_link_health_task_unlock();
        return CSP_ERR_TX;
    }

    if (operational_state_get() != CSP_RS485_LINK_RUNNING) {
        csp_rs485_freertos_give_tx_mutex();
        return CSP_ERR_TX;
    }

    const uint16_t original_length = packet->length;
    if (csp_crc32_append(packet, false) != CSP_ERR_NONE) {
        packet->length = original_length;
        csp_rs485_link_health_task_lock();
        ++link.health.tx_failures;
        published_health = link.health;
        csp_rs485_link_health_task_unlock();
        csp_rs485_freertos_give_tx_mutex();
        return CSP_ERR_TX;
    }

    size_t frame_length = 0U;
    const csp_rs485_kiss_encode_result_t encode_result =
        csp_rs485_kiss_encode(
            packet->id,
            packet->data,
            packet->length,
            tx_frame,
            sizeof(tx_frame),
            &frame_length);
    if (encode_result != CSP_RS485_KISS_ENCODE_OK) {
        packet->length = original_length;
        csp_rs485_link_health_task_lock();
        ++link.health.tx_failures;
        published_health = link.health;
        csp_rs485_link_health_task_unlock();
        csp_rs485_freertos_give_tx_mutex();
        return CSP_ERR_TX;
    }

    const uint32_t timeout_ms =
        frame_wire_time_ms(frame_length) + link.config.tx_margin_ms;
    const csp_rs485_port_result_t transmit_result =
        link.config.port_ops->transmit_frame(
            link.config.port_context,
            tx_frame,
            frame_length,
            timeout_ms);

    if (transmit_result == CSP_RS485_PORT_OK) {
        csp_buffer_free(packet);
        csp_rs485_freertos_give_tx_mutex();
        return CSP_ERR_NONE;
    }

    packet->length = original_length;
    record_transmit_failure(transmit_result);
    csp_rs485_freertos_give_tx_mutex();
    return CSP_ERR_TX;
}
