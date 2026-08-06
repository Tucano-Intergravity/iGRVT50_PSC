/* Imported from C:\PSC\csp-rs485\tests\host (authorized subset). */
/* This import makes no assertion about upstream licensing. */
#include "fakes/fake_port.h"
#include "support/test.h"

#include <csp/csp_error.h>

#include <csp_rs485_link.h>

#include "csp_rs485_internal.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

/*
 * The link retains its port context until deinitialization. Static storage
 * keeps the fake valid for the runner's unconditional post-test cleanup,
 * including assertion-failure exits.
 */
static fake_port_t fake;

static csp_rs485_link_config_t make_link_config(fake_port_t *port)
{
    const csp_rs485_link_config_t config = {
        .port_ops = fake_port_get_ops(),
        .port_context = port,
        .tx_margin_ms = 5U,
        .recovery_retry_ms = 25U,
        .task_priority = 3U,
        .task_stack_words = 512U,
    };
    return config;
}

static int initialize_link(fake_port_t *port)
{
    csp_rs485_link_deinit();
    fake_port_init(port);
    const csp_rs485_link_config_t config = make_link_config(port);
    return csp_rs485_link_init(&config);
}

static bool operations_match(
    const fake_port_t *port,
    const char *const *expected,
    size_t expected_count)
{
    if (port->operation_overflow
        || (port->operation_count != expected_count)) {
        return false;
    }

    for (size_t index = 0U; index < expected_count; ++index) {
        if (strcmp(expected[index], port->operations[index].name) != 0) {
            return false;
        }
    }

    return true;
}

static size_t operation_count_named(
    const fake_port_t *port,
    const char *name)
{
    size_t count = 0U;

    for (size_t index = 0U; index < port->operation_count; ++index) {
        if (strcmp(name, port->operations[index].name) == 0) {
            ++count;
        }
    }

    return count;
}

static void supervisor_starts_stopped(void)
{
    csp_rs485_health_t health;

    csp_rs485_link_deinit();
    csp_rs485_link_get_health(&health);

    TEST_ASSERT_EQ_SIZE(CSP_RS485_LINK_STOPPED, health.state);
}

static void supervisor_init_publishes_running_only_after_rx_and_irqs(void)
{
    static const char *const expected[] = {
        "initialize",
        "arm_receive",
        "enable_irqs",
    };
    const int init_result = initialize_link(&fake);
    csp_rs485_health_t health;
    csp_rs485_link_get_health(&health);

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_TRUE(
        operations_match(
            &fake,
            expected,
            sizeof(expected) / sizeof(expected[0])));
    TEST_ASSERT_EQ_SIZE(
        CSP_RS485_LINK_STOPPED,
        fake.operations[0].link_state);
    TEST_ASSERT_EQ_SIZE(
        CSP_RS485_LINK_STOPPED,
        fake.operations[1].link_state);
    TEST_ASSERT_EQ_SIZE(
        CSP_RS485_LINK_STOPPED,
        fake.operations[2].link_state);
    TEST_ASSERT_EQ_SIZE(CSP_RS485_LINK_RUNNING, health.state);
}

static void supervisor_fault_enters_recovering(void)
{
    const int init_result = initialize_link(&fake);
    fake_port_clear_calls(&fake);

    csp_rs485_link_report_fault(CSP_RS485_FAULT_UART);
    csp_rs485_health_t health;
    csp_rs485_link_get_health(&health);

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_EQ_SIZE(CSP_RS485_LINK_RECOVERING, health.state);
    TEST_ASSERT_EQ_SIZE(CSP_RS485_FAULT_UART, health.last_error);
    TEST_ASSERT_EQ_SIZE(1U, health.uart_errors);
    TEST_ASSERT_EQ_SIZE(0U, fake.operation_count);
}

static void supervisor_recovery_uses_exact_nine_step_order(void)
{
    static const char *const expected_port_operations[] = {
        "force_receive_mode",
        "disable_and_clear_irqs",
        "abort_receive",
        "deinitialize",
        "reset_rx_position",
        "initialize",
        "arm_receive",
        "enable_irqs",
    };
    const int init_result = initialize_link(&fake);
    fake_port_clear_calls(&fake);

    csp_rs485_link_report_fault(CSP_RS485_FAULT_DMA);
    csp_rs485_health_t before;
    csp_rs485_link_get_health(&before);
    const csp_rs485_recovery_result_t recovery_result =
        csp_rs485_link_recovery_step();
    csp_rs485_health_t after;
    csp_rs485_link_get_health(&after);

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    /*
     * RECOVERING is the observable block_tx step. The eight port calls then
     * complete the approved nine-step sequence.
     */
    TEST_ASSERT_EQ_SIZE(CSP_RS485_LINK_RECOVERING, before.state);
    TEST_ASSERT_TRUE(
        operations_match(
            &fake,
            expected_port_operations,
            sizeof(expected_port_operations)
                / sizeof(expected_port_operations[0])));
    for (size_t index = 0U; index < fake.operation_count; ++index) {
        TEST_ASSERT_EQ_SIZE(
            CSP_RS485_LINK_RECOVERING,
            fake.operations[index].link_state);
    }
    TEST_ASSERT_EQ_SIZE(CSP_RS485_RECOVERY_COMPLETE, recovery_result);
    TEST_ASSERT_EQ_SIZE(CSP_RS485_LINK_RUNNING, after.state);
    TEST_ASSERT_EQ_SIZE(1U, after.stream_discontinuities);
    TEST_ASSERT_EQ_SIZE(1U, after.recovery_attempts);
    TEST_ASSERT_EQ_SIZE(1U, after.recovery_successes);
    TEST_ASSERT_EQ_SIZE(0U, after.recovery_failures);
}

static void supervisor_cleanup_continues_after_abort_failure(void)
{
    static const char *const expected[] = {
        "force_receive_mode",
        "disable_and_clear_irqs",
        "abort_receive",
        "deinitialize",
        "reset_rx_position",
        "initialize",
        "arm_receive",
        "enable_irqs",
    };
    const int init_result = initialize_link(&fake);
    fake_port_clear_calls(&fake);
    fake.abort_receive_result = CSP_RS485_PORT_ERROR;

    csp_rs485_link_report_fault(CSP_RS485_FAULT_UART);
    const csp_rs485_recovery_result_t recovery_result =
        csp_rs485_link_recovery_step();
    csp_rs485_health_t health;
    csp_rs485_link_get_health(&health);

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_TRUE(
        operations_match(
            &fake,
            expected,
            sizeof(expected) / sizeof(expected[0])));
    TEST_ASSERT_EQ_SIZE(CSP_RS485_RECOVERY_COMPLETE, recovery_result);
    TEST_ASSERT_EQ_SIZE(CSP_RS485_LINK_RUNNING, health.state);
    TEST_ASSERT_EQ_SIZE(1U, health.recovery_successes);
    TEST_ASSERT_EQ_SIZE(1U, health.recovery_failures);
}

static void supervisor_init_failure_stays_recovering(void)
{
    static const char *const expected[] = {
        "force_receive_mode",
        "disable_and_clear_irqs",
        "abort_receive",
        "deinitialize",
        "reset_rx_position",
        "initialize",
        "force_receive_mode",
    };
    const int init_result = initialize_link(&fake);
    fake_port_clear_calls(&fake);
    fake.initialize_result = CSP_RS485_PORT_ERROR;

    csp_rs485_link_report_fault(CSP_RS485_FAULT_DMA);
    const csp_rs485_recovery_result_t recovery_result =
        csp_rs485_link_recovery_step();
    csp_rs485_health_t health;
    csp_rs485_link_get_health(&health);

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_TRUE(
        operations_match(
            &fake,
            expected,
            sizeof(expected) / sizeof(expected[0])));
    TEST_ASSERT_EQ_SIZE(
        CSP_RS485_RECOVERY_RETRY_AFTER_WAIT,
        recovery_result);
    TEST_ASSERT_EQ_SIZE(CSP_RS485_LINK_RECOVERING, health.state);
    TEST_ASSERT_EQ_SIZE(0U, health.recovery_successes);
    TEST_ASSERT_EQ_SIZE(1U, health.recovery_failures);
}

static void supervisor_arm_failure_returns_to_safe_receive_mode(void)
{
    static const char *const expected[] = {
        "force_receive_mode",
        "disable_and_clear_irqs",
        "abort_receive",
        "deinitialize",
        "reset_rx_position",
        "initialize",
        "arm_receive",
        "force_receive_mode",
    };
    const int init_result = initialize_link(&fake);
    fake_port_clear_calls(&fake);
    fake.arm_receive_result = CSP_RS485_PORT_ERROR;

    csp_rs485_link_report_fault(CSP_RS485_FAULT_UART);
    const csp_rs485_recovery_result_t recovery_result =
        csp_rs485_link_recovery_step();
    csp_rs485_health_t health;
    csp_rs485_link_get_health(&health);

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_TRUE(
        operations_match(
            &fake,
            expected,
            sizeof(expected) / sizeof(expected[0])));
    TEST_ASSERT_EQ_SIZE(
        CSP_RS485_RECOVERY_RETRY_AFTER_WAIT,
        recovery_result);
    TEST_ASSERT_EQ_SIZE(CSP_RS485_LINK_RECOVERING, health.state);
    TEST_ASSERT_EQ_SIZE(0U, operation_count_named(&fake, "enable_irqs"));
    TEST_ASSERT_EQ_SIZE(1U, health.recovery_failures);
}

static void supervisor_failed_attempt_does_not_loop(void)
{
    const int init_result = initialize_link(&fake);
    fake_port_clear_calls(&fake);
    fake.initialize_result = CSP_RS485_PORT_ERROR;

    csp_rs485_link_report_fault(CSP_RS485_FAULT_DMA);
    const csp_rs485_recovery_result_t recovery_result =
        csp_rs485_link_recovery_step();
    csp_rs485_health_t health;
    csp_rs485_link_get_health(&health);

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_EQ_SIZE(
        CSP_RS485_RECOVERY_RETRY_AFTER_WAIT,
        recovery_result);
    TEST_ASSERT_EQ_SIZE(1U, operation_count_named(&fake, "initialize"));
    TEST_ASSERT_EQ_SIZE(1U, health.recovery_attempts);
    TEST_ASSERT_EQ_SIZE(1U, health.recovery_failures);
}

static void supervisor_delayed_retry_can_reach_running(void)
{
    const int init_result = initialize_link(&fake);
    fake_port_clear_calls(&fake);
    fake.initialize_result = CSP_RS485_PORT_ERROR;

    csp_rs485_link_report_fault(CSP_RS485_FAULT_UART);
    const csp_rs485_recovery_result_t first_result =
        csp_rs485_link_recovery_step();
    fake.initialize_result = CSP_RS485_PORT_OK;
    const csp_rs485_recovery_result_t second_result =
        csp_rs485_link_recovery_step();
    csp_rs485_health_t health;
    csp_rs485_link_get_health(&health);

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_EQ_SIZE(
        CSP_RS485_RECOVERY_RETRY_AFTER_WAIT,
        first_result);
    TEST_ASSERT_EQ_SIZE(CSP_RS485_RECOVERY_COMPLETE, second_result);
    TEST_ASSERT_EQ_SIZE(CSP_RS485_LINK_RUNNING, health.state);
    TEST_ASSERT_EQ_SIZE(2U, health.recovery_attempts);
    TEST_ASSERT_EQ_SIZE(1U, health.recovery_successes);
    TEST_ASSERT_EQ_SIZE(1U, health.recovery_failures);
}

static void supervisor_deinit_cancels_recovery_and_stops(void)
{
    static const char *const expected[] = {
        "force_receive_mode",
        "disable_and_clear_irqs",
        "abort_receive",
        "deinitialize",
        "reset_rx_position",
    };
    const int init_result = initialize_link(&fake);
    fake_port_clear_calls(&fake);

    csp_rs485_link_report_fault(CSP_RS485_FAULT_DMA);
    csp_rs485_link_deinit();
    const size_t operation_count_after_stop = fake.operation_count;
    const csp_rs485_recovery_result_t recovery_result =
        csp_rs485_link_recovery_step();
    csp_rs485_health_t health;
    csp_rs485_link_get_health(&health);

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_TRUE(
        operations_match(
            &fake,
            expected,
            sizeof(expected) / sizeof(expected[0])));
    TEST_ASSERT_EQ_SIZE(
        CSP_RS485_RECOVERY_CANCELLED,
        recovery_result);
    TEST_ASSERT_EQ_SIZE(
        operation_count_after_stop,
        fake.operation_count);
    TEST_ASSERT_EQ_SIZE(CSP_RS485_LINK_STOPPED, health.state);
}

static void supervisor_fault_during_recovery_does_not_recurse(void)
{
    const int init_result = initialize_link(&fake);
    fake_port_clear_calls(&fake);

    csp_rs485_link_report_fault(CSP_RS485_FAULT_UART);
    csp_rs485_link_report_fault(CSP_RS485_FAULT_DMA);
    csp_rs485_health_t health;
    csp_rs485_link_get_health(&health);

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_EQ_SIZE(CSP_RS485_LINK_RECOVERING, health.state);
    TEST_ASSERT_EQ_SIZE(CSP_RS485_FAULT_DMA, health.last_error);
    TEST_ASSERT_EQ_SIZE(1U, health.uart_errors);
    TEST_ASSERT_EQ_SIZE(1U, health.dma_errors);
    TEST_ASSERT_EQ_SIZE(0U, health.recovery_attempts);
    TEST_ASSERT_EQ_SIZE(0U, fake.operation_count);
}

const test_case_t supervisor_tests[] = {
    {
        "test_supervisor",
        "supervisor_starts_stopped",
        supervisor_starts_stopped,
    },
    {
        "test_supervisor",
        "supervisor_init_publishes_running_only_after_rx_and_irqs",
        supervisor_init_publishes_running_only_after_rx_and_irqs,
    },
    {
        "test_supervisor",
        "supervisor_fault_enters_recovering",
        supervisor_fault_enters_recovering,
    },
    {
        "test_supervisor",
        "supervisor_recovery_uses_exact_nine_step_order",
        supervisor_recovery_uses_exact_nine_step_order,
    },
    {
        "test_supervisor",
        "supervisor_cleanup_continues_after_abort_failure",
        supervisor_cleanup_continues_after_abort_failure,
    },
    {
        "test_supervisor",
        "supervisor_init_failure_stays_recovering",
        supervisor_init_failure_stays_recovering,
    },
    {
        "test_supervisor",
        "supervisor_arm_failure_returns_to_safe_receive_mode",
        supervisor_arm_failure_returns_to_safe_receive_mode,
    },
    {
        "test_supervisor",
        "supervisor_failed_attempt_does_not_loop",
        supervisor_failed_attempt_does_not_loop,
    },
    {
        "test_supervisor",
        "supervisor_delayed_retry_can_reach_running",
        supervisor_delayed_retry_can_reach_running,
    },
    {
        "test_supervisor",
        "supervisor_deinit_cancels_recovery_and_stops",
        supervisor_deinit_cancels_recovery_and_stops,
    },
    {
        "test_supervisor",
        "supervisor_fault_during_recovery_does_not_recurse",
        supervisor_fault_during_recovery_does_not_recurse,
    },
};

const size_t supervisor_test_count =
    sizeof(supervisor_tests) / sizeof(supervisor_tests[0]);
