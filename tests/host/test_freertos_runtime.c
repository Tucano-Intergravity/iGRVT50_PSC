/* Imported from C:\PSC\csp-rs485\tests\host (authorized subset). */
/* This import makes no assertion about upstream licensing. */
#include "fakes/fake_port.h"
#include "fakes/freertos/fake_freertos.h"
#include "support/test.h"

#include <FreeRTOS.h>

#include <csp/csp.h>
#include <csp/csp_error.h>
#include <csp/csp_buffer.h>
#include <csp/csp_crc32.h>
#include <csp/interfaces/csp_if_kiss.h>

#include <csp_rs485_link.h>
#include <csp_rs485_profile.h>

#include "csp_rs485_internal.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

static fake_port_t fake;
static uint8_t retry_wait_byte = 0xC0U;
static uint8_t teardown_byte = 0xC0U;
static uint8_t health_interleave_bytes[CSP_RS485_STREAM_BUFFER_SIZE + 2U];
static uint8_t second_gap_bytes[CSP_RS485_DMA_MAX_EVENT_BYTES + 1U];
static uint8_t exit_window_gap_bytes[CSP_RS485_STREAM_BUFFER_SIZE + 1U];
static size_t saturated_gap_injections_remaining;
static size_t create_preemption_drained;
static uint8_t initial_irq_byte = 0xC0U;
static bool inject_initial_irq_events;
static size_t cleanup_operation_count;
static size_t cleanup_operations_before_mutex_release;
static bool cleanup_observed_unlocked_mutex;
static int tx_during_deinit_result;
static csp_rs485_recovery_result_t recovery_during_deinit_result;

static csp_rs485_link_config_t make_config(
    uint32_t stack_words,
    uint32_t retry_ms,
    const csp_rs485_port_ops_t *port_ops)
{
    const csp_rs485_link_config_t config = {
        .port_ops = port_ops,
        .port_context = &fake,
        .tx_margin_ms = 5U,
        .recovery_retry_ms = retry_ms,
        .task_priority = 3U,
        .task_stack_words = stack_words,
    };
    return config;
}

static void prepare_link(void)
{
    csp_rs485_link_deinit();
    fake_freertos_reset();
    fake_port_init(&fake);
    create_preemption_drained = 0U;
    inject_initial_irq_events = false;
    cleanup_operation_count = 0U;
    cleanup_operations_before_mutex_release = 0U;
    cleanup_observed_unlocked_mutex = false;
    tx_during_deinit_result = CSP_ERR_NONE;
    recovery_during_deinit_result = CSP_RS485_RECOVERY_COMPLETE;
}

static csp_rs485_port_result_t test_port_initialize(void *context)
{
    return fake_port_get_ops()->initialize(context);
}

static csp_rs485_port_result_t test_port_arm_receive(void *context)
{
    return fake_port_get_ops()->arm_receive(context);
}

static void test_port_enable_irqs(void *context)
{
    fake_port_get_ops()->enable_irqs(context);
    if (inject_initial_irq_events) {
        inject_initial_irq_events = false;
        csp_rs485_freertos_rx_from_isr(&initial_irq_byte, 1U);
        csp_rs485_link_report_fault_from_isr(CSP_RS485_FAULT_UART);
    }
}

static void record_cleanup_lock(void)
{
    ++cleanup_operation_count;
    if (fake_freertos_mutex_is_available()) {
        cleanup_observed_unlocked_mutex = true;
    }
}

static void test_port_disable_and_clear_irqs(void *context)
{
    record_cleanup_lock();
    fake_port_get_ops()->disable_and_clear_irqs(context);
}

static csp_rs485_port_result_t test_port_abort_receive(void *context)
{
    record_cleanup_lock();
    return fake_port_get_ops()->abort_receive(context);
}

static csp_rs485_port_result_t test_port_deinitialize(void *context)
{
    record_cleanup_lock();
    return fake_port_get_ops()->deinitialize(context);
}

static void test_port_force_receive_mode(void *context)
{
    record_cleanup_lock();
    fake_port_get_ops()->force_receive_mode(context);
}

static void test_port_reset_rx_position(void *context)
{
    record_cleanup_lock();
    fake_port_get_ops()->reset_rx_position(context);
}

static csp_rs485_port_result_t test_port_transmit_frame(
    void *context,
    const uint8_t *frame,
    size_t frame_length,
    uint32_t timeout_ms)
{
    return fake_port_get_ops()->transmit_frame(
        context,
        frame,
        frame_length,
        timeout_ms);
}

static const csp_rs485_port_ops_t test_port_ops = {
    .initialize = test_port_initialize,
    .arm_receive = test_port_arm_receive,
    .enable_irqs = test_port_enable_irqs,
    .disable_and_clear_irqs = test_port_disable_and_clear_irqs,
    .abort_receive = test_port_abort_receive,
    .deinitialize = test_port_deinitialize,
    .force_receive_mode = test_port_force_receive_mode,
    .reset_rx_position = test_port_reset_rx_position,
    .transmit_frame = test_port_transmit_frame,
};

static csp_packet_t *make_empty_packet(void)
{
    csp_packet_t *packet = csp_buffer_get(0U);
    if (packet != NULL) {
        packet->id.ext = 0x12345678U;
        packet->length = 0U;
    }
    return packet;
}

static int transmit_packet(csp_packet_t *packet)
{
    csp_route_t route = {
        .iface = csp_rs485_link_get_interface(),
        .via = CSP_NO_VIA_ADDRESS,
    };
    return route.iface->nexthop(&route, packet);
}

static void notify_rx_during_retry_wait(void)
{
    csp_rs485_freertos_rx_from_isr(&retry_wait_byte, 1U);
}

static void notify_stop_during_retry_wait(void)
{
    csp_rs485_freertos_request_stop();
}

static void notify_rx_during_runtime_teardown(void)
{
    csp_rs485_freertos_rx_from_isr(&teardown_byte, 1U);
}

static void update_health_during_snapshot(void)
{
    csp_rs485_freertos_rx_from_isr(
        health_interleave_bytes,
        sizeof(health_interleave_bytes));
}

static void inject_second_gap_after_task_frees_stream_space(void)
{
    csp_rs485_freertos_rx_from_isr(
        second_gap_bytes,
        sizeof(second_gap_bytes));
}

static void inject_isr_fault_after_recovery_running(void)
{
    csp_rs485_link_report_fault_from_isr(CSP_RS485_FAULT_UART);
}

static void inject_saturating_gap_after_task_frees_stream_space(void)
{
    csp_rs485_freertos_rx_from_isr(
        second_gap_bytes,
        sizeof(second_gap_bytes));
    if (saturated_gap_injections_remaining > 0U) {
        --saturated_gap_injections_remaining;
    }
    if (saturated_gap_injections_remaining > 0U) {
        fake_freertos_set_next_stream_receive_hook(
            inject_saturating_gap_after_task_frees_stream_space);
    }
}

static void inject_gap_after_task_observes_empty_stream(void)
{
    csp_rs485_freertos_rx_from_isr(
        exit_window_gap_bytes,
        sizeof(exit_window_gap_bytes));
}

static void run_runtime_iteration_during_task_create(void)
{
    create_preemption_drained = csp_rs485_freertos_run_once();
}

static void release_in_progress_tx_mutex(void)
{
    cleanup_operations_before_mutex_release = cleanup_operation_count;
    csp_rs485_freertos_give_tx_mutex();
}

static void fault_then_release_tx_mutex(void)
{
    csp_rs485_link_report_fault(CSP_RS485_FAULT_DMA);
    csp_rs485_freertos_give_tx_mutex();
}

static void attempt_tx_then_release_for_deinit(void)
{
    cleanup_operations_before_mutex_release = cleanup_operation_count;
    csp_rs485_link_report_fault(CSP_RS485_FAULT_DMA);
    csp_rs485_freertos_give_tx_mutex();
    recovery_during_deinit_result = csp_rs485_link_recovery_step();
    csp_packet_t *packet = make_empty_packet();
    if (packet == NULL) {
        tx_during_deinit_result = CSP_ERR_NOMEM;
    } else {
        tx_during_deinit_result = transmit_packet(packet);
        if (tx_during_deinit_result != CSP_ERR_NONE) {
            csp_buffer_free(packet);
        }
    }
}

static bool build_frame(
    uint32_t id,
    const uint8_t *payload,
    size_t payload_length,
    uint8_t *frame,
    size_t *frame_length)
{
    csp_packet_t *packet =
        csp_buffer_get(payload_length + CSP_RS485_CSP_CRC_SIZE);
    if (packet == NULL) {
        return false;
    }

    packet->id.ext = id;
    packet->length = (uint16_t) payload_length;
    if (payload_length > 0U) {
        memcpy(packet->data, payload, payload_length);
    }
    if (csp_crc32_append(packet, false) != CSP_ERR_NONE) {
        csp_buffer_free(packet);
        return false;
    }

    const csp_rs485_kiss_encode_result_t result = csp_rs485_kiss_encode(
        packet->id,
        packet->data,
        packet->length,
        frame,
        CSP_RS485_TX_FRAME_MAX,
        frame_length);
    csp_buffer_free(packet);
    return result == CSP_RS485_KISS_ENCODE_OK;
}

static int initialize_link(uint32_t stack_words)
{
    prepare_link();
    const csp_rs485_link_config_t config =
        make_config(stack_words, 25U, fake_port_get_ops());
    return csp_rs485_link_init(&config);
}

static void runtime_creates_only_static_objects(void)
{
    const int init_result =
        initialize_link(CSP_RS485_TASK_STACK_CAPACITY_WORDS);
    fake_freertos_observations_t observations;
    fake_freertos_get_observations(&observations);
    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_EQ_SIZE(1U, observations.static_task_create_calls);
    TEST_ASSERT_EQ_SIZE(1U, observations.static_stream_create_calls);
    TEST_ASSERT_EQ_SIZE(1U, observations.static_mutex_create_calls);
    TEST_ASSERT_EQ_SIZE(
        1U,
        observations.static_binary_semaphore_create_calls);
    TEST_ASSERT_EQ_SIZE(0U, observations.dynamic_allocator_calls);
}

static void runtime_serializes_tx_buffer_and_physical_tx(void)
{
    const int init_result = initialize_link(32U);
    csp_packet_t *packet = csp_buffer_get(0U);
    csp_route_t route = {
        .iface = csp_rs485_link_get_interface(),
        .via = CSP_NO_VIA_ADDRESS,
    };
    TEST_ASSERT_TRUE(packet != NULL);
    packet->id.ext = 0x12345678U;
    packet->length = 0U;
    const int tx_result = route.iface->nexthop(&route, packet);
    fake_freertos_observations_t observations;
    fake_freertos_get_observations(&observations);
    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, tx_result);
    TEST_ASSERT_EQ_SIZE(1U, fake.transmit_call_count);
    TEST_ASSERT_EQ_SIZE(1U, observations.mutex_take_calls);
    TEST_ASSERT_EQ_SIZE(1U, observations.mutex_give_calls);
}

static void runtime_isr_accepts_complete_span(void)
{
    static const uint8_t bytes[] = {0xC0U, 0x00U, 0xC0U};
    const int init_result = initialize_link(32U);
    csp_rs485_freertos_rx_from_isr(bytes, sizeof(bytes));
    const size_t drained = csp_rs485_freertos_run_once();
    csp_rs485_health_t health;
    csp_rs485_link_get_health(&health);
    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_EQ_SIZE(sizeof(bytes), drained);
    TEST_ASSERT_EQ_SIZE(0U, health.stream_dropped_bytes);
}

static void runtime_isr_records_partial_stream_write(void)
{
    uint8_t bytes[CSP_RS485_STREAM_BUFFER_SIZE + 1U] = {0};
    const int init_result = initialize_link(32U);
    csp_rs485_freertos_rx_from_isr(bytes, sizeof(bytes));
    csp_rs485_health_t health;
    csp_rs485_link_get_health(&health);
    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_EQ_SIZE(1U, health.stream_dropped_bytes);
}

static void runtime_overflow_marks_one_discontinuity(void)
{
    uint8_t bytes[CSP_RS485_STREAM_BUFFER_SIZE + 1U] = {0};
    const int init_result = initialize_link(32U);
    csp_rs485_freertos_rx_from_isr(bytes, sizeof(bytes));
    csp_rs485_freertos_rx_from_isr(bytes, sizeof(bytes));
    csp_rs485_health_t health;
    csp_rs485_link_get_health(&health);
    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_EQ_SIZE(1U, health.stream_discontinuities);
}

static void runtime_tracks_stream_high_watermark(void)
{
    uint8_t bytes[17U] = {0};
    const int init_result = initialize_link(32U);
    csp_rs485_freertos_rx_from_isr(bytes, sizeof(bytes));
    csp_rs485_health_t health;
    csp_rs485_link_get_health(&health);
    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_EQ_SIZE(sizeof(bytes), health.stream_high_watermark);
}

static void runtime_task_drains_arbitrary_chunks(void)
{
    static const uint8_t first[] = {0xC0U, 0x00U};
    static const uint8_t second[] = {0x11U, 0x22U, 0xC0U};
    const int init_result = initialize_link(32U);
    csp_rs485_freertos_rx_from_isr(first, sizeof(first));
    csp_rs485_freertos_rx_from_isr(second, sizeof(second));
    const size_t drained = csp_rs485_freertos_run_once();
    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_EQ_SIZE(sizeof(first) + sizeof(second), drained);
}

static void runtime_fault_notification_runs_one_recovery_attempt(void)
{
    const int init_result = initialize_link(32U);
    csp_rs485_link_report_fault(CSP_RS485_FAULT_UART);
    (void) csp_rs485_freertos_run_once();
    csp_rs485_health_t health;
    csp_rs485_link_get_health(&health);
    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_EQ_SIZE(1U, health.recovery_attempts);
}

static void runtime_initial_initialize_failure_retries_and_recovers(void)
{
    prepare_link();
    fake.initialize_result = CSP_RS485_PORT_ERROR;
    const csp_rs485_link_config_t config =
        make_config(32U, 25U, fake_port_get_ops());

    const int init_result = csp_rs485_link_init(&config);
    csp_rs485_health_t recovering;
    csp_rs485_link_get_health(&recovering);
    fake.initialize_result = CSP_RS485_PORT_OK;
    (void) csp_rs485_freertos_run_once();
    csp_rs485_health_t recovered;
    csp_rs485_link_get_health(&recovered);
    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_EQ_SIZE(CSP_RS485_LINK_RECOVERING, recovering.state);
    TEST_ASSERT_EQ_SIZE(CSP_RS485_LINK_RUNNING, recovered.state);
    TEST_ASSERT_EQ_SIZE(1U, recovered.recovery_attempts);
    TEST_ASSERT_EQ_SIZE(1U, recovered.recovery_successes);
}

static void runtime_initial_arm_failure_retries_and_recovers(void)
{
    prepare_link();
    fake.arm_receive_result = CSP_RS485_PORT_ERROR;
    const csp_rs485_link_config_t config =
        make_config(32U, 25U, fake_port_get_ops());

    const int init_result = csp_rs485_link_init(&config);
    csp_rs485_health_t recovering;
    csp_rs485_link_get_health(&recovering);
    fake.arm_receive_result = CSP_RS485_PORT_OK;
    (void) csp_rs485_freertos_run_once();
    csp_rs485_health_t recovered;
    csp_rs485_link_get_health(&recovered);
    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_EQ_SIZE(CSP_RS485_LINK_RECOVERING, recovering.state);
    TEST_ASSERT_EQ_SIZE(CSP_RS485_LINK_RUNNING, recovered.state);
    TEST_ASSERT_EQ_SIZE(1U, recovered.recovery_attempts);
    TEST_ASSERT_EQ_SIZE(1U, recovered.recovery_successes);
}

static void runtime_failed_recovery_waits_configured_ticks(void)
{
    const int init_result = initialize_link(32U);
    fake.initialize_result = CSP_RS485_PORT_ERROR;
    csp_rs485_link_report_fault(CSP_RS485_FAULT_DMA);
    (void) csp_rs485_freertos_run_once();
    fake_freertos_observations_t observations;
    fake_freertos_get_observations(&observations);
    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_EQ_SIZE(2U, observations.last_wait_ticks);
}

static void runtime_stop_notification_cancels_wait(void)
{
    const int init_result = initialize_link(32U);
    fake.initialize_result = CSP_RS485_PORT_ERROR;
    csp_rs485_link_report_fault(CSP_RS485_FAULT_DMA);
    (void) csp_rs485_freertos_run_once();
    csp_rs485_freertos_request_stop();
    (void) csp_rs485_freertos_run_once();
    csp_rs485_health_t health;
    csp_rs485_link_get_health(&health);
    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_EQ_SIZE(CSP_RS485_LINK_RECOVERING, health.state);
    TEST_ASSERT_EQ_SIZE(1U, health.recovery_attempts);
}

static void runtime_health_snapshot_is_coherent(void)
{
    uint8_t bytes[CSP_RS485_STREAM_BUFFER_SIZE + 2U] = {0};
    const int init_result = initialize_link(32U);
    csp_rs485_freertos_rx_from_isr(bytes, sizeof(bytes));
    csp_rs485_health_t health;
    csp_rs485_link_get_health(&health);
    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_EQ_SIZE(2U, health.stream_dropped_bytes);
    TEST_ASSERT_EQ_SIZE(1U, health.stream_discontinuities);
    TEST_ASSERT_EQ_SIZE(
        CSP_RS485_STREAM_BUFFER_SIZE,
        health.stream_high_watermark);
}

static void runtime_task_blocks_until_external_delete(void)
{
    const int init_result = initialize_link(32U);
    csp_rs485_freertos_request_stop();
    const bool blocked = fake_freertos_run_task_until_blocked();
    csp_rs485_link_deinit();
    fake_freertos_observations_t observations;
    fake_freertos_get_observations(&observations);

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_TRUE(blocked);
    TEST_ASSERT_EQ_SIZE(1U, observations.task_block_calls);
    TEST_ASSERT_EQ_SIZE(0U, observations.self_delete_calls);
    TEST_ASSERT_EQ_SIZE(1U, observations.non_null_delete_calls);
    TEST_ASSERT_EQ_SIZE(1U, observations.binary_semaphore_take_calls);
    TEST_ASSERT_EQ_SIZE(0U, observations.binary_semaphore_wait_calls);
    TEST_ASSERT_EQ_SIZE(1U, observations.binary_semaphore_give_calls);
}

static void runtime_isr_fault_notification_uses_isr_api_and_yields_as_requested(void)
{
    const int init_result = initialize_link(32U);
    fake_freertos_clear_observations();
    fake_freertos_set_isr_task_woken(false);
    csp_rs485_link_report_fault_from_isr(CSP_RS485_FAULT_UART);
    fake_freertos_observations_t without_yield;
    fake_freertos_get_observations(&without_yield);

    fake_freertos_clear_observations();
    fake_freertos_set_isr_task_woken(true);
    csp_rs485_link_report_fault_from_isr(CSP_RS485_FAULT_DMA);
    fake_freertos_observations_t with_yield;
    fake_freertos_get_observations(&with_yield);
    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_EQ_SIZE(1U, without_yield.notify_from_isr_calls);
    TEST_ASSERT_EQ_SIZE(0U, without_yield.task_notify_calls);
    TEST_ASSERT_EQ_SIZE(0U, without_yield.yield_calls);
    TEST_ASSERT_EQ_SIZE(1U, with_yield.notify_from_isr_calls);
    TEST_ASSERT_EQ_SIZE(0U, with_yield.task_notify_calls);
    TEST_ASSERT_EQ_SIZE(1U, with_yield.yield_calls);
}

static void runtime_overflow_resets_parser_after_accepted_prefix(void)
{
    static const uint8_t payload[] = {0x10U, 0x20U};
    uint8_t valid_frame[CSP_RS485_TX_FRAME_MAX];
    uint8_t buffered[CSP_RS485_STREAM_BUFFER_SIZE];
    uint8_t dropped_byte = 0x55U;
    size_t valid_length = 0U;
    const int init_result = initialize_link(32U);
    const bool frame_result = build_frame(
        0x10203040U,
        payload,
        sizeof(payload),
        valid_frame,
        &valid_length);

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_TRUE(frame_result);
    TEST_ASSERT_TRUE(valid_length > 6U);

    memset(buffered, 0xC0, sizeof(buffered) - 6U);
    memcpy(
        &buffered[sizeof(buffered) - 6U],
        valid_frame,
        6U);
    csp_rs485_freertos_rx_from_isr(buffered, sizeof(buffered));
    csp_rs485_freertos_rx_from_isr(&dropped_byte, 1U);
    (void) csp_rs485_freertos_run_once();
    csp_rs485_freertos_rx_from_isr(valid_frame, valid_length);
    (void) csp_rs485_freertos_run_once();
    csp_rs485_health_t health;
    csp_rs485_link_get_health(&health);
    const csp_kiss_interface_data_t *kiss_state =
        csp_rs485_link_get_interface()->interface_data;
    const csp_kiss_mode_t kiss_mode = kiss_state->rx_mode;
    const csp_packet_t *kiss_packet = kiss_state->rx_packet;
    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(0U, health.protocol_errors);
    TEST_ASSERT_EQ_SIZE(KISS_MODE_NOT_STARTED, kiss_mode);
    TEST_ASSERT_TRUE(kiss_packet == NULL);
}

static void runtime_cursor_gap_is_ordered_between_queued_spans(void)
{
    static const uint8_t payload[] = {0x31U, 0x32U};
    uint8_t valid_frame[CSP_RS485_TX_FRAME_MAX];
    size_t valid_length = 0U;
    const int init_result = initialize_link(32U);
    const bool frame_result = build_frame(
        0x10203040U,
        payload,
        sizeof(payload),
        valid_frame,
        &valid_length);

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_TRUE(frame_result);
    TEST_ASSERT_TRUE(valid_length > 6U);

    const uint32_t frames_before =
        csp_rs485_link_get_interface()->frame;
    csp_rs485_freertos_rx_from_isr(valid_frame, 6U);
    csp_rs485_link_mark_rx_discontinuity_from_isr();
    csp_rs485_freertos_rx_from_isr(
        &valid_frame[6U],
        valid_length - 6U);
    (void) csp_rs485_freertos_run_once();
    const uint32_t frames_after =
        csp_rs485_link_get_interface()->frame;
    csp_rs485_health_t health;
    csp_rs485_link_get_health(&health);
    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(frames_before, frames_after);
    TEST_ASSERT_EQ_SIZE(1U, health.stream_discontinuities);
}

static void runtime_cursor_gap_without_more_bytes_wakes_and_resets(void)
{
    static const uint8_t payload[] = {0x41U, 0x42U};
    uint8_t valid_frame[CSP_RS485_TX_FRAME_MAX];
    size_t valid_length = 0U;
    const int init_result = initialize_link(32U);
    const bool frame_result = build_frame(
        0x10203040U,
        payload,
        sizeof(payload),
        valid_frame,
        &valid_length);

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_TRUE(frame_result);
    TEST_ASSERT_TRUE(valid_length > 6U);

    csp_rs485_freertos_rx_from_isr(valid_frame, 6U);
    (void) csp_rs485_freertos_run_once();
    const csp_kiss_interface_data_t *kiss_state =
        csp_rs485_link_get_interface()->interface_data;
    TEST_ASSERT_TRUE(kiss_state->rx_packet != NULL);

    fake_freertos_clear_observations();
    csp_rs485_link_mark_rx_discontinuity_from_isr();
    fake_freertos_observations_t observations;
    fake_freertos_get_observations(&observations);
    (void) csp_rs485_freertos_run_once();
    const csp_packet_t *packet_after_gap = kiss_state->rx_packet;
    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(1U, observations.notify_from_isr_calls);
    TEST_ASSERT_TRUE(packet_after_gap == NULL);
}

static void runtime_retry_wait_ignores_rx_until_full_interval(void)
{
    const int init_result = initialize_link(32U);
    fake.initialize_result = CSP_RS485_PORT_ERROR;
    csp_rs485_link_report_fault(CSP_RS485_FAULT_DMA);
    fake_freertos_set_next_wait_hook(notify_rx_during_retry_wait, 1U);
    (void) csp_rs485_freertos_run_once();
    fake_freertos_observations_t observations;
    fake_freertos_get_observations(&observations);
    csp_rs485_health_t health;
    csp_rs485_link_get_health(&health);
    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_EQ_SIZE(1U, health.recovery_attempts);
    TEST_ASSERT_EQ_SIZE(1U, observations.last_wait_ticks);
}

static void runtime_stop_during_retry_wait_cancels_recovery(void)
{
    const int init_result = initialize_link(32U);
    fake.initialize_result = CSP_RS485_PORT_ERROR;
    csp_rs485_link_report_fault(CSP_RS485_FAULT_DMA);
    fake_freertos_set_next_wait_hook(notify_stop_during_retry_wait, 5U);
    (void) csp_rs485_freertos_run_once();
    (void) csp_rs485_freertos_run_once();
    csp_rs485_health_t health;
    csp_rs485_link_get_health(&health);
    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_EQ_SIZE(1U, health.recovery_attempts);
}

static void runtime_health_snapshot_defers_isr_update_until_after_copy(void)
{
    const int init_result = initialize_link(32U);
    fake_freertos_set_critical_exit_hook(update_health_during_snapshot);
    csp_rs485_health_t before;
    csp_rs485_link_get_health(&before);
    fake_freertos_observations_t observations;
    fake_freertos_get_observations(&observations);
    csp_rs485_health_t after;
    csp_rs485_link_get_health(&after);
    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_EQ_SIZE(1U, observations.critical_exit_hook_calls);
    TEST_ASSERT_EQ_SIZE(0U, before.stream_dropped_bytes);
    TEST_ASSERT_EQ_SIZE(2U, after.stream_dropped_bytes);
    TEST_ASSERT_EQ_SIZE(1U, after.stream_discontinuities);
    TEST_ASSERT_EQ_SIZE(
        CSP_RS485_STREAM_BUFFER_SIZE,
        after.stream_high_watermark);
}

static void runtime_quiesces_producer_before_handle_teardown(void)
{
    const int init_result = initialize_link(32U);
    fake_freertos_clear_observations();
    fake_freertos_set_delete_hook(notify_rx_during_runtime_teardown);
    csp_rs485_link_deinit();
    fake_freertos_observations_t observations;
    fake_freertos_get_observations(&observations);

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_EQ_SIZE(0U, observations.stream_send_from_isr_calls);
}

static void runtime_isr_fault_defers_health_transition_to_task(void)
{
    const int init_result = initialize_link(32U);
    csp_rs485_link_report_fault_from_isr(CSP_RS485_FAULT_UART);
    csp_rs485_health_t before_task;
    csp_rs485_link_get_health(&before_task);
    (void) csp_rs485_freertos_run_once();
    csp_rs485_health_t after_task;
    csp_rs485_link_get_health(&after_task);
    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_EQ_SIZE(CSP_RS485_LINK_RUNNING, before_task.state);
    TEST_ASSERT_EQ_SIZE(0U, before_task.uart_errors);
    TEST_ASSERT_EQ_SIZE(CSP_RS485_LINK_RUNNING, after_task.state);
    TEST_ASSERT_EQ_SIZE(1U, after_task.uart_errors);
    TEST_ASSERT_EQ_SIZE(1U, after_task.recovery_attempts);
}

static void runtime_tracks_two_gaps_after_task_frees_stream_space(void)
{
    static const uint8_t payload[] = {0x31U, 0x32U};
    uint8_t valid_frame[CSP_RS485_TX_FRAME_MAX];
    uint8_t first_gap_bytes[CSP_RS485_STREAM_BUFFER_SIZE];
    uint8_t dropped_byte = 0x55U;
    size_t valid_length = 0U;
    const int init_result = initialize_link(32U);
    const bool frame_result = build_frame(
        0x10203040U,
        payload,
        sizeof(payload),
        valid_frame,
        &valid_length);

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_TRUE(frame_result);
    TEST_ASSERT_TRUE(valid_length > 6U);

    memset(first_gap_bytes, 0xC0, sizeof(first_gap_bytes) - 6U);
    memcpy(
        &first_gap_bytes[sizeof(first_gap_bytes) - 6U],
        valid_frame,
        6U);
    memset(second_gap_bytes, 0xC0, sizeof(second_gap_bytes) - 7U);
    memcpy(
        &second_gap_bytes[sizeof(second_gap_bytes) - 7U],
        valid_frame,
        6U);
    second_gap_bytes[sizeof(second_gap_bytes) - 1U] = dropped_byte;

    csp_rs485_freertos_rx_from_isr(
        first_gap_bytes,
        sizeof(first_gap_bytes));
    csp_rs485_freertos_rx_from_isr(&dropped_byte, 1U);
    fake_freertos_set_next_stream_receive_hook(
        inject_second_gap_after_task_frees_stream_space);
    (void) csp_rs485_freertos_run_once();
    csp_rs485_freertos_rx_from_isr(valid_frame, valid_length);
    (void) csp_rs485_freertos_run_once();
    csp_rs485_health_t health;
    csp_rs485_link_get_health(&health);
    const csp_kiss_interface_data_t *kiss_state =
        csp_rs485_link_get_interface()->interface_data;
    const csp_kiss_mode_t kiss_mode = kiss_state->rx_mode;
    const csp_packet_t *kiss_packet = kiss_state->rx_packet;
    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(2U, health.stream_dropped_bytes);
    TEST_ASSERT_EQ_SIZE(2U, health.stream_discontinuities);
    TEST_ASSERT_EQ_SIZE(0U, health.protocol_errors);
    TEST_ASSERT_EQ_SIZE(KISS_MODE_NOT_STARTED, kiss_mode);
    TEST_ASSERT_TRUE(kiss_packet == NULL);
}

static void runtime_isr_fault_during_recovery_never_reopens_tx(void)
{
    const int init_result = initialize_link(32U);
    csp_rs485_link_report_fault(CSP_RS485_FAULT_DMA);
    fake_freertos_set_critical_exit_hook_after(
        10U,
        inject_isr_fault_after_recovery_running);
    (void) csp_rs485_freertos_run_once();
    csp_packet_t *packet = csp_buffer_get(0U);
    csp_route_t route = {
        .iface = csp_rs485_link_get_interface(),
        .via = CSP_NO_VIA_ADDRESS,
    };
    TEST_ASSERT_TRUE(packet != NULL);
    packet->id.ext = 0x12345678U;
    packet->length = 0U;
    const int tx_result = route.iface->nexthop(&route, packet);
    if (tx_result != CSP_ERR_NONE) {
        csp_buffer_free(packet);
    }
    (void) csp_rs485_freertos_run_once();
    csp_rs485_health_t health;
    csp_rs485_link_get_health(&health);
    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_EQ_SIZE(CSP_ERR_TX, tx_result);
    TEST_ASSERT_EQ_SIZE(1U, health.uart_errors);
    TEST_ASSERT_EQ_SIZE(2U, health.recovery_attempts);
}

static void runtime_saturated_gap_tracking_discards_until_clean_resync(void)
{
    static const uint8_t payload[] = {0x41U, 0x42U};
    uint8_t valid_frame[CSP_RS485_TX_FRAME_MAX];
    uint8_t first_gap_bytes[CSP_RS485_STREAM_BUFFER_SIZE];
    uint8_t dropped_byte = 0x55U;
    size_t valid_length = 0U;
    const int init_result = initialize_link(32U);
    const bool frame_result = build_frame(
        0x10203040U,
        payload,
        sizeof(payload),
        valid_frame,
        &valid_length);

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_TRUE(frame_result);
    TEST_ASSERT_TRUE(valid_length > 6U);

    memset(first_gap_bytes, 0xC0, sizeof(first_gap_bytes) - 6U);
    memcpy(
        &first_gap_bytes[sizeof(first_gap_bytes) - 6U],
        valid_frame,
        6U);
    memset(second_gap_bytes, 0xC0, sizeof(second_gap_bytes) - 7U);
    memcpy(
        &second_gap_bytes[sizeof(second_gap_bytes) - 7U],
        valid_frame,
        6U);
    second_gap_bytes[sizeof(second_gap_bytes) - 1U] = dropped_byte;

    saturated_gap_injections_remaining = 4U;
    csp_rs485_freertos_rx_from_isr(
        first_gap_bytes,
        sizeof(first_gap_bytes));
    csp_rs485_freertos_rx_from_isr(&dropped_byte, 1U);
    fake_freertos_set_next_stream_receive_hook(
        inject_saturating_gap_after_task_frees_stream_space);
    (void) csp_rs485_freertos_run_once();
    csp_rs485_freertos_rx_from_isr(valid_frame, valid_length);
    (void) csp_rs485_freertos_run_once();
    csp_rs485_health_t health;
    csp_rs485_link_get_health(&health);
    const csp_kiss_interface_data_t *kiss_state =
        csp_rs485_link_get_interface()->interface_data;
    const csp_kiss_mode_t kiss_mode = kiss_state->rx_mode;
    const csp_packet_t *kiss_packet = kiss_state->rx_packet;
    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(5U, health.stream_dropped_bytes);
    TEST_ASSERT_EQ_SIZE(5U, health.stream_discontinuities);
    TEST_ASSERT_EQ_SIZE(0U, health.protocol_errors);
    TEST_ASSERT_EQ_SIZE(KISS_MODE_NOT_STARTED, kiss_mode);
    TEST_ASSERT_TRUE(kiss_packet == NULL);
}

static void runtime_saturated_discard_exit_preserves_concurrent_gap(void)
{
    static const uint8_t payload[] = {0x51U, 0x52U};
    uint8_t valid_frame[CSP_RS485_TX_FRAME_MAX];
    uint8_t first_gap_bytes[CSP_RS485_STREAM_BUFFER_SIZE];
    uint8_t dropped_byte = 0x55U;
    size_t valid_length = 0U;
    const int init_result = initialize_link(32U);
    const bool frame_result = build_frame(
        0x10203040U,
        payload,
        sizeof(payload),
        valid_frame,
        &valid_length);

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_TRUE(frame_result);
    TEST_ASSERT_TRUE(valid_length > 6U);

    memset(first_gap_bytes, 0xC0, sizeof(first_gap_bytes) - 6U);
    memcpy(
        &first_gap_bytes[sizeof(first_gap_bytes) - 6U],
        valid_frame,
        6U);
    memset(second_gap_bytes, 0xC0, sizeof(second_gap_bytes) - 1U);
    second_gap_bytes[sizeof(second_gap_bytes) - 1U] = dropped_byte;
    memset(exit_window_gap_bytes, 0xC0, sizeof(exit_window_gap_bytes));
    memcpy(
        &exit_window_gap_bytes[CSP_RS485_STREAM_BUFFER_SIZE - 6U],
        valid_frame,
        6U);
    exit_window_gap_bytes[CSP_RS485_STREAM_BUFFER_SIZE] = dropped_byte;

    const uint32_t frames_before =
        csp_rs485_link_get_interface()->frame;
    saturated_gap_injections_remaining = 4U;
    csp_rs485_freertos_rx_from_isr(
        first_gap_bytes,
        sizeof(first_gap_bytes));
    csp_rs485_freertos_rx_from_isr(&dropped_byte, 1U);
    fake_freertos_set_next_stream_receive_hook(
        inject_saturating_gap_after_task_frees_stream_space);
    fake_freertos_set_empty_stream_receive_hook(
        inject_gap_after_task_observes_empty_stream);
    (void) csp_rs485_freertos_run_once();
    (void) csp_rs485_freertos_run_once();

    csp_rs485_freertos_rx_from_isr(
        &valid_frame[6U],
        valid_length - 6U);
    (void) csp_rs485_freertos_run_once();
    const uint32_t frames_after_gap_suffix =
        csp_rs485_link_get_interface()->frame;

    csp_rs485_freertos_rx_from_isr(valid_frame, valid_length);
    (void) csp_rs485_freertos_run_once();
    const uint32_t frames_after_clean_frame =
        csp_rs485_link_get_interface()->frame;
    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(frames_before, frames_after_gap_suffix);
    TEST_ASSERT_EQ_SIZE(frames_before + 1U, frames_after_clean_frame);
}

static void runtime_task_create_preemption_blocks_without_published_handle(void)
{
    prepare_link();
    const csp_rs485_link_config_t config =
        make_config(32U, 25U, fake_port_get_ops());
    fake_freertos_set_task_create_hook(
        run_runtime_iteration_during_task_create);
    const int init_result = csp_rs485_link_init(&config);
    fake_freertos_observations_t observations;
    fake_freertos_get_observations(&observations);
    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_EQ_SIZE(0U, create_preemption_drained);
    TEST_ASSERT_EQ_SIZE(1U, observations.task_create_hook_calls);
    TEST_ASSERT_EQ_SIZE(1U, observations.notify_wait_calls);
    TEST_ASSERT_EQ_SIZE(UINT32_MAX, observations.last_wait_ticks);
}

static void runtime_stop_notify_preemption_never_deletes_stale_handle(void)
{
    const int init_result = initialize_link(32U);
    fake_freertos_clear_observations();
    fake_freertos_set_task_notify_runs_task(true);
    csp_rs485_link_deinit();
    fake_freertos_observations_t observations;
    fake_freertos_get_observations(&observations);

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_EQ_SIZE(1U, observations.notify_preemption_block_calls);
    TEST_ASSERT_EQ_SIZE(0U, observations.self_delete_calls);
    TEST_ASSERT_EQ_SIZE(1U, observations.non_null_delete_calls);
    TEST_ASSERT_EQ_SIZE(0U, observations.stale_delete_calls);
    TEST_ASSERT_EQ_SIZE(1U, observations.binary_semaphore_take_calls);
    TEST_ASSERT_EQ_SIZE(0U, observations.binary_semaphore_wait_calls);
    TEST_ASSERT_EQ_SIZE(1U, observations.binary_semaphore_give_calls);
}

static void runtime_deinit_allows_immediate_static_task_reinit(void)
{
    prepare_link();
    const csp_rs485_link_config_t config =
        make_config(32U, 25U, fake_port_get_ops());
    const int first_result = csp_rs485_link_init(&config);
    csp_rs485_link_deinit();

    const int second_result = csp_rs485_link_init(&config);
    csp_rs485_link_deinit();
    fake_freertos_observations_t observations;
    fake_freertos_get_observations(&observations);

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, first_result);
    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, second_result);
    TEST_ASSERT_EQ_SIZE(2U, observations.static_task_create_calls);
    TEST_ASSERT_EQ_SIZE(
        2U,
        observations.static_binary_semaphore_create_calls);
    TEST_ASSERT_EQ_SIZE(2U, observations.non_null_delete_calls);
}

static void runtime_deinit_waits_for_task_safe_point_before_delete(void)
{
    const int init_result = initialize_link(32U);
    fake_freertos_clear_observations();

    csp_rs485_link_deinit();

    fake_freertos_observations_t observations;
    fake_freertos_get_observations(&observations);
    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_EQ_SIZE(1U, observations.binary_semaphore_take_calls);
    TEST_ASSERT_EQ_SIZE(1U, observations.binary_semaphore_wait_calls);
    TEST_ASSERT_EQ_SIZE(1U, observations.binary_semaphore_give_calls);
    TEST_ASSERT_EQ_SIZE(1U, observations.task_block_calls);
    TEST_ASSERT_EQ_SIZE(1U, observations.non_null_delete_calls);
}

static void runtime_isr_ignores_empty_span(void)
{
    const int init_result = initialize_link(32U);
    fake_freertos_clear_observations();

    csp_rs485_freertos_rx_from_isr(NULL, 0U);

    fake_freertos_observations_t observations;
    fake_freertos_get_observations(&observations);
    csp_rs485_health_t health;
    csp_rs485_link_get_health(&health);
    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_EQ_SIZE(0U, observations.stream_send_from_isr_calls);
    TEST_ASSERT_EQ_SIZE(0U, observations.notify_from_isr_calls);
    TEST_ASSERT_EQ_SIZE(0U, health.stream_dropped_bytes);
    TEST_ASSERT_EQ_SIZE(0U, health.stream_discontinuities);
}

static void runtime_is_ready_before_irq_enable_observes_initial_events(void)
{
    prepare_link();
    const csp_rs485_link_config_t config =
        make_config(32U, 25U, &test_port_ops);
    inject_initial_irq_events = true;
    const int init_result = csp_rs485_link_init(&config);
    const size_t drained = csp_rs485_freertos_run_once();
    fake_freertos_observations_t observations;
    fake_freertos_get_observations(&observations);
    csp_rs485_health_t health;
    csp_rs485_link_get_health(&health);
    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_EQ_SIZE(1U, drained);
    TEST_ASSERT_EQ_SIZE(1U, observations.stream_send_from_isr_calls);
    TEST_ASSERT_EQ_SIZE(2U, observations.notify_from_isr_calls);
    TEST_ASSERT_EQ_SIZE(1U, health.uart_errors);
    TEST_ASSERT_EQ_SIZE(1U, health.recovery_attempts);
}

static void runtime_recovery_waits_for_in_progress_tx_lock(void)
{
    prepare_link();
    const csp_rs485_link_config_t config =
        make_config(32U, 25U, &test_port_ops);
    const int init_result = csp_rs485_link_init(&config);
    const bool owner_locked = csp_rs485_freertos_take_tx_mutex();
    csp_rs485_link_report_fault(CSP_RS485_FAULT_DMA);
    fake_freertos_set_mutex_wait_hook(release_in_progress_tx_mutex);
    (void) csp_rs485_freertos_run_once();
    fake_freertos_observations_t observations;
    fake_freertos_get_observations(&observations);
    if (!fake_freertos_mutex_is_available()) {
        csp_rs485_freertos_give_tx_mutex();
    }
    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_TRUE(owner_locked);
    TEST_ASSERT_EQ_SIZE(1U, observations.mutex_wait_calls);
    TEST_ASSERT_EQ_SIZE(0U, cleanup_operations_before_mutex_release);
    TEST_ASSERT_TRUE(cleanup_operation_count >= 5U);
    TEST_ASSERT_TRUE(!cleanup_observed_unlocked_mutex);
}

static void runtime_tx_waiter_rechecks_state_after_lock(void)
{
    const int init_result = initialize_link(32U);
    const bool owner_locked = csp_rs485_freertos_take_tx_mutex();
    fake_freertos_set_mutex_wait_hook(fault_then_release_tx_mutex);
    csp_packet_t *packet = make_empty_packet();
    TEST_ASSERT_TRUE(packet != NULL);
    const int tx_result = transmit_packet(packet);
    if (tx_result != CSP_ERR_NONE) {
        csp_buffer_free(packet);
    }
    fake_freertos_observations_t observations;
    fake_freertos_get_observations(&observations);
    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_TRUE(owner_locked);
    TEST_ASSERT_EQ_SIZE(1U, observations.mutex_wait_calls);
    TEST_ASSERT_EQ_SIZE(CSP_ERR_TX, tx_result);
    TEST_ASSERT_EQ_SIZE(0U, fake.transmit_call_count);
}

static void runtime_deinit_blocks_tx_and_serializes_cleanup(void)
{
    prepare_link();
    const csp_rs485_link_config_t config =
        make_config(32U, 25U, &test_port_ops);
    const int init_result = csp_rs485_link_init(&config);
    cleanup_operation_count = 0U;
    const bool owner_locked = csp_rs485_freertos_take_tx_mutex();
    fake_freertos_clear_observations();
    fake_freertos_set_mutex_wait_hook(attempt_tx_then_release_for_deinit);
    csp_rs485_link_deinit();
    fake_freertos_observations_t observations;
    fake_freertos_get_observations(&observations);

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_TRUE(owner_locked);
    TEST_ASSERT_EQ_SIZE(1U, observations.mutex_wait_calls);
    TEST_ASSERT_EQ_SIZE(
        CSP_RS485_RECOVERY_CANCELLED,
        recovery_during_deinit_result);
    TEST_ASSERT_EQ_SIZE(CSP_ERR_TX, tx_during_deinit_result);
    TEST_ASSERT_EQ_SIZE(0U, fake.transmit_call_count);
    TEST_ASSERT_EQ_SIZE(0U, cleanup_operations_before_mutex_release);
    TEST_ASSERT_TRUE(cleanup_operation_count >= 5U);
    TEST_ASSERT_TRUE(!cleanup_observed_unlocked_mutex);
}

typedef struct {
    int init_result;
    bool frame_result;
    uint32_t frames_before;
    uint32_t frames_after_gap_suffix;
    uint32_t frames_after_clean_frame;
} gap_wrap_result_t;

static gap_wrap_result_t exercise_gap_wrap(size_t initial_sequence)
{
    static const uint8_t payload[] = {0x61U, 0x62U};
    uint8_t valid_frame[CSP_RS485_TX_FRAME_MAX];
    uint8_t buffered[CSP_RS485_STREAM_BUFFER_SIZE];
    uint8_t dropped_byte = 0x55U;
    size_t valid_length = 0U;
    gap_wrap_result_t result = {
        .init_result = initialize_link(32U),
    };
    result.frame_result = build_frame(
        0x10203040U,
        payload,
        sizeof(payload),
        valid_frame,
        &valid_length);
    if ((result.init_result != CSP_ERR_NONE)
        || (!result.frame_result)
        || (valid_length <= 6U)) {
        csp_rs485_link_deinit();
        return result;
    }

    memset(buffered, 0xC0, sizeof(buffered) - 6U);
    memcpy(&buffered[sizeof(buffered) - 6U], valid_frame, 6U);
    csp_rs485_freertos_test_set_rx_sequence(initial_sequence);
    result.frames_before = csp_rs485_link_get_interface()->frame;
    csp_rs485_freertos_rx_from_isr(buffered, sizeof(buffered));
    csp_rs485_freertos_rx_from_isr(&dropped_byte, 1U);
    (void) csp_rs485_freertos_run_once();

    csp_rs485_freertos_rx_from_isr(
        &valid_frame[6U],
        valid_length - 6U);
    (void) csp_rs485_freertos_run_once();
    result.frames_after_gap_suffix = csp_rs485_link_get_interface()->frame;

    csp_rs485_freertos_rx_from_isr(valid_frame, valid_length);
    (void) csp_rs485_freertos_run_once();
    result.frames_after_clean_frame = csp_rs485_link_get_interface()->frame;
    csp_rs485_link_deinit();
    return result;
}

static void runtime_gap_boundary_before_uint32_wrap_resets_parser(void)
{
    const gap_wrap_result_t result = exercise_gap_wrap(
        SIZE_MAX - CSP_RS485_STREAM_BUFFER_SIZE);

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, result.init_result);
    TEST_ASSERT_TRUE(result.frame_result);
    TEST_ASSERT_EQ_SIZE(
        result.frames_before,
        result.frames_after_gap_suffix);
    TEST_ASSERT_EQ_SIZE(
        result.frames_before + 1U,
        result.frames_after_clean_frame);
}

static void runtime_gap_boundary_after_uint32_wrap_resets_parser(void)
{
    const gap_wrap_result_t result = exercise_gap_wrap(
        SIZE_MAX - (CSP_RS485_STREAM_BUFFER_SIZE - 2U));

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, result.init_result);
    TEST_ASSERT_TRUE(result.frame_result);
    TEST_ASSERT_EQ_SIZE(
        result.frames_before,
        result.frames_after_gap_suffix);
    TEST_ASSERT_EQ_SIZE(
        result.frames_before + 1U,
        result.frames_after_clean_frame);
}

static void runtime_gap_boundary_at_uint32_wrap_records_discontinuity(void)
{
    const gap_wrap_result_t result = exercise_gap_wrap(
        SIZE_MAX - (CSP_RS485_STREAM_BUFFER_SIZE - 1U));

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, result.init_result);
    TEST_ASSERT_TRUE(result.frame_result);
    TEST_ASSERT_EQ_SIZE(
        result.frames_before,
        result.frames_after_gap_suffix);
    TEST_ASSERT_EQ_SIZE(
        result.frames_before + 1U,
        result.frames_after_clean_frame);
}

static void runtime_rejects_zero_tick_recovery_retry(void)
{
    prepare_link();
    const csp_rs485_link_config_t config =
        make_config(32U, 0U, fake_port_get_ops());
    const int init_result = csp_rs485_link_init(&config);
    fake_freertos_observations_t observations;
    fake_freertos_get_observations(&observations);
    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(CSP_ERR_INVAL, init_result);
    TEST_ASSERT_EQ_SIZE(0U, observations.static_task_create_calls);
    TEST_ASSERT_EQ_SIZE(0U, observations.notify_wait_calls);
}

static void runtime_rejects_sub_tick_recovery_retry(void)
{
    prepare_link();
    const csp_rs485_link_config_t config =
        make_config(32U, 1U, fake_port_get_ops());
    const int init_result = csp_rs485_link_init(&config);
    fake_freertos_observations_t observations;
    fake_freertos_get_observations(&observations);
    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(CSP_ERR_INVAL, init_result);
    TEST_ASSERT_EQ_SIZE(0U, observations.static_task_create_calls);
    TEST_ASSERT_EQ_SIZE(0U, observations.notify_wait_calls);
}

static void runtime_rejects_task_stack_above_static_capacity(void)
{
    prepare_link();
    const csp_rs485_link_config_t config = make_config(
        CSP_RS485_TASK_STACK_CAPACITY_WORDS + 1U,
        25U,
        fake_port_get_ops());
    const int init_result = csp_rs485_link_init(&config);
    fake_freertos_observations_t observations;
    fake_freertos_get_observations(&observations);
    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(CSP_ERR_INVAL, init_result);
    TEST_ASSERT_EQ_SIZE(0U, observations.static_task_create_calls);
    TEST_ASSERT_EQ_SIZE(0U, observations.static_stream_create_calls);
    TEST_ASSERT_EQ_SIZE(0U, observations.static_mutex_create_calls);
}

static void runtime_rejects_task_priority_at_kernel_limit(void)
{
    prepare_link();
    csp_rs485_link_config_t config =
        make_config(32U, 25U, fake_port_get_ops());
    config.task_priority = configMAX_PRIORITIES;
    const int init_result = csp_rs485_link_init(&config);
    fake_freertos_observations_t observations;
    fake_freertos_get_observations(&observations);
    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(CSP_ERR_INVAL, init_result);
    TEST_ASSERT_EQ_SIZE(0U, observations.static_task_create_calls);
    TEST_ASSERT_EQ_SIZE(0U, observations.static_stream_create_calls);
    TEST_ASSERT_EQ_SIZE(0U, observations.static_mutex_create_calls);
    TEST_ASSERT_EQ_SIZE(
        0U,
        observations.static_binary_semaphore_create_calls);
}

const test_case_t freertos_runtime_tests[] = {
    {"test_freertos_runtime", "runtime_creates_only_static_objects", runtime_creates_only_static_objects},
    {"test_freertos_runtime", "runtime_serializes_tx_buffer_and_physical_tx", runtime_serializes_tx_buffer_and_physical_tx},
    {"test_freertos_runtime", "runtime_isr_accepts_complete_span", runtime_isr_accepts_complete_span},
    {"test_freertos_runtime", "runtime_isr_records_partial_stream_write", runtime_isr_records_partial_stream_write},
    {"test_freertos_runtime", "runtime_overflow_marks_one_discontinuity", runtime_overflow_marks_one_discontinuity},
    {"test_freertos_runtime", "runtime_tracks_stream_high_watermark", runtime_tracks_stream_high_watermark},
    {"test_freertos_runtime", "runtime_task_drains_arbitrary_chunks", runtime_task_drains_arbitrary_chunks},
    {"test_freertos_runtime", "runtime_fault_notification_runs_one_recovery_attempt", runtime_fault_notification_runs_one_recovery_attempt},
    {"test_freertos_runtime", "runtime_initial_initialize_failure_retries_and_recovers", runtime_initial_initialize_failure_retries_and_recovers},
    {"test_freertos_runtime", "runtime_initial_arm_failure_retries_and_recovers", runtime_initial_arm_failure_retries_and_recovers},
    {"test_freertos_runtime", "runtime_failed_recovery_waits_configured_ticks", runtime_failed_recovery_waits_configured_ticks},
    {"test_freertos_runtime", "runtime_stop_notification_cancels_wait", runtime_stop_notification_cancels_wait},
    {"test_freertos_runtime", "runtime_health_snapshot_is_coherent", runtime_health_snapshot_is_coherent},
    {"test_freertos_runtime", "runtime_task_blocks_until_external_delete", runtime_task_blocks_until_external_delete},
    {"test_freertos_runtime", "runtime_isr_fault_notification_uses_isr_api_and_yields_as_requested", runtime_isr_fault_notification_uses_isr_api_and_yields_as_requested},
    {"test_freertos_runtime", "runtime_overflow_resets_parser_after_accepted_prefix", runtime_overflow_resets_parser_after_accepted_prefix},
    {"test_freertos_runtime", "runtime_cursor_gap_is_ordered_between_queued_spans", runtime_cursor_gap_is_ordered_between_queued_spans},
    {"test_freertos_runtime", "runtime_cursor_gap_without_more_bytes_wakes_and_resets", runtime_cursor_gap_without_more_bytes_wakes_and_resets},
    {"test_freertos_runtime", "runtime_retry_wait_ignores_rx_until_full_interval", runtime_retry_wait_ignores_rx_until_full_interval},
    {"test_freertos_runtime", "runtime_stop_during_retry_wait_cancels_recovery", runtime_stop_during_retry_wait_cancels_recovery},
    {"test_freertos_runtime", "runtime_health_snapshot_defers_isr_update_until_after_copy", runtime_health_snapshot_defers_isr_update_until_after_copy},
    {"test_freertos_runtime", "runtime_quiesces_producer_before_handle_teardown", runtime_quiesces_producer_before_handle_teardown},
    {"test_freertos_runtime", "runtime_isr_fault_defers_health_transition_to_task", runtime_isr_fault_defers_health_transition_to_task},
    {"test_freertos_runtime", "runtime_tracks_two_gaps_after_task_frees_stream_space", runtime_tracks_two_gaps_after_task_frees_stream_space},
    {"test_freertos_runtime", "runtime_isr_fault_during_recovery_never_reopens_tx", runtime_isr_fault_during_recovery_never_reopens_tx},
    {"test_freertos_runtime", "runtime_saturated_gap_tracking_discards_until_clean_resync", runtime_saturated_gap_tracking_discards_until_clean_resync},
    {"test_freertos_runtime", "runtime_saturated_discard_exit_preserves_concurrent_gap", runtime_saturated_discard_exit_preserves_concurrent_gap},
    {"test_freertos_runtime", "runtime_task_create_preemption_blocks_without_published_handle", runtime_task_create_preemption_blocks_without_published_handle},
    {"test_freertos_runtime", "runtime_stop_notify_preemption_never_deletes_stale_handle", runtime_stop_notify_preemption_never_deletes_stale_handle},
    {"test_freertos_runtime", "runtime_deinit_allows_immediate_static_task_reinit", runtime_deinit_allows_immediate_static_task_reinit},
    {"test_freertos_runtime", "runtime_deinit_waits_for_task_safe_point_before_delete", runtime_deinit_waits_for_task_safe_point_before_delete},
    {"test_freertos_runtime", "runtime_isr_ignores_empty_span", runtime_isr_ignores_empty_span},
    {"test_freertos_runtime", "runtime_is_ready_before_irq_enable_observes_initial_events", runtime_is_ready_before_irq_enable_observes_initial_events},
    {"test_freertos_runtime", "runtime_recovery_waits_for_in_progress_tx_lock", runtime_recovery_waits_for_in_progress_tx_lock},
    {"test_freertos_runtime", "runtime_tx_waiter_rechecks_state_after_lock", runtime_tx_waiter_rechecks_state_after_lock},
    {"test_freertos_runtime", "runtime_deinit_blocks_tx_and_serializes_cleanup", runtime_deinit_blocks_tx_and_serializes_cleanup},
    {"test_freertos_runtime", "runtime_gap_boundary_before_uint32_wrap_resets_parser", runtime_gap_boundary_before_uint32_wrap_resets_parser},
    {"test_freertos_runtime", "runtime_gap_boundary_after_uint32_wrap_resets_parser", runtime_gap_boundary_after_uint32_wrap_resets_parser},
    {"test_freertos_runtime", "runtime_gap_boundary_at_uint32_wrap_records_discontinuity", runtime_gap_boundary_at_uint32_wrap_records_discontinuity},
    {"test_freertos_runtime", "runtime_rejects_zero_tick_recovery_retry", runtime_rejects_zero_tick_recovery_retry},
    {"test_freertos_runtime", "runtime_rejects_sub_tick_recovery_retry", runtime_rejects_sub_tick_recovery_retry},
    {"test_freertos_runtime", "runtime_rejects_task_stack_above_static_capacity", runtime_rejects_task_stack_above_static_capacity},
    {"test_freertos_runtime", "runtime_rejects_task_priority_at_kernel_limit", runtime_rejects_task_priority_at_kernel_limit},
};

const size_t freertos_runtime_test_count =
    sizeof(freertos_runtime_tests) / sizeof(freertos_runtime_tests[0]);
