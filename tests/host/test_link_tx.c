/* Imported from C:\PSC\csp-rs485\tests\host (authorized subset). */
/* This import makes no assertion about upstream licensing. */
#include "fakes/fake_port.h"
#include "support/host_csp.h"
#include "support/test.h"

#include <csp/csp.h>
#include <csp/csp_buffer.h>
#include <csp/csp_error.h>
#include <csp/csp_iflist.h>
#include <csp/csp_rtable.h>

#include <csp_rs485_link.h>
#include <csp_rs485_profile.h>

#include <stdint.h>
#include <string.h>

#define TEST_INTERFACE_NAME "RS485"

static csp_rs485_link_config_t make_link_config(
    fake_port_t *fake,
    uint32_t tx_margin_ms)
{
    const csp_rs485_link_config_t config = {
        .port_ops = fake_port_get_ops(),
        .port_context = fake,
        .tx_margin_ms = tx_margin_ms,
        .recovery_retry_ms = 25U,
        .task_priority = 3U,
        .task_stack_words = 512U,
    };
    return config;
}

static int initialize_link(fake_port_t *fake, uint32_t tx_margin_ms)
{
    csp_rs485_link_deinit();
    fake_port_init(fake);
    const csp_rs485_link_config_t config =
        make_link_config(fake, tx_margin_ms);
    return csp_rs485_link_init(&config);
}

static csp_packet_t *make_packet(
    uint32_t id,
    const uint8_t *payload,
    size_t payload_length)
{
    csp_packet_t *packet =
        csp_buffer_get(payload_length + CSP_RS485_CSP_CRC_SIZE);

    if (packet == NULL) {
        return NULL;
    }

    packet->id.ext = id;
    packet->length = (uint16_t) payload_length;
    if (payload_length > 0U) {
        memcpy(packet->data, payload, payload_length);
    }
    return packet;
}

static int call_next_hop(csp_packet_t *packet)
{
    csp_route_t route = {
        .iface = csp_rs485_link_get_interface(),
        .via = CSP_NO_VIA_ADDRESS,
    };
    return route.iface->nexthop(&route, packet);
}

static size_t count_interface_pointer(const csp_iface_t *expected)
{
    size_t count = 0U;

    for (const csp_iface_t *interface = csp_iflist_get();
         interface != NULL;
         interface = interface->next) {
        if (interface == expected) {
            ++count;
        }
    }
    return count;
}

static void link_registers_one_named_interface(void)
{
    fake_port_t fake;
    const int init_result = initialize_link(&fake, 5U);
    csp_iface_t *interface = csp_rs485_link_get_interface();
    csp_iface_t *listed = csp_iflist_get_by_name(TEST_INTERFACE_NAME);
    const size_t count = count_interface_pointer(interface);

    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_TRUE(interface != NULL);
    TEST_ASSERT_TRUE(strcmp(TEST_INTERFACE_NAME, interface->name) == 0);
    TEST_ASSERT_TRUE(listed == interface);
    TEST_ASSERT_EQ_SIZE(1U, count);
}

static void link_reinit_does_not_register_duplicate_interface(void)
{
    fake_port_t fake;
    const int first_result = initialize_link(&fake, 5U);
    csp_rs485_link_deinit();

    const csp_rs485_link_config_t config = make_link_config(&fake, 5U);
    const int second_result = csp_rs485_link_init(&config);
    csp_iface_t *interface = csp_rs485_link_get_interface();
    const size_t count = count_interface_pointer(interface);

    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, first_result);
    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, second_result);
    TEST_ASSERT_EQ_SIZE(1U, count);
}

static void link_rejects_runtime_buffer_size_mismatch(void)
{
    fake_port_t fake;
    csp_conf_t config;

    csp_rs485_link_deinit();
    fake_port_init(&fake);
    host_csp_cleanup();

    csp_conf_get_defaults(&config);
    config.address = 10U;
    config.buffers = 20U;
    config.buffer_data_size =
        (uint16_t) (CSP_RS485_CSP_BUFFER_DATA_SIZE - 1U);
    const int csp_result = csp_init(&config);

    const csp_rs485_link_config_t link_config = make_link_config(&fake, 5U);
    const int link_result = csp_rs485_link_init(&link_config);
    csp_rs485_health_t health;
    csp_rs485_link_get_health(&health);

    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, csp_result);
    TEST_ASSERT_EQ_SIZE(CSP_ERR_INVAL, link_result);
    TEST_ASSERT_EQ_SIZE(CSP_RS485_LINK_STOPPED, health.state);
    TEST_ASSERT_EQ_SIZE(0U, fake.transmit_call_count);
}

static void link_sets_mtu_to_296(void)
{
    fake_port_t fake;
    const int init_result = initialize_link(&fake, 5U);
    const uint16_t mtu = csp_rs485_link_get_interface()->mtu;

    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_EQ_SIZE(296U, mtu);
}

static void link_tx_calls_port_once_with_complete_frame(void)
{
    static const uint8_t expected_frame[] = {
        0xC0U,
        0x00U,
        0x01U,
        0x02U,
        0x03U,
        0x04U,
        0x00U,
        0x00U,
        0x00U,
        0x00U,
        0xC0U,
    };
    fake_port_t fake;
    const int init_result = initialize_link(&fake, 5U);
    csp_packet_t *packet = make_packet(0x01020304U, NULL, 0U);

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_TRUE(packet != NULL);

    const int tx_result = call_next_hop(packet);
    const size_t call_count = fake.transmit_call_count;
    const bool second_call = fake.second_transmit_detected;
    const char *operation = fake.last_operation_name;
    const size_t frame_length = fake.frame_length;
    uint8_t frame[sizeof(expected_frame)];
    memcpy(frame, fake.frame, sizeof(frame));

    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, tx_result);
    TEST_ASSERT_EQ_SIZE(1U, call_count);
    TEST_ASSERT_TRUE(!second_call);
    TEST_ASSERT_TRUE(strcmp("transmit_frame", operation) == 0);
    TEST_ASSERT_EQ_SIZE(sizeof(expected_frame), frame_length);
    TEST_ASSERT_TRUE(memcmp(expected_frame, frame, sizeof(frame)) == 0);
}

static void link_tx_timeout_uses_wire_time_plus_margin(void)
{
    uint8_t payload[CSP_RS485_INTERFACE_MTU];
    fake_port_t fake;

    memset(payload, 0xC0, sizeof(payload));
    const int init_result = initialize_link(&fake, 37U);
    csp_packet_t *packet =
        make_packet(0xC0C0C0C0U, payload, sizeof(payload));

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_TRUE(packet != NULL);

    const int tx_result = call_next_hop(packet);
    const size_t frame_length = fake.frame_length;
    const uint32_t timeout_ms = fake.timeout_ms;

    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, tx_result);
    TEST_ASSERT_TRUE(frame_length >= 607U);
    TEST_ASSERT_TRUE(frame_length <= CSP_RS485_TX_FRAME_MAX);
    TEST_ASSERT_EQ_SIZE(44U, timeout_ms);
}

static void link_tx_success_frees_packet(void)
{
    static const uint8_t payload[] = {0x11U, 0x22U};
    fake_port_t fake;
    const int init_result = initialize_link(&fake, 5U);
    const size_t buffers_before = (size_t) csp_buffer_remaining();
    csp_packet_t *packet =
        make_packet(0x10203040U, payload, sizeof(payload));

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_TRUE(packet != NULL);
    TEST_ASSERT_EQ_SIZE(buffers_before - 1U, csp_buffer_remaining());

    const int tx_result = call_next_hop(packet);
    const size_t buffers_after = (size_t) csp_buffer_remaining();

    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, tx_result);
    TEST_ASSERT_EQ_SIZE(buffers_before, buffers_after);
}

static void link_tx_failure_leaves_packet_owned_by_caller(void)
{
    static const uint8_t payload[] = {0x33U, 0x44U};
    fake_port_t fake;
    const int init_result = initialize_link(&fake, 5U);
    fake.transmit_result = CSP_RS485_PORT_ERROR;
    const size_t buffers_before = (size_t) csp_buffer_remaining();
    csp_packet_t *packet =
        make_packet(0x10203040U, payload, sizeof(payload));

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_TRUE(packet != NULL);

    const int tx_result = call_next_hop(packet);
    const size_t buffers_after_tx = (size_t) csp_buffer_remaining();
    csp_buffer_free(packet);
    const size_t buffers_after_cleanup = (size_t) csp_buffer_remaining();

    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(CSP_ERR_TX, tx_result);
    TEST_ASSERT_EQ_SIZE(buffers_before - 1U, buffers_after_tx);
    TEST_ASSERT_EQ_SIZE(buffers_before, buffers_after_cleanup);
}

static void link_tx_failure_restores_packet_length(void)
{
    static const uint8_t payload[] = {0x55U, 0x66U, 0x77U};
    fake_port_t fake;
    const int init_result = initialize_link(&fake, 5U);
    fake.transmit_result = CSP_RS485_PORT_ERROR;
    csp_packet_t *packet =
        make_packet(0x10203040U, payload, sizeof(payload));

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_TRUE(packet != NULL);

    const int tx_result = call_next_hop(packet);
    const uint16_t packet_length = packet->length;
    const int payload_result =
        memcmp(packet->data, payload, sizeof(payload));
    csp_buffer_free(packet);

    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(CSP_ERR_TX, tx_result);
    TEST_ASSERT_EQ_SIZE(sizeof(payload), packet_length);
    TEST_ASSERT_EQ_SIZE(0U, payload_result);
}

static void link_tx_failure_is_not_retried(void)
{
    fake_port_t fake;
    const int init_result = initialize_link(&fake, 5U);
    fake.transmit_result = CSP_RS485_PORT_ERROR;
    csp_packet_t *packet = make_packet(0x10203040U, NULL, 0U);

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_TRUE(packet != NULL);

    const int tx_result = call_next_hop(packet);
    const size_t call_count = fake.transmit_call_count;
    const bool second_call = fake.second_transmit_detected;
    csp_buffer_free(packet);

    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(CSP_ERR_TX, tx_result);
    TEST_ASSERT_EQ_SIZE(1U, call_count);
    TEST_ASSERT_TRUE(!second_call);
}

static void link_tx_during_recovery_fails_without_port_call(void)
{
    fake_port_t fake;
    const int init_result = initialize_link(&fake, 5U);
    fake.transmit_result = CSP_RS485_PORT_TIMEOUT;
    csp_packet_t *first_packet = make_packet(0x10203040U, NULL, 0U);

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_TRUE(first_packet != NULL);

    const int first_result = call_next_hop(first_packet);
    csp_buffer_free(first_packet);

    fake.transmit_result = CSP_RS485_PORT_OK;
    fake_port_clear_calls(&fake);
    csp_packet_t *second_packet = make_packet(0x50607080U, NULL, 0U);
    TEST_ASSERT_TRUE(second_packet != NULL);

    const int second_result = call_next_hop(second_packet);
    const size_t second_call_count = fake.transmit_call_count;
    const uint16_t second_length = second_packet->length;
    csp_buffer_free(second_packet);

    csp_rs485_health_t health;
    csp_rs485_link_get_health(&health);
    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(CSP_ERR_TX, first_result);
    TEST_ASSERT_EQ_SIZE(CSP_ERR_TX, second_result);
    TEST_ASSERT_EQ_SIZE(0U, second_call_count);
    TEST_ASSERT_EQ_SIZE(0U, second_length);
    TEST_ASSERT_EQ_SIZE(CSP_RS485_LINK_RECOVERING, health.state);
    TEST_ASSERT_EQ_SIZE(CSP_RS485_FAULT_TX_TIMEOUT, health.last_error);
    TEST_ASSERT_EQ_SIZE(1U, health.tx_timeouts);
    TEST_ASSERT_EQ_SIZE(1U, health.tx_failures);
}

static void link_tx_state_error_schedules_recovery(void)
{
    fake_port_t fake;
    const int init_result = initialize_link(&fake, 5U);
    fake.transmit_result = CSP_RS485_PORT_STATE_ERROR;
    csp_packet_t *packet = make_packet(0x10203040U, NULL, 0U);

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_TRUE(packet != NULL);

    const int tx_result = call_next_hop(packet);
    csp_rs485_health_t health;
    csp_rs485_link_get_health(&health);
    const size_t call_count = fake.transmit_call_count;
    csp_buffer_free(packet);

    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(CSP_ERR_TX, tx_result);
    TEST_ASSERT_EQ_SIZE(1U, call_count);
    TEST_ASSERT_EQ_SIZE(CSP_RS485_LINK_RECOVERING, health.state);
    TEST_ASSERT_EQ_SIZE(CSP_RS485_FAULT_TX_STATE, health.last_error);
    TEST_ASSERT_EQ_SIZE(1U, health.tx_failures);
}

static void link_tx_oversize_fails_before_port_call(void)
{
    uint8_t payload[CSP_RS485_INTERFACE_MTU + 1U] = {0};
    fake_port_t fake;
    const int init_result = initialize_link(&fake, 5U);
    const size_t buffers_before = (size_t) csp_buffer_remaining();
    csp_packet_t *packet =
        csp_buffer_get(CSP_RS485_CSP_BUFFER_DATA_SIZE);

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_TRUE(packet != NULL);

    packet->id.ext = 0x10203040U;
    packet->length = (uint16_t) sizeof(payload);
    memcpy(packet->data, payload, sizeof(payload));

    const int tx_result = call_next_hop(packet);
    const size_t call_count = fake.transmit_call_count;
    const uint16_t packet_length = packet->length;
    const size_t buffers_after_tx = (size_t) csp_buffer_remaining();
    csp_buffer_free(packet);
    const size_t buffers_after_cleanup = (size_t) csp_buffer_remaining();

    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(CSP_ERR_TX, tx_result);
    TEST_ASSERT_EQ_SIZE(0U, call_count);
    TEST_ASSERT_EQ_SIZE(sizeof(payload), packet_length);
    TEST_ASSERT_EQ_SIZE(buffers_before - 1U, buffers_after_tx);
    TEST_ASSERT_EQ_SIZE(buffers_before, buffers_after_cleanup);
}

const test_case_t link_tx_tests[] = {
    {
        "test_link_tx",
        "link_registers_one_named_interface",
        link_registers_one_named_interface,
    },
    {
        "test_link_tx",
        "link_reinit_does_not_register_duplicate_interface",
        link_reinit_does_not_register_duplicate_interface,
    },
    {
        "test_link_tx",
        "link_rejects_runtime_buffer_size_mismatch",
        link_rejects_runtime_buffer_size_mismatch,
    },
    {
        "test_link_tx",
        "link_sets_mtu_to_296",
        link_sets_mtu_to_296,
    },
    {
        "test_link_tx",
        "link_tx_calls_port_once_with_complete_frame",
        link_tx_calls_port_once_with_complete_frame,
    },
    {
        "test_link_tx",
        "link_tx_timeout_uses_wire_time_plus_margin",
        link_tx_timeout_uses_wire_time_plus_margin,
    },
    {
        "test_link_tx",
        "link_tx_success_frees_packet",
        link_tx_success_frees_packet,
    },
    {
        "test_link_tx",
        "link_tx_failure_leaves_packet_owned_by_caller",
        link_tx_failure_leaves_packet_owned_by_caller,
    },
    {
        "test_link_tx",
        "link_tx_failure_restores_packet_length",
        link_tx_failure_restores_packet_length,
    },
    {
        "test_link_tx",
        "link_tx_failure_is_not_retried",
        link_tx_failure_is_not_retried,
    },
    {
        "test_link_tx",
        "link_tx_during_recovery_fails_without_port_call",
        link_tx_during_recovery_fails_without_port_call,
    },
    {
        "test_link_tx",
        "link_tx_state_error_schedules_recovery",
        link_tx_state_error_schedules_recovery,
    },
    {
        "test_link_tx",
        "link_tx_oversize_fails_before_port_call",
        link_tx_oversize_fails_before_port_call,
    },
};

const size_t link_tx_test_count =
    sizeof(link_tx_tests) / sizeof(link_tx_tests[0]);
