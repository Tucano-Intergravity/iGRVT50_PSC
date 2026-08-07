#include "fakes/fake_domain.h"
#include "support/test.h"

#include <csp/csp.h>
#include <csp/sam_csp_config.h>
#include <csp/sam_csp_protocol.h>

#include <stdint.h>
#include <string.h>

#if defined(SAM_CSP_SERVICE_HAVE_IMPLEMENTATION)
#include <csp/sam_csp_service.h>
#include "csp/sam_csp_service_internal.h"
#else
typedef enum {
    SAM_CSP_DISPATCH_DROP = 0,
    SAM_CSP_DISPATCH_RESPOND,
    SAM_CSP_DISPATCH_DELEGATE_PING,
} sam_csp_dispatch_action_t;

static sam_csp_dispatch_action_t sam_csp_service_dispatch(
    uint8_t source_address,
    uint8_t destination_port,
    const uint8_t *request,
    size_t request_length,
    uint8_t *response,
    size_t response_capacity,
    size_t *response_length)
{
    (void) source_address;
    (void) destination_port;
    (void) request;
    (void) request_length;
    (void) response;
    (void) response_capacity;
    (void) response_length;
    return SAM_CSP_DISPATCH_DROP;
}

typedef struct {
    uint32_t malformed_packets;
    uint32_t allocation_failures;
    uint32_t send_failures;
    uint32_t rejected_peers;
    uint32_t dropped_ports;
} sam_csp_service_counters_t;

typedef struct {
    csp_socket_t *(*socket_create)(uint32_t options);
    int (*bind)(csp_socket_t *socket, uint8_t port);
    int (*listen)(csp_socket_t *socket, size_t backlog);
    csp_conn_t *(*accept)(csp_socket_t *socket, uint32_t timeout_ms);
    csp_packet_t *(*read)(csp_conn_t *connection, uint32_t timeout_ms);
    int (*connection_source)(csp_conn_t *connection);
    int (*connection_destination_port)(csp_conn_t *connection);
    csp_packet_t *(*buffer_get)(size_t data_size);
    void (*buffer_free)(void *packet);
    int (*send)(
        csp_conn_t *connection,
        csp_packet_t *packet,
        uint32_t timeout_ms);
    void (*service_handler)(csp_conn_t *connection, csp_packet_t *packet);
    int (*close)(csp_conn_t *connection);
} sam_csp_service_csp_ops_t;

static void sam_csp_service_reset(void)
{
}

static void sam_csp_service_test_bind_csp_ops(
    const sam_csp_service_csp_ops_t *ops)
{
    (void) ops;
}

static int sam_csp_service_prepare(void)
{
    return -1;
}

static void sam_csp_service_process_connection(csp_conn_t *connection)
{
    (void) connection;
}

static void sam_csp_service_get_counters(
    sam_csp_service_counters_t *counters)
{
    if (counters != NULL) {
        memset(counters, 0, sizeof(*counters));
    }
}
#endif

static const uint8_t valid_set_outputs[] = {
    0x01U, 0x01U, 0x12U, 0x34U, 0x00U,
    0x03U, 0x05U, 0x09U, 0x01U, 0x00U,
};
static const uint8_t valid_set_mode[] = {
    0x01U, 0x02U, 0x45U, 0x67U, 0x02U,
};
static const uint8_t valid_get_snapshot[] = {
    0x01U, 0x01U, 0x89U, 0xABU,
};
static const uint8_t valid_get_health[] = {
    0x01U, 0x01U, 0xCDU, 0xEFU,
};

static void reset_dispatch_fakes(void)
{
#if defined(SAM_CSP_SERVICE_HAVE_IMPLEMENTATION)
    sam_csp_service_reset();
#endif
    fake_domain_reset();
    fake_domain_bind();
}

static void assert_status_response(
    uint8_t port,
    const uint8_t *request,
    size_t request_length,
    uint8_t expected_status,
    uint8_t expected_detail)
{
    uint8_t response[SAM_CSP_HEALTH_RESPONSE_LENGTH];
    size_t response_length = 0xBEEFU;

    memset(response, 0xA5, sizeof(response));
    TEST_ASSERT_EQ_SIZE(
        SAM_CSP_DISPATCH_RESPOND,
        sam_csp_service_dispatch(
            SAM_CSP_PEER_ADDRESS,
            port,
            request,
            request_length,
            response,
            sizeof(response),
            &response_length));
    TEST_ASSERT_EQ_SIZE(SAM_CSP_RESPONSE_HEADER_LENGTH, response_length);
    TEST_ASSERT_EQ_SIZE(SAM_CSP_PROTOCOL_VERSION, response[0]);
    TEST_ASSERT_EQ_SIZE(request[1], response[1]);
    TEST_ASSERT_EQ_SIZE(request[2], response[2]);
    TEST_ASSERT_EQ_SIZE(request[3], response[3]);
    TEST_ASSERT_EQ_SIZE(expected_status, response[4]);
    TEST_ASSERT_EQ_SIZE(expected_detail, response[5]);
}

static void command_set_outputs_maps_domain_results_and_details(void)
{
    fake_domain_observations_t observed;

    reset_dispatch_fakes();
    assert_status_response(
        SAM_CSP_COMMAND_PORT,
        valid_set_outputs,
        sizeof(valid_set_outputs),
        SAM_CSP_STATUS_OK,
        0U);
    fake_domain_get_observations(&observed);
    TEST_ASSERT_EQ_SIZE(1U, observed.apply_outputs_calls);
    TEST_ASSERT_EQ_SIZE(0x0003U, observed.last_outputs.lpv_on_mask);
    TEST_ASSERT_EQ_SIZE(0x05U, observed.last_outputs.hpv_on_mask);
    TEST_ASSERT_EQ_SIZE(0x09U, observed.last_outputs.heater_on_mask);
    TEST_ASSERT_EQ_SIZE(1U, observed.last_outputs.spark_on);

    reset_dispatch_fakes();
    fake_domain_set_apply_result(SAM_CSP_DOMAIN_INVALID_STATE);
    fake_domain_set_current_mode(3U, SAM_CSP_DOMAIN_OK);
    assert_status_response(
        SAM_CSP_COMMAND_PORT,
        valid_set_outputs,
        sizeof(valid_set_outputs),
        SAM_CSP_STATUS_INVALID_STATE,
        3U);

    reset_dispatch_fakes();
    fake_domain_set_apply_result(SAM_CSP_DOMAIN_APPLY_FAILED);
    assert_status_response(
        SAM_CSP_COMMAND_PORT,
        valid_set_outputs,
        sizeof(valid_set_outputs),
        SAM_CSP_STATUS_APPLY_FAILED,
        1U);

    reset_dispatch_fakes();
    fake_domain_set_apply_result((sam_csp_domain_result_t) 99);
    assert_status_response(
        SAM_CSP_COMMAND_PORT,
        valid_set_outputs,
        sizeof(valid_set_outputs),
        SAM_CSP_STATUS_INTERNAL_ERROR,
        255U);
}

static void command_set_mode_maps_domain_results_and_details(void)
{
    fake_domain_observations_t observed;

    reset_dispatch_fakes();
    assert_status_response(
        SAM_CSP_COMMAND_PORT,
        valid_set_mode,
        sizeof(valid_set_mode),
        SAM_CSP_STATUS_OK,
        0U);
    fake_domain_get_observations(&observed);
    TEST_ASSERT_EQ_SIZE(1U, observed.request_mode_calls);
    TEST_ASSERT_EQ_SIZE(2U, observed.last_mode);

    reset_dispatch_fakes();
    fake_domain_set_mode_result(SAM_CSP_DOMAIN_INVALID_STATE);
    fake_domain_set_current_mode(1U, SAM_CSP_DOMAIN_OK);
    assert_status_response(
        SAM_CSP_COMMAND_PORT,
        valid_set_mode,
        sizeof(valid_set_mode),
        SAM_CSP_STATUS_INVALID_STATE,
        1U);

    reset_dispatch_fakes();
    fake_domain_set_mode_result(SAM_CSP_DOMAIN_APPLY_FAILED);
    assert_status_response(
        SAM_CSP_COMMAND_PORT,
        valid_set_mode,
        sizeof(valid_set_mode),
        SAM_CSP_STATUS_APPLY_FAILED,
        2U);

    reset_dispatch_fakes();
    fake_domain_set_mode_result((sam_csp_domain_result_t) 99);
    assert_status_response(
        SAM_CSP_COMMAND_PORT,
        valid_set_mode,
        sizeof(valid_set_mode),
        SAM_CSP_STATUS_INTERNAL_ERROR,
        255U);
}

static void command_port_reports_exact_bad_header_details(void)
{
    static const uint8_t bad_version[] = {
        0x02U, 0x01U, 0x12U, 0x34U, 0U, 0U, 0U, 0U, 0U, 0U,
    };
    static const uint8_t bad_opcode[] = {
        0x01U, 0x7FU, 0x12U, 0x34U,
    };
    static const uint8_t short_outputs[] = {
        0x01U, 0x01U, 0x12U, 0x34U, 0U, 0U, 0U, 0U, 0U,
    };
    static const uint8_t short_mode[] = {
        0x01U, 0x02U, 0x12U, 0x34U,
    };
    fake_domain_observations_t observed;

    reset_dispatch_fakes();
    assert_status_response(
        SAM_CSP_COMMAND_PORT,
        bad_version,
        sizeof(bad_version),
        SAM_CSP_STATUS_BAD_VERSION,
        0U);
    assert_status_response(
        SAM_CSP_COMMAND_PORT,
        bad_opcode,
        sizeof(bad_opcode),
        SAM_CSP_STATUS_BAD_OPCODE,
        0x7FU);
    assert_status_response(
        SAM_CSP_COMMAND_PORT,
        short_outputs,
        sizeof(short_outputs),
        SAM_CSP_STATUS_BAD_LENGTH,
        10U);
    assert_status_response(
        SAM_CSP_COMMAND_PORT,
        short_mode,
        sizeof(short_mode),
        SAM_CSP_STATUS_BAD_LENGTH,
        5U);
    fake_domain_get_observations(&observed);
    TEST_ASSERT_EQ_SIZE(0U, observed.apply_outputs_calls);
    TEST_ASSERT_EQ_SIZE(0U, observed.request_mode_calls);
}

static void telemetry_snapshot_maps_success_and_failure(void)
{
    const sam_csp_snapshot_t snapshot = {
        .sample_time_ms = 0x10203040U,
        .current_mode = 2U,
        .requested_mode = 3U,
        .validity_mask = 0x1001U,
        .pt_millivolt = {-1, 0, 0, 0, 0, 0, 0, 0, 7},
        .tc_microvolt = {-2, 0, 0, 8},
    };
    uint8_t response[SAM_CSP_SNAPSHOT_RESPONSE_LENGTH];
    size_t response_length = 0U;

    reset_dispatch_fakes();
    fake_domain_set_snapshot(&snapshot);
    memset(response, 0, sizeof(response));
    TEST_ASSERT_EQ_SIZE(
        SAM_CSP_DISPATCH_RESPOND,
        sam_csp_service_dispatch(
            SAM_CSP_PEER_ADDRESS,
            SAM_CSP_TELEMETRY_PORT,
            valid_get_snapshot,
            sizeof(valid_get_snapshot),
            response,
            sizeof(response),
            &response_length));
    TEST_ASSERT_EQ_SIZE(66U, response_length);
    TEST_ASSERT_EQ_SIZE(0x89U, response[2]);
    TEST_ASSERT_EQ_SIZE(0xABU, response[3]);
    TEST_ASSERT_EQ_SIZE(SAM_CSP_STATUS_OK, response[4]);
    TEST_ASSERT_EQ_SIZE(0x10U, response[6]);
    TEST_ASSERT_EQ_SIZE(0x40U, response[9]);
    TEST_ASSERT_EQ_SIZE(2U, response[10]);
    TEST_ASSERT_EQ_SIZE(3U, response[11]);
    TEST_ASSERT_EQ_SIZE(0x10U, response[12]);
    TEST_ASSERT_EQ_SIZE(0x01U, response[13]);
    TEST_ASSERT_EQ_SIZE(0xFFU, response[14]);
    TEST_ASSERT_EQ_SIZE(0xFFU, response[17]);
    TEST_ASSERT_EQ_SIZE(0xFFU, response[50]);
    TEST_ASSERT_EQ_SIZE(0xFEU, response[53]);
    TEST_ASSERT_EQ_SIZE(8U, response[65]);

    reset_dispatch_fakes();
    fake_domain_set_snapshot_result(SAM_CSP_DOMAIN_SNAPSHOT_FAILED);
    assert_status_response(
        SAM_CSP_TELEMETRY_PORT,
        valid_get_snapshot,
        sizeof(valid_get_snapshot),
        SAM_CSP_STATUS_INTERNAL_ERROR,
        2U);
}

static void telemetry_port_reports_exact_bad_header_details(void)
{
    static const uint8_t bad_version[] = {0x02U, 0x01U, 0x22U, 0x33U};
    static const uint8_t bad_opcode[] = {0x01U, 0x09U, 0x22U, 0x33U};
    static const uint8_t bad_length[] = {
        0x01U, 0x01U, 0x22U, 0x33U, 0x00U,
    };

    reset_dispatch_fakes();
    assert_status_response(
        SAM_CSP_TELEMETRY_PORT,
        bad_version,
        sizeof(bad_version),
        SAM_CSP_STATUS_BAD_VERSION,
        0U);
    assert_status_response(
        SAM_CSP_TELEMETRY_PORT,
        bad_opcode,
        sizeof(bad_opcode),
        SAM_CSP_STATUS_BAD_OPCODE,
        9U);
    assert_status_response(
        SAM_CSP_TELEMETRY_PORT,
        bad_length,
        sizeof(bad_length),
        SAM_CSP_STATUS_BAD_LENGTH,
        4U);
}

static void diagnostic_health_encodes_wide_uptime_and_counter_order(void)
{
    static const uint32_t counters[SAM_CSP_HEALTH_COUNTER_COUNT] = {
        1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 9U, 10U, 11U,
    };
    uint8_t response[SAM_CSP_HEALTH_RESPONSE_LENGTH];
    size_t response_length = 0U;

    reset_dispatch_fakes();
    fake_domain_set_tick_count(UINT32_C(0x40000000));
    fake_domain_set_link_health(2U, 4U, counters, SAM_CSP_HEALTH_COUNTER_COUNT);
    memset(response, 0, sizeof(response));
    TEST_ASSERT_EQ_SIZE(
        SAM_CSP_DISPATCH_RESPOND,
        sam_csp_service_dispatch(
            SAM_CSP_PEER_ADDRESS,
            SAM_CSP_DIAGNOSTIC_PORT,
            valid_get_health,
            sizeof(valid_get_health),
            response,
            sizeof(response),
            &response_length));
    TEST_ASSERT_EQ_SIZE(58U, response_length);
    TEST_ASSERT_EQ_SIZE(0x80U, response[6]);
    TEST_ASSERT_EQ_SIZE(0U, response[7]);
    TEST_ASSERT_EQ_SIZE(0U, response[8]);
    TEST_ASSERT_EQ_SIZE(0U, response[9]);
    TEST_ASSERT_EQ_SIZE(2U, response[10]);
    TEST_ASSERT_EQ_SIZE(4U, response[11]);
    TEST_ASSERT_EQ_SIZE(0U, response[12]);
    TEST_ASSERT_EQ_SIZE(0U, response[13]);
    for (size_t index = 0U; index < SAM_CSP_HEALTH_COUNTER_COUNT; ++index) {
        const size_t offset = 14U + (index * 4U);
        TEST_ASSERT_EQ_SIZE(0U, response[offset]);
        TEST_ASSERT_EQ_SIZE(0U, response[offset + 1U]);
        TEST_ASSERT_EQ_SIZE(0U, response[offset + 2U]);
        TEST_ASSERT_EQ_SIZE(index + 1U, response[offset + 3U]);
    }
}

static void diagnostic_port_reports_exact_bad_header_details(void)
{
    static const uint8_t bad_version[] = {0x02U, 0x01U, 0x44U, 0x55U};
    static const uint8_t bad_opcode[] = {0x01U, 0x08U, 0x44U, 0x55U};
    static const uint8_t bad_length[] = {
        0x01U, 0x01U, 0x44U, 0x55U, 0x00U,
    };

    reset_dispatch_fakes();
    assert_status_response(
        SAM_CSP_DIAGNOSTIC_PORT,
        bad_version,
        sizeof(bad_version),
        SAM_CSP_STATUS_BAD_VERSION,
        0U);
    assert_status_response(
        SAM_CSP_DIAGNOSTIC_PORT,
        bad_opcode,
        sizeof(bad_opcode),
        SAM_CSP_STATUS_BAD_OPCODE,
        8U);
    assert_status_response(
        SAM_CSP_DIAGNOSTIC_PORT,
        bad_length,
        sizeof(bad_length),
        SAM_CSP_STATUS_BAD_LENGTH,
        4U);
}

static void rejected_inputs_leave_output_and_domain_untouched(void)
{
    static const uint8_t short_header[] = {0x01U, 0x01U, 0x12U};
    uint8_t response[SAM_CSP_HEALTH_RESPONSE_LENGTH];
    uint8_t expected[SAM_CSP_HEALTH_RESPONSE_LENGTH];
    size_t response_length;
    fake_domain_observations_t observed;

    reset_dispatch_fakes();
    memset(response, 0xA5, sizeof(response));
    memset(expected, 0xA5, sizeof(expected));
    response_length = 0xCAFEU;
    TEST_ASSERT_EQ_SIZE(
        SAM_CSP_DISPATCH_DROP,
        sam_csp_service_dispatch(
            3U,
            SAM_CSP_COMMAND_PORT,
            valid_set_outputs,
            sizeof(valid_set_outputs),
            response,
            sizeof(response),
            &response_length));
    TEST_ASSERT_EQ_SIZE(0xCAFEU, response_length);
    TEST_ASSERT_TRUE(memcmp(expected, response, sizeof(response)) == 0);

    TEST_ASSERT_EQ_SIZE(
        SAM_CSP_DISPATCH_DROP,
        sam_csp_service_dispatch(
            SAM_CSP_PEER_ADDRESS,
            SAM_CSP_COMMAND_PORT,
            short_header,
            sizeof(short_header),
            response,
            sizeof(response),
            &response_length));
    TEST_ASSERT_EQ_SIZE(0xCAFEU, response_length);
    TEST_ASSERT_TRUE(memcmp(expected, response, sizeof(response)) == 0);

    TEST_ASSERT_EQ_SIZE(
        SAM_CSP_DISPATCH_DROP,
        sam_csp_service_dispatch(
            SAM_CSP_PEER_ADDRESS,
            SAM_CSP_COMMAND_PORT,
            NULL,
            sizeof(valid_set_outputs),
            response,
            sizeof(response),
            &response_length));
    TEST_ASSERT_EQ_SIZE(0xCAFEU, response_length);
    TEST_ASSERT_TRUE(memcmp(expected, response, sizeof(response)) == 0);

    TEST_ASSERT_EQ_SIZE(
        SAM_CSP_DISPATCH_DROP,
        sam_csp_service_dispatch(
            SAM_CSP_PEER_ADDRESS,
            SAM_CSP_COMMAND_PORT,
            valid_set_outputs,
            sizeof(valid_set_outputs),
            response,
            SAM_CSP_RESPONSE_HEADER_LENGTH - 1U,
            &response_length));
    TEST_ASSERT_EQ_SIZE(0xCAFEU, response_length);
    TEST_ASSERT_TRUE(memcmp(expected, response, sizeof(response)) == 0);
    fake_domain_get_observations(&observed);
    TEST_ASSERT_EQ_SIZE(0U, observed.apply_outputs_calls);
    TEST_ASSERT_EQ_SIZE(0U, observed.request_mode_calls);
    TEST_ASSERT_EQ_SIZE(0U, observed.get_snapshot_calls);
}

static void ping_delegates_but_reserved_and_unknown_ports_drop(void)
{
    static const uint8_t ports_to_drop[] = {
        CSP_CMP, CSP_PS, CSP_MEMFREE, CSP_REBOOT, CSP_BUF_FREE, CSP_UPTIME,
        7U, 8U, 9U, 13U, 63U,
    };
    uint8_t response[8];
    uint8_t expected[8];
    size_t response_length = 0x1234U;

    reset_dispatch_fakes();
    memset(response, 0xA5, sizeof(response));
    memset(expected, 0xA5, sizeof(expected));
    TEST_ASSERT_EQ_SIZE(
        SAM_CSP_DISPATCH_DELEGATE_PING,
        sam_csp_service_dispatch(
            SAM_CSP_PEER_ADDRESS,
            CSP_PING,
            NULL,
            0U,
            response,
            sizeof(response),
            &response_length));
    TEST_ASSERT_EQ_SIZE(0x1234U, response_length);
    TEST_ASSERT_TRUE(memcmp(expected, response, sizeof(response)) == 0);

    for (size_t index = 0U;
         index < (sizeof(ports_to_drop) / sizeof(ports_to_drop[0]));
         ++index) {
        TEST_ASSERT_EQ_SIZE(
            SAM_CSP_DISPATCH_DROP,
            sam_csp_service_dispatch(
                SAM_CSP_PEER_ADDRESS,
                ports_to_drop[index],
                valid_get_health,
                sizeof(valid_get_health),
                response,
                sizeof(response),
                &response_length));
        TEST_ASSERT_EQ_SIZE(0x1234U, response_length);
        TEST_ASSERT_TRUE(memcmp(expected, response, sizeof(response)) == 0);
    }
}

static void null_output_arguments_fail_without_domain_calls(void)
{
    uint8_t response[SAM_CSP_RESPONSE_HEADER_LENGTH] = {0};
    size_t response_length = 77U;
    fake_domain_observations_t observed;

    reset_dispatch_fakes();
    TEST_ASSERT_EQ_SIZE(
        SAM_CSP_DISPATCH_DROP,
        sam_csp_service_dispatch(
            SAM_CSP_PEER_ADDRESS,
            SAM_CSP_COMMAND_PORT,
            valid_set_outputs,
            sizeof(valid_set_outputs),
            NULL,
            sizeof(response),
            &response_length));
    TEST_ASSERT_EQ_SIZE(77U, response_length);
    TEST_ASSERT_EQ_SIZE(
        SAM_CSP_DISPATCH_DROP,
        sam_csp_service_dispatch(
            SAM_CSP_PEER_ADDRESS,
            SAM_CSP_COMMAND_PORT,
            valid_set_outputs,
            sizeof(valid_set_outputs),
            response,
            sizeof(response),
            NULL));
    fake_domain_get_observations(&observed);
    TEST_ASSERT_EQ_SIZE(0U, observed.apply_outputs_calls);
}

typedef union {
    uint64_t alignment;
    uint8_t bytes[sizeof(csp_packet_t) + SAM_CSP_BUFFER_DATA_SIZE];
} packet_storage_t;

typedef struct {
    size_t socket_calls;
    size_t bind_calls;
    size_t listen_calls;
    size_t read_calls;
    size_t buffer_get_calls;
    size_t buffer_free_calls;
    size_t send_calls;
    size_t handler_calls;
    size_t close_calls;
    uint32_t socket_options;
    uint8_t bind_port;
    size_t listen_backlog;
    void *freed_packets[3];
    csp_packet_t *sent_packet;
    csp_packet_t *handled_packet;
    int source_address;
    int destination_port;
    int send_result;
    int bind_result;
    int listen_result;
    uint8_t allocation_enabled;
    uint8_t read_enabled;
} fake_csp_observations_t;

static fake_csp_observations_t fake_csp;
static packet_storage_t request_storage;
static packet_storage_t response_storage;
static uint8_t fake_socket_object;
static uint8_t fake_connection_object;

static csp_packet_t *request_packet(void)
{
    return (csp_packet_t *) (void *) request_storage.bytes;
}

static csp_packet_t *response_packet(void)
{
    return (csp_packet_t *) (void *) response_storage.bytes;
}

static csp_socket_t *fake_socket_create(uint32_t options)
{
    ++fake_csp.socket_calls;
    fake_csp.socket_options = options;
    return (csp_socket_t *) (void *) &fake_socket_object;
}

static int fake_bind(csp_socket_t *socket, uint8_t port)
{
    if (socket != (csp_socket_t *) (void *) &fake_socket_object) {
        test_fail_true(__FILE__, __LINE__, "bind socket identity");
        return CSP_ERR_INVAL;
    }
    ++fake_csp.bind_calls;
    fake_csp.bind_port = port;
    return fake_csp.bind_result;
}

static int fake_listen(csp_socket_t *socket, size_t backlog)
{
    if (socket != (csp_socket_t *) (void *) &fake_socket_object) {
        test_fail_true(__FILE__, __LINE__, "listen socket identity");
        return CSP_ERR_INVAL;
    }
    ++fake_csp.listen_calls;
    fake_csp.listen_backlog = backlog;
    return fake_csp.listen_result;
}

static csp_conn_t *fake_accept(csp_socket_t *socket, uint32_t timeout_ms)
{
    (void) socket;
    (void) timeout_ms;
    return NULL;
}

static csp_packet_t *fake_read(
    csp_conn_t *connection,
    uint32_t timeout_ms)
{
    if (connection != (csp_conn_t *) (void *) &fake_connection_object) {
        test_fail_true(__FILE__, __LINE__, "read connection identity");
        return NULL;
    }
    if (timeout_ms != CSP_MAX_TIMEOUT) {
        test_fail_size(
            __FILE__, __LINE__, "read timeout", CSP_MAX_TIMEOUT, timeout_ms);
        return NULL;
    }
    ++fake_csp.read_calls;
    return (fake_csp.read_enabled != 0U) ? request_packet() : NULL;
}

static int fake_connection_source(csp_conn_t *connection)
{
    (void) connection;
    return fake_csp.source_address;
}

static int fake_connection_destination_port(csp_conn_t *connection)
{
    (void) connection;
    return fake_csp.destination_port;
}

static csp_packet_t *fake_buffer_get(size_t data_size)
{
    if (data_size != 0U) {
        test_fail_size(__FILE__, __LINE__, "buffer size", 0U, data_size);
        return NULL;
    }
    ++fake_csp.buffer_get_calls;
    return (fake_csp.allocation_enabled != 0U) ? response_packet() : NULL;
}

static void fake_buffer_free(void *packet)
{
    if (fake_csp.buffer_free_calls
        < (sizeof(fake_csp.freed_packets) / sizeof(fake_csp.freed_packets[0]))) {
        fake_csp.freed_packets[fake_csp.buffer_free_calls] = packet;
    }
    ++fake_csp.buffer_free_calls;
}

static int fake_send(
    csp_conn_t *connection,
    csp_packet_t *packet,
    uint32_t timeout_ms)
{
    if (connection != (csp_conn_t *) (void *) &fake_connection_object) {
        test_fail_true(__FILE__, __LINE__, "send connection identity");
        return 0;
    }
    if (timeout_ms != CSP_MAX_TIMEOUT) {
        test_fail_size(
            __FILE__, __LINE__, "send timeout", CSP_MAX_TIMEOUT, timeout_ms);
        return 0;
    }
    ++fake_csp.send_calls;
    fake_csp.sent_packet = packet;
    return fake_csp.send_result;
}

static void fake_service_handler(
    csp_conn_t *connection,
    csp_packet_t *packet)
{
    (void) connection;
    ++fake_csp.handler_calls;
    fake_csp.handled_packet = packet;
}

static int fake_close(csp_conn_t *connection)
{
    if (connection != (csp_conn_t *) (void *) &fake_connection_object) {
        test_fail_true(__FILE__, __LINE__, "close connection identity");
        return CSP_ERR_INVAL;
    }
    ++fake_csp.close_calls;
    return CSP_ERR_NONE;
}

static const sam_csp_service_csp_ops_t fake_csp_ops = {
    .socket_create = fake_socket_create,
    .bind = fake_bind,
    .listen = fake_listen,
    .accept = fake_accept,
    .read = fake_read,
    .connection_source = fake_connection_source,
    .connection_destination_port = fake_connection_destination_port,
    .buffer_get = fake_buffer_get,
    .buffer_free = fake_buffer_free,
    .send = fake_send,
    .service_handler = fake_service_handler,
    .close = fake_close,
};

static void reset_service_fakes(uint8_t destination_port)
{
    memset(&fake_csp, 0, sizeof(fake_csp));
    memset(&request_storage, 0, sizeof(request_storage));
    memset(&response_storage, 0, sizeof(response_storage));
    fake_csp.source_address = (int) SAM_CSP_PEER_ADDRESS;
    fake_csp.destination_port = (int) destination_port;
    fake_csp.send_result = 1;
    fake_csp.bind_result = CSP_ERR_NONE;
    fake_csp.listen_result = CSP_ERR_NONE;
    fake_csp.allocation_enabled = 1U;
    fake_csp.read_enabled = 1U;
    sam_csp_service_reset();
    fake_domain_reset();
    fake_domain_bind();
    sam_csp_service_test_bind_csp_ops(&fake_csp_ops);
}

static void set_request_payload(const uint8_t *payload, size_t length)
{
    csp_packet_t *const packet = request_packet();
    packet->length = (uint16_t) length;
    if ((payload != NULL) && (length > 0U)) {
        memcpy(packet->data, payload, length);
    }
}

static void service_prepare_uses_socket_any_and_exact_backlog(void)
{
    reset_service_fakes(SAM_CSP_COMMAND_PORT);
    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, sam_csp_service_prepare());
    TEST_ASSERT_EQ_SIZE(1U, fake_csp.socket_calls);
    TEST_ASSERT_EQ_SIZE(CSP_SO_NONE, fake_csp.socket_options);
    TEST_ASSERT_EQ_SIZE(1U, fake_csp.bind_calls);
    TEST_ASSERT_EQ_SIZE(CSP_ANY, fake_csp.bind_port);
    TEST_ASSERT_EQ_SIZE(1U, fake_csp.listen_calls);
    TEST_ASSERT_EQ_SIZE(SAM_CSP_CONN_QUEUE_LENGTH, fake_csp.listen_backlog);
}

static void successful_application_reply_transfers_only_response_ownership(void)
{
    reset_service_fakes(SAM_CSP_COMMAND_PORT);
    set_request_payload(valid_set_outputs, sizeof(valid_set_outputs));
    sam_csp_service_process_connection(
        (csp_conn_t *) (void *) &fake_connection_object);
    TEST_ASSERT_EQ_SIZE(1U, fake_csp.buffer_get_calls);
    TEST_ASSERT_TRUE(fake_csp.sent_packet == response_packet());
    TEST_ASSERT_TRUE(fake_csp.sent_packet != request_packet());
    TEST_ASSERT_EQ_SIZE(1U, fake_csp.send_calls);
    TEST_ASSERT_EQ_SIZE(1U, fake_csp.buffer_free_calls);
    TEST_ASSERT_TRUE(fake_csp.freed_packets[0] == request_packet());
    TEST_ASSERT_EQ_SIZE(1U, fake_csp.close_calls);
}

static void send_failure_frees_request_and_unsent_response(void)
{
    sam_csp_service_counters_t counters;

    reset_service_fakes(SAM_CSP_COMMAND_PORT);
    fake_csp.send_result = 0;
    set_request_payload(valid_set_outputs, sizeof(valid_set_outputs));
    sam_csp_service_process_connection(
        (csp_conn_t *) (void *) &fake_connection_object);
    TEST_ASSERT_EQ_SIZE(2U, fake_csp.buffer_free_calls);
    TEST_ASSERT_TRUE(fake_csp.freed_packets[0] == response_packet());
    TEST_ASSERT_TRUE(fake_csp.freed_packets[1] == request_packet());
    sam_csp_service_get_counters(&counters);
    TEST_ASSERT_EQ_SIZE(1U, counters.send_failures);
}

static void allocation_failure_frees_request_and_counts_failure(void)
{
    sam_csp_service_counters_t counters;

    reset_service_fakes(SAM_CSP_COMMAND_PORT);
    fake_csp.allocation_enabled = 0U;
    set_request_payload(valid_set_outputs, sizeof(valid_set_outputs));
    sam_csp_service_process_connection(
        (csp_conn_t *) (void *) &fake_connection_object);
    TEST_ASSERT_EQ_SIZE(1U, fake_csp.buffer_free_calls);
    TEST_ASSERT_TRUE(fake_csp.freed_packets[0] == request_packet());
    TEST_ASSERT_EQ_SIZE(0U, fake_csp.send_calls);
    sam_csp_service_get_counters(&counters);
    TEST_ASSERT_EQ_SIZE(1U, counters.allocation_failures);
}

static void ping_transfers_original_once_without_allocating_or_freeing(void)
{
    reset_service_fakes(CSP_PING);
    set_request_payload(NULL, 0U);
    sam_csp_service_process_connection(
        (csp_conn_t *) (void *) &fake_connection_object);
    TEST_ASSERT_EQ_SIZE(1U, fake_csp.handler_calls);
    TEST_ASSERT_TRUE(fake_csp.handled_packet == request_packet());
    TEST_ASSERT_EQ_SIZE(0U, fake_csp.buffer_get_calls);
    TEST_ASSERT_EQ_SIZE(0U, fake_csp.buffer_free_calls);
    TEST_ASSERT_EQ_SIZE(1U, fake_csp.close_calls);
}

static void reserved_port_never_reaches_default_handler(void)
{
    sam_csp_service_counters_t counters;

    reset_service_fakes(CSP_REBOOT);
    set_request_payload(valid_get_health, sizeof(valid_get_health));
    sam_csp_service_process_connection(
        (csp_conn_t *) (void *) &fake_connection_object);
    TEST_ASSERT_EQ_SIZE(0U, fake_csp.handler_calls);
    TEST_ASSERT_EQ_SIZE(0U, fake_csp.buffer_get_calls);
    TEST_ASSERT_EQ_SIZE(1U, fake_csp.buffer_free_calls);
    TEST_ASSERT_TRUE(fake_csp.freed_packets[0] == request_packet());
    sam_csp_service_get_counters(&counters);
    TEST_ASSERT_EQ_SIZE(1U, counters.dropped_ports);
}

static void rejected_peer_and_malformed_packet_increment_separate_counters(void)
{
    sam_csp_service_counters_t counters;

    reset_service_fakes(SAM_CSP_COMMAND_PORT);
    fake_csp.source_address = 3;
    set_request_payload(valid_set_outputs, sizeof(valid_set_outputs));
    sam_csp_service_process_connection(
        (csp_conn_t *) (void *) &fake_connection_object);
    sam_csp_service_get_counters(&counters);
    TEST_ASSERT_EQ_SIZE(1U, counters.rejected_peers);
    TEST_ASSERT_EQ_SIZE(0U, counters.malformed_packets);
    TEST_ASSERT_EQ_SIZE(0U, fake_csp.buffer_get_calls);

    reset_service_fakes(SAM_CSP_COMMAND_PORT);
    set_request_payload(valid_set_outputs, 3U);
    sam_csp_service_process_connection(
        (csp_conn_t *) (void *) &fake_connection_object);
    sam_csp_service_get_counters(&counters);
    TEST_ASSERT_EQ_SIZE(0U, counters.rejected_peers);
    TEST_ASSERT_EQ_SIZE(1U, counters.malformed_packets);
    TEST_ASSERT_EQ_SIZE(0U, fake_csp.buffer_get_calls);
}

static void null_read_closes_connection_without_touching_buffers(void)
{
    reset_service_fakes(SAM_CSP_COMMAND_PORT);
    fake_csp.read_enabled = 0U;
    sam_csp_service_process_connection(
        (csp_conn_t *) (void *) &fake_connection_object);
    TEST_ASSERT_EQ_SIZE(1U, fake_csp.read_calls);
    TEST_ASSERT_EQ_SIZE(0U, fake_csp.buffer_get_calls);
    TEST_ASSERT_EQ_SIZE(0U, fake_csp.buffer_free_calls);
    TEST_ASSERT_EQ_SIZE(1U, fake_csp.close_calls);
}

const test_case_t sam_csp_service_tests[] = {
    {"sam_csp_service", "SET_OUTPUTS maps domain results and details", command_set_outputs_maps_domain_results_and_details},
    {"sam_csp_service", "SET_MODE maps domain results and details", command_set_mode_maps_domain_results_and_details},
    {"sam_csp_service", "command BAD responses have exact details", command_port_reports_exact_bad_header_details},
    {"sam_csp_service", "snapshot maps success and failure", telemetry_snapshot_maps_success_and_failure},
    {"sam_csp_service", "telemetry BAD responses have exact details", telemetry_port_reports_exact_bad_header_details},
    {"sam_csp_service", "health uses wide uptime and counter order", diagnostic_health_encodes_wide_uptime_and_counter_order},
    {"sam_csp_service", "diagnostic BAD responses have exact details", diagnostic_port_reports_exact_bad_header_details},
    {"sam_csp_service", "rejected inputs preserve caller output", rejected_inputs_leave_output_and_domain_untouched},
    {"sam_csp_service", "only PING delegates among non-application ports", ping_delegates_but_reserved_and_unknown_ports_drop},
    {"sam_csp_service", "null output arguments do not call domain", null_output_arguments_fail_without_domain_calls},
    {"sam_csp_service", "prepare binds one ANY socket with backlog ten", service_prepare_uses_socket_any_and_exact_backlog},
    {"sam_csp_service", "successful reply transfers response ownership", successful_application_reply_transfers_only_response_ownership},
    {"sam_csp_service", "send failure frees both packets", send_failure_frees_request_and_unsent_response},
    {"sam_csp_service", "allocation failure frees request", allocation_failure_frees_request_and_counts_failure},
    {"sam_csp_service", "PING transfers original packet once", ping_transfers_original_once_without_allocating_or_freeing},
    {"sam_csp_service", "reserved ports never invoke default handler", reserved_port_never_reaches_default_handler},
    {"sam_csp_service", "peer and malformed counters stay distinct", rejected_peer_and_malformed_packet_increment_separate_counters},
    {"sam_csp_service", "null read only closes connection", null_read_closes_connection_without_touching_buffers},
};

const size_t sam_csp_service_test_count =
    sizeof(sam_csp_service_tests) / sizeof(sam_csp_service_tests[0]);
