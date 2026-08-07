#include "fakes/samv71/fake_samv71_hw.h"
#include "support/test.h"

#include "csp_rs485_internal.h"

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static bool capture_link_boundary;
static uint8_t forwarded_bytes[FAKE_SAMV71_CAPACITY];
static size_t forwarded_count;
static size_t discontinuity_count;
static csp_rs485_fault_t reported_faults[FAKE_SAMV71_CAPACITY];
static size_t reported_fault_count;

void __real_csp_rs485_freertos_rx_from_isr(
    const uint8_t *bytes,
    size_t length);
void __real_csp_rs485_freertos_mark_rx_discontinuity_from_isr(void);
void __real_csp_rs485_link_report_fault_from_isr(csp_rs485_fault_t fault);

void __wrap_csp_rs485_freertos_rx_from_isr(
    const uint8_t *bytes,
    size_t length)
{
    if (!capture_link_boundary) {
        __real_csp_rs485_freertos_rx_from_isr(bytes, length);
        return;
    }

    if ((bytes != NULL)
        && (length <= (FAKE_SAMV71_CAPACITY - forwarded_count))) {
        memcpy(&forwarded_bytes[forwarded_count], bytes, length);
        forwarded_count += length;
    }
}

void __wrap_csp_rs485_freertos_mark_rx_discontinuity_from_isr(void)
{
    if (!capture_link_boundary) {
        __real_csp_rs485_freertos_mark_rx_discontinuity_from_isr();
        return;
    }
    ++discontinuity_count;
}

void __wrap_csp_rs485_link_report_fault_from_isr(csp_rs485_fault_t fault)
{
    if (!capture_link_boundary) {
        __real_csp_rs485_link_report_fault_from_isr(fault);
        return;
    }
    if (reported_fault_count < FAKE_SAMV71_CAPACITY) {
        reported_faults[reported_fault_count] = fault;
        ++reported_fault_count;
    }
}

static void setup_port_test(void)
{
    fake_samv71_hw_reset();
    capture_link_boundary = true;
    memset(forwarded_bytes, 0, sizeof(forwarded_bytes));
    forwarded_count = 0U;
    discontinuity_count = 0U;
    memset(reported_faults, 0, sizeof(reported_faults));
    reported_fault_count = 0U;
}

static bool initialize_arm_enable(void)
{
    return (samv71_rs485_port_ops.initialize(&samv71_rs485_port_context)
            == CSP_RS485_PORT_OK)
        && (samv71_rs485_port_ops.arm_receive(&samv71_rs485_port_context)
            == CSP_RS485_PORT_OK)
        && (samv71_rs485_port_ops.enable_irqs(
                &samv71_rs485_port_context),
            true);
}

static bool event_kinds_match(
    const fake_samv71_event_kind_t *expected,
    size_t expected_count)
{
    const fake_samv71_hw_t *const fake = fake_samv71_hw_state();
    if (fake->event_overflow || (fake->event_count != expected_count)) {
        return false;
    }
    for (size_t index = 0U; index < expected_count; ++index) {
        if (fake->events[index].kind != expected[index]) {
            return false;
        }
    }
    return true;
}

static void initialization_is_receive_safe_and_programs_fractional_8n1(void)
{
    static const uint8_t queued[] = {0x11U, 0x22U};
    setup_port_test();
    fake_samv71_hw_set_rx_bytes(queued, sizeof(queued));

    const csp_rs485_port_result_t result =
        samv71_rs485_port_ops.initialize(&samv71_rs485_port_context);
    const fake_samv71_hw_t *const fake = fake_samv71_hw_state();

    TEST_ASSERT_EQ_SIZE(CSP_RS485_PORT_OK, result);
    TEST_ASSERT_EQ_SIZE(UINT32_C(0x000008c0), USART1_REGS->US_MR);
    TEST_ASSERT_EQ_SIZE(UINT32_C(0x0001000a), USART1_REGS->US_BRGR);
    TEST_ASSERT_TRUE(!fake->rx_irq_enabled);
    TEST_ASSERT_TRUE(!fake->de_high);
    TEST_ASSERT_TRUE(!fake->nre_high);
    TEST_ASSERT_EQ_SIZE(sizeof(queued), fake->rx_position);
    TEST_ASSERT_EQ_SIZE(FAKE_SAMV71_EVENT_RX_IRQ_OFF, fake->events[0].kind);
}

static void arm_clears_receive_state_before_irq_enable(void)
{
    static const fake_samv71_event_kind_t expected[] = {
        FAKE_SAMV71_EVENT_RX_IRQ_OFF,
        FAKE_SAMV71_EVENT_RESET_STATUS_AND_FLUSH,
        FAKE_SAMV71_EVENT_RESET_RX,
        FAKE_SAMV71_EVENT_CLEAR_PENDING,
        FAKE_SAMV71_EVENT_RX_IRQ_ON,
    };
    setup_port_test();
    TEST_ASSERT_EQ_SIZE(
        CSP_RS485_PORT_OK,
        samv71_rs485_port_ops.initialize(&samv71_rs485_port_context));
    fake_samv71_hw_clear_events();

    TEST_ASSERT_EQ_SIZE(
        CSP_RS485_PORT_OK,
        samv71_rs485_port_ops.arm_receive(&samv71_rs485_port_context));
    samv71_rs485_port_ops.enable_irqs(&samv71_rs485_port_context);

    TEST_ASSERT_TRUE(
        event_kinds_match(expected, sizeof(expected) / sizeof(expected[0])));
}

static void hooks_decline_ownership_before_initialization(void)
{
    static const uint8_t queued[] = {0x41U};
    setup_port_test();
    fake_samv71_hw_set_rx_bytes(queued, sizeof(queued));

    const bool rx_owned = USART1_UartCommRxReadyHook();
    const bool error_owned = USART1_UartCommErrorHook(UINT32_C(0x20));

    TEST_ASSERT_TRUE(!rx_owned);
    TEST_ASSERT_TRUE(!error_owned);
    TEST_ASSERT_EQ_SIZE(0U, forwarded_count);
    TEST_ASSERT_EQ_SIZE(0U, discontinuity_count);
    TEST_ASSERT_EQ_SIZE(0U, reported_fault_count);
    TEST_ASSERT_EQ_SIZE(0U, fake_samv71_hw_state()->rx_position);
}

static void rx_hook_forwards_every_available_byte(void)
{
    static const uint8_t queued[] = {0xc0U, 0xdbU, 0x7eU};
    setup_port_test();
    TEST_ASSERT_TRUE(initialize_arm_enable());
    fake_samv71_hw_set_rx_bytes(queued, sizeof(queued));

    const bool owned = USART1_UartCommRxReadyHook();

    TEST_ASSERT_TRUE(owned);
    TEST_ASSERT_EQ_SIZE(sizeof(queued), forwarded_count);
    TEST_ASSERT_TRUE(memcmp(queued, forwarded_bytes, sizeof(queued)) == 0);
}

static void error_hook_marks_one_discontinuity_and_uart_fault(void)
{
    static const uint8_t queued[] = {0x33U, 0x44U};
    setup_port_test();
    TEST_ASSERT_TRUE(initialize_arm_enable());
    fake_samv71_hw_set_rx_bytes(queued, sizeof(queued));

    const bool owned = USART1_UartCommErrorHook(UINT32_C(0x28));

    TEST_ASSERT_TRUE(owned);
    TEST_ASSERT_EQ_SIZE(sizeof(queued), fake_samv71_hw_state()->rx_position);
    TEST_ASSERT_EQ_SIZE(1U, discontinuity_count);
    TEST_ASSERT_EQ_SIZE(1U, reported_fault_count);
    TEST_ASSERT_EQ_SIZE(CSP_RS485_FAULT_UART, reported_faults[0]);
}

static void successful_transmit_has_exact_direction_and_byte_order(void)
{
    static const uint8_t frame[] = {0xa5U, 0x5aU};
    static const uint32_t status[] = {
        0U,
        SAMV71_RS485_STATUS_TX_READY,
        SAMV71_RS485_STATUS_TX_READY,
        0U,
        SAMV71_RS485_STATUS_TX_EMPTY,
    };
    static const uint32_t times[] = {10U, 11U, 12U};
    static const fake_samv71_event_kind_t expected[] = {
        FAKE_SAMV71_EVENT_RX_IRQ_OFF,
        FAKE_SAMV71_EVENT_NRE_HIGH,
        FAKE_SAMV71_EVENT_DE_HIGH,
        FAKE_SAMV71_EVENT_GUARD_1BIT,
        FAKE_SAMV71_EVENT_WRITE,
        FAKE_SAMV71_EVENT_WRITE,
        FAKE_SAMV71_EVENT_WAIT_TXEMPTY,
        FAKE_SAMV71_EVENT_DE_LOW,
        FAKE_SAMV71_EVENT_NRE_LOW,
        FAKE_SAMV71_EVENT_RX_IRQ_ON,
    };
    setup_port_test();
    TEST_ASSERT_TRUE(initialize_arm_enable());
    fake_samv71_hw_clear_events();
    fake_samv71_hw_expect_frame_length(sizeof(frame));
    fake_samv71_hw_set_status_script(
        status,
        sizeof(status) / sizeof(status[0]),
        SAMV71_RS485_STATUS_TX_EMPTY);
    fake_samv71_hw_set_time_script(times, sizeof(times) / sizeof(times[0]));

    const csp_rs485_port_result_t result =
        samv71_rs485_port_ops.transmit_frame(
            &samv71_rs485_port_context,
            frame,
            sizeof(frame),
            5U);
    const fake_samv71_hw_t *const fake = fake_samv71_hw_state();

    TEST_ASSERT_EQ_SIZE(CSP_RS485_PORT_OK, result);
    TEST_ASSERT_TRUE(
        event_kinds_match(expected, sizeof(expected) / sizeof(expected[0])));
    TEST_ASSERT_EQ_SIZE(frame[0], fake->events[4].value);
    TEST_ASSERT_EQ_SIZE(frame[1], fake->events[5].value);
    TEST_ASSERT_EQ_SIZE(1U, fake->guard_count);
}

static void transmit_uses_one_absolute_frame_deadline(void)
{
    static const uint8_t frame[] = {0x12U, 0x34U};
    static const uint32_t status[] = {
        SAMV71_RS485_STATUS_TX_READY,
        0U,
        SAMV71_RS485_STATUS_TX_READY,
        0U,
        SAMV71_RS485_STATUS_TX_EMPTY,
    };
    static const uint32_t times[] = {100U, 103U, 105U};
    setup_port_test();
    TEST_ASSERT_TRUE(initialize_arm_enable());
    fake_samv71_hw_clear_events();
    fake_samv71_hw_expect_frame_length(sizeof(frame));
    fake_samv71_hw_set_status_script(
        status,
        sizeof(status) / sizeof(status[0]),
        SAMV71_RS485_STATUS_TX_EMPTY);
    fake_samv71_hw_set_time_script(times, sizeof(times) / sizeof(times[0]));

    const csp_rs485_port_result_t result =
        samv71_rs485_port_ops.transmit_frame(
            &samv71_rs485_port_context,
            frame,
            sizeof(frame),
            5U);

    TEST_ASSERT_EQ_SIZE(CSP_RS485_PORT_TIMEOUT, result);
    TEST_ASSERT_EQ_SIZE(sizeof(frame), fake_samv71_hw_state()->write_count);
    TEST_ASSERT_EQ_SIZE(1U, fake_samv71_hw_state()->reset_tx_count);
}

static void txrdy_timeout_writes_nothing_and_restores_receive(void)
{
    static const uint8_t frame[] = {0x9aU};
    static const uint32_t status[] = {0U};
    static const uint32_t times[] = {20U, 23U};
    setup_port_test();
    TEST_ASSERT_TRUE(initialize_arm_enable());
    fake_samv71_hw_clear_events();
    fake_samv71_hw_set_status_script(status, 1U, 0U);
    fake_samv71_hw_set_time_script(times, 2U);

    const csp_rs485_port_result_t result =
        samv71_rs485_port_ops.transmit_frame(
            &samv71_rs485_port_context,
            frame,
            sizeof(frame),
            3U);
    const fake_samv71_hw_t *const fake = fake_samv71_hw_state();

    TEST_ASSERT_EQ_SIZE(CSP_RS485_PORT_TIMEOUT, result);
    TEST_ASSERT_EQ_SIZE(0U, fake->write_count);
    TEST_ASSERT_EQ_SIZE(1U, fake->reset_tx_count);
    TEST_ASSERT_TRUE(!fake->de_high);
    TEST_ASSERT_TRUE(!fake->nre_high);
    TEST_ASSERT_TRUE(fake->rx_irq_enabled);
}

static void txempty_timeout_never_retries_partial_frame(void)
{
    static const uint8_t frame[] = {0x71U, 0x72U};
    static const uint32_t status[] = {
        SAMV71_RS485_STATUS_TX_READY,
        SAMV71_RS485_STATUS_TX_READY,
        0U,
    };
    static const uint32_t times[] = {50U, 54U};
    setup_port_test();
    TEST_ASSERT_TRUE(initialize_arm_enable());
    fake_samv71_hw_clear_events();
    fake_samv71_hw_expect_frame_length(sizeof(frame));
    fake_samv71_hw_set_status_script(status, 3U, 0U);
    fake_samv71_hw_set_time_script(times, 2U);

    const csp_rs485_port_result_t result =
        samv71_rs485_port_ops.transmit_frame(
            &samv71_rs485_port_context,
            frame,
            sizeof(frame),
            4U);
    const fake_samv71_hw_t *const fake = fake_samv71_hw_state();

    TEST_ASSERT_EQ_SIZE(CSP_RS485_PORT_TIMEOUT, result);
    TEST_ASSERT_EQ_SIZE(sizeof(frame), fake->write_count);
    TEST_ASSERT_EQ_SIZE(1U, fake->reset_tx_count);
    TEST_ASSERT_TRUE(!fake->de_high && !fake->nre_high);
    TEST_ASSERT_TRUE(fake->rx_irq_enabled);
}

static void null_frame_is_rejected_without_a_write(void)
{
    setup_port_test();
    TEST_ASSERT_TRUE(initialize_arm_enable());
    fake_samv71_hw_clear_events();

    const csp_rs485_port_result_t result =
        samv71_rs485_port_ops.transmit_frame(
            &samv71_rs485_port_context,
            NULL,
            1U,
            5U);

    TEST_ASSERT_EQ_SIZE(CSP_RS485_PORT_ERROR, result);
    TEST_ASSERT_EQ_SIZE(0U, fake_samv71_hw_state()->write_count);
    TEST_ASSERT_TRUE(!fake_samv71_hw_state()->de_high);
    TEST_ASSERT_TRUE(!fake_samv71_hw_state()->nre_high);
    TEST_ASSERT_TRUE(fake_samv71_hw_state()->rx_irq_enabled);
}

static void zero_length_frame_is_rejected_without_a_write(void)
{
    static const uint8_t byte = 0x55U;
    setup_port_test();
    TEST_ASSERT_TRUE(initialize_arm_enable());
    fake_samv71_hw_clear_events();

    const csp_rs485_port_result_t result =
        samv71_rs485_port_ops.transmit_frame(
            &samv71_rs485_port_context,
            &byte,
            0U,
            5U);

    TEST_ASSERT_EQ_SIZE(CSP_RS485_PORT_ERROR, result);
    TEST_ASSERT_EQ_SIZE(0U, fake_samv71_hw_state()->write_count);
    TEST_ASSERT_TRUE(!fake_samv71_hw_state()->de_high);
    TEST_ASSERT_TRUE(!fake_samv71_hw_state()->nre_high);
    TEST_ASSERT_TRUE(fake_samv71_hw_state()->rx_irq_enabled);
}

static void transmit_restores_a_previously_disabled_rx_irq(void)
{
    static const uint8_t frame[] = {0x22U};
    static const uint32_t status[] = {
        SAMV71_RS485_STATUS_TX_READY,
        SAMV71_RS485_STATUS_TX_EMPTY,
    };
    static const uint32_t times[] = {1U};
    setup_port_test();
    TEST_ASSERT_EQ_SIZE(
        CSP_RS485_PORT_OK,
        samv71_rs485_port_ops.initialize(&samv71_rs485_port_context));
    fake_samv71_hw_clear_events();
    fake_samv71_hw_expect_frame_length(sizeof(frame));
    fake_samv71_hw_set_status_script(status, 2U, 0U);
    fake_samv71_hw_set_time_script(times, 1U);

    const csp_rs485_port_result_t result =
        samv71_rs485_port_ops.transmit_frame(
            &samv71_rs485_port_context,
            frame,
            sizeof(frame),
            5U);

    TEST_ASSERT_EQ_SIZE(CSP_RS485_PORT_OK, result);
    TEST_ASSERT_TRUE(!fake_samv71_hw_state()->rx_irq_enabled);
}

static void abort_and_deinitialize_are_idempotent_and_receive_safe(void)
{
    setup_port_test();
    TEST_ASSERT_TRUE(initialize_arm_enable());

    TEST_ASSERT_EQ_SIZE(
        CSP_RS485_PORT_OK,
        samv71_rs485_port_ops.abort_receive(&samv71_rs485_port_context));
    TEST_ASSERT_EQ_SIZE(
        CSP_RS485_PORT_OK,
        samv71_rs485_port_ops.abort_receive(&samv71_rs485_port_context));
    samv71_rs485_port_ops.reset_rx_position(&samv71_rs485_port_context);
    samv71_rs485_port_ops.reset_rx_position(&samv71_rs485_port_context);
    TEST_ASSERT_EQ_SIZE(
        CSP_RS485_PORT_OK,
        samv71_rs485_port_ops.deinitialize(&samv71_rs485_port_context));
    TEST_ASSERT_EQ_SIZE(
        CSP_RS485_PORT_OK,
        samv71_rs485_port_ops.deinitialize(&samv71_rs485_port_context));

    TEST_ASSERT_TRUE(!fake_samv71_hw_state()->rx_irq_enabled);
    TEST_ASSERT_TRUE(!fake_samv71_hw_state()->de_high);
    TEST_ASSERT_TRUE(!fake_samv71_hw_state()->nre_high);
}

static void deadline_comparison_is_safe_across_u32_wrap(void)
{
    static const uint8_t frame[] = {0xeeU};
    static const uint32_t status[] = {
        0U,
        0U,
        SAMV71_RS485_STATUS_TX_READY,
        SAMV71_RS485_STATUS_TX_EMPTY,
    };
    static const uint32_t times[] = {
        UINT32_MAX - 2U,
        UINT32_MAX,
        1U,
    };
    setup_port_test();
    TEST_ASSERT_TRUE(initialize_arm_enable());
    fake_samv71_hw_clear_events();
    fake_samv71_hw_expect_frame_length(sizeof(frame));
    fake_samv71_hw_set_status_script(status, 4U, 0U);
    fake_samv71_hw_set_time_script(times, 3U);

    const csp_rs485_port_result_t result =
        samv71_rs485_port_ops.transmit_frame(
            &samv71_rs485_port_context,
            frame,
            sizeof(frame),
            5U);

    TEST_ASSERT_EQ_SIZE(CSP_RS485_PORT_OK, result);
    TEST_ASSERT_EQ_SIZE(1U, fake_samv71_hw_state()->write_count);
}

const test_case_t samv71_rs485_port_tests[] = {
    {"samv71_rs485_port", "initialization is receive safe and programs fractional 8N1", initialization_is_receive_safe_and_programs_fractional_8n1},
    {"samv71_rs485_port", "arm clears receive state before IRQ enable", arm_clears_receive_state_before_irq_enable},
    {"samv71_rs485_port", "hooks decline ownership before initialization", hooks_decline_ownership_before_initialization},
    {"samv71_rs485_port", "RX hook forwards every available byte", rx_hook_forwards_every_available_byte},
    {"samv71_rs485_port", "error hook marks one discontinuity and UART fault", error_hook_marks_one_discontinuity_and_uart_fault},
    {"samv71_rs485_port", "successful transmit has exact direction and byte order", successful_transmit_has_exact_direction_and_byte_order},
    {"samv71_rs485_port", "transmit uses one absolute frame deadline", transmit_uses_one_absolute_frame_deadline},
    {"samv71_rs485_port", "TXRDY timeout writes nothing and restores receive", txrdy_timeout_writes_nothing_and_restores_receive},
    {"samv71_rs485_port", "TXEMPTY timeout never retries partial frame", txempty_timeout_never_retries_partial_frame},
    {"samv71_rs485_port", "null frame is rejected without a write", null_frame_is_rejected_without_a_write},
    {"samv71_rs485_port", "zero length frame is rejected without a write", zero_length_frame_is_rejected_without_a_write},
    {"samv71_rs485_port", "transmit restores a previously disabled RX IRQ", transmit_restores_a_previously_disabled_rx_irq},
    {"samv71_rs485_port", "abort and deinitialize are idempotent and receive safe", abort_and_deinitialize_are_idempotent_and_receive_safe},
    {"samv71_rs485_port", "deadline comparison is safe across u32 wrap", deadline_comparison_is_safe_across_u32_wrap},
};

const size_t samv71_rs485_port_test_count =
    sizeof(samv71_rs485_port_tests) / sizeof(samv71_rs485_port_tests[0]);
