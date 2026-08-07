#include <csp/sam_csp_service.h>

#include "sam_csp_service_internal.h"

#include "FreeRTOS.h"
#include "task.h"

#include <csp/csp.h>
#include <csp/csp_buffer.h>
#include <csp/sam_csp_config.h>
#include <csp/sam_csp_domain.h>
#include <csp/sam_csp_protocol.h>

#include <csp_rs485_link.h>

#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

enum {
    SAM_CSP_DETAIL_APPLY_OUTPUTS = 1U,
    SAM_CSP_DETAIL_APPLY_MODE = 2U,
    SAM_CSP_DETAIL_SNAPSHOT = 2U,
    SAM_CSP_DETAIL_UNSPECIFIED = 255U,
};

typedef struct {
    _Atomic uint32_t malformed_packets;
    _Atomic uint32_t allocation_failures;
    _Atomic uint32_t send_failures;
    _Atomic uint32_t rejected_peers;
    _Atomic uint32_t dropped_ports;
} sam_csp_service_atomic_counters_t;

static sam_csp_domain_result_t default_get_current_mode(uint8_t *mode)
{
    sam_csp_snapshot_t snapshot;
    if (mode == NULL) {
        return SAM_CSP_DOMAIN_SNAPSHOT_FAILED;
    }
    const sam_csp_domain_result_t result =
        sam_csp_domain_get_snapshot(&snapshot);
    if (result == SAM_CSP_DOMAIN_OK) {
        *mode = snapshot.current_mode;
    }
    return result;
}

static uint32_t default_get_tick_count(void)
{
    return (uint32_t) xTaskGetTickCount();
}

static csp_packet_t *default_buffer_get(size_t data_size)
{
    return csp_buffer_get(data_size);
}

static const sam_csp_service_dependencies_t default_dependencies = {
    .apply_outputs = sam_csp_domain_apply_outputs,
    .request_mode = sam_csp_domain_request_mode,
    .get_snapshot = sam_csp_domain_get_snapshot,
    .get_current_mode = default_get_current_mode,
    .get_tick_count = default_get_tick_count,
    .get_link_health = csp_rs485_link_get_health,
};

static const sam_csp_service_csp_ops_t default_csp_ops = {
    .socket_create = csp_socket,
    .bind = csp_bind,
    .listen = csp_listen,
    .accept = csp_accept,
    .read = csp_read,
    .connection_source = csp_conn_src,
    .connection_destination_port = csp_conn_dport,
    .buffer_get = default_buffer_get,
    .buffer_free = csp_buffer_free,
    .send = csp_send,
    .service_handler = csp_service_handler,
    .close = csp_close,
};

static const sam_csp_service_dependencies_t *dependencies =
    &default_dependencies;
static const sam_csp_service_csp_ops_t *csp_ops = &default_csp_ops;
static sam_csp_service_atomic_counters_t service_counters;
static csp_socket_t *service_socket;

static bool is_application_port(uint8_t port)
{
    return (port == SAM_CSP_COMMAND_PORT)
        || (port == SAM_CSP_TELEMETRY_PORT)
        || (port == SAM_CSP_DIAGNOSTIC_PORT);
}

static size_t maximum_response_length(uint8_t port)
{
    switch (port) {
        case SAM_CSP_COMMAND_PORT:
            return SAM_CSP_RESPONSE_HEADER_LENGTH;
        case SAM_CSP_TELEMETRY_PORT:
            return SAM_CSP_SNAPSHOT_RESPONSE_LENGTH;
        case SAM_CSP_DIAGNOSTIC_PORT:
            return SAM_CSP_HEALTH_RESPONSE_LENGTH;
        default:
            return 0U;
    }
}

static uint16_t request_transaction_id(const uint8_t *request)
{
    return (uint16_t) (((uint16_t) request[2] << 8U)
        | (uint16_t) request[3]);
}

static sam_csp_dispatch_action_t emit_status(
    uint8_t opcode,
    uint16_t transaction_id,
    sam_csp_status_t status,
    uint8_t detail,
    uint8_t *response,
    size_t response_capacity,
    size_t *response_length)
{
    uint8_t encoded[SAM_CSP_RESPONSE_HEADER_LENGTH];
    const size_t length = sam_csp_encode_status(
        opcode,
        transaction_id,
        status,
        detail,
        encoded,
        sizeof(encoded));
    if ((length == 0U) || (length > response_capacity)) {
        return SAM_CSP_DISPATCH_DROP;
    }
    memcpy(response, encoded, length);
    *response_length = length;
    return SAM_CSP_DISPATCH_RESPOND;
}

static uint8_t invalid_state_detail(void)
{
    uint8_t mode = SAM_CSP_DETAIL_UNSPECIFIED;
    if ((dependencies->get_current_mode == NULL)
        || (dependencies->get_current_mode(&mode) != SAM_CSP_DOMAIN_OK)) {
        mode = SAM_CSP_DETAIL_UNSPECIFIED;
    }
    return mode;
}

static sam_csp_dispatch_action_t emit_domain_status(
    uint8_t opcode,
    uint16_t transaction_id,
    sam_csp_domain_result_t domain_result,
    uint8_t apply_failed_detail,
    uint8_t *response,
    size_t response_capacity,
    size_t *response_length)
{
    sam_csp_status_t status;
    uint8_t detail;

    switch (domain_result) {
        case SAM_CSP_DOMAIN_OK:
            status = SAM_CSP_STATUS_OK;
            detail = 0U;
            break;
        case SAM_CSP_DOMAIN_INVALID_STATE:
            status = SAM_CSP_STATUS_INVALID_STATE;
            detail = invalid_state_detail();
            break;
        case SAM_CSP_DOMAIN_APPLY_FAILED:
            status = SAM_CSP_STATUS_APPLY_FAILED;
            detail = apply_failed_detail;
            break;
        default:
            status = SAM_CSP_STATUS_INTERNAL_ERROR;
            detail = SAM_CSP_DETAIL_UNSPECIFIED;
            break;
    }

    return emit_status(
        opcode,
        transaction_id,
        status,
        detail,
        response,
        response_capacity,
        response_length);
}

static sam_csp_dispatch_action_t dispatch_command(
    const uint8_t *request,
    size_t request_length,
    uint8_t *response,
    size_t response_capacity,
    size_t *response_length)
{
    const uint8_t opcode = request[1];
    const uint16_t transaction_id = request_transaction_id(request);

    if (request[0] != SAM_CSP_PROTOCOL_VERSION) {
        return emit_status(
            opcode, transaction_id, SAM_CSP_STATUS_BAD_VERSION, 0U,
            response, response_capacity, response_length);
    }
    if ((opcode != SAM_CSP_OPCODE_SET_OUTPUTS)
        && (opcode != SAM_CSP_OPCODE_SET_MODE)) {
        return emit_status(
            opcode, transaction_id, SAM_CSP_STATUS_BAD_OPCODE, opcode,
            response, response_capacity, response_length);
    }

    if (opcode == SAM_CSP_OPCODE_SET_OUTPUTS) {
        sam_csp_set_outputs_request_t decoded;
        uint8_t detail = 0U;
        const sam_csp_status_t decode_result = sam_csp_decode_set_outputs(
            request,
            request_length,
            &decoded,
            &detail);
        if (decode_result != SAM_CSP_STATUS_OK) {
            return emit_status(
                opcode,
                transaction_id,
                decode_result,
                detail,
                response,
                response_capacity,
                response_length);
        }
        return emit_domain_status(
            opcode,
            transaction_id,
            dependencies->apply_outputs(&decoded),
            SAM_CSP_DETAIL_APPLY_OUTPUTS,
            response,
            response_capacity,
            response_length);
    }

    if (request_length != SAM_CSP_SET_MODE_REQUEST_LENGTH) {
        return emit_status(
            opcode,
            transaction_id,
            SAM_CSP_STATUS_BAD_LENGTH,
            SAM_CSP_SET_MODE_REQUEST_LENGTH,
            response,
            response_capacity,
            response_length);
    }
    return emit_domain_status(
        opcode,
        transaction_id,
        dependencies->request_mode(request[4]),
        SAM_CSP_DETAIL_APPLY_MODE,
        response,
        response_capacity,
        response_length);
}

static sam_csp_dispatch_action_t dispatch_snapshot(
    const uint8_t *request,
    size_t request_length,
    uint8_t *response,
    size_t response_capacity,
    size_t *response_length)
{
    const uint8_t opcode = request[1];
    const uint16_t transaction_id = request_transaction_id(request);

    if (request[0] != SAM_CSP_PROTOCOL_VERSION) {
        return emit_status(
            opcode, transaction_id, SAM_CSP_STATUS_BAD_VERSION, 0U,
            response, response_capacity, response_length);
    }
    if (opcode != SAM_CSP_OPCODE_GET_SNAPSHOT) {
        return emit_status(
            opcode, transaction_id, SAM_CSP_STATUS_BAD_OPCODE, opcode,
            response, response_capacity, response_length);
    }
    if (request_length != SAM_CSP_REQUEST_HEADER_LENGTH) {
        return emit_status(
            opcode,
            transaction_id,
            SAM_CSP_STATUS_BAD_LENGTH,
            SAM_CSP_REQUEST_HEADER_LENGTH,
            response,
            response_capacity,
            response_length);
    }

    sam_csp_snapshot_t snapshot;
    if (dependencies->get_snapshot(&snapshot) != SAM_CSP_DOMAIN_OK) {
        return emit_status(
            opcode,
            transaction_id,
            SAM_CSP_STATUS_INTERNAL_ERROR,
            SAM_CSP_DETAIL_SNAPSHOT,
            response,
            response_capacity,
            response_length);
    }

    uint8_t encoded[SAM_CSP_SNAPSHOT_RESPONSE_LENGTH];
    const size_t length = sam_csp_encode_snapshot(
        opcode,
        transaction_id,
        &snapshot,
        encoded,
        sizeof(encoded));
    if ((length == 0U) || (length > response_capacity)) {
        return SAM_CSP_DISPATCH_DROP;
    }
    memcpy(response, encoded, length);
    *response_length = length;
    return SAM_CSP_DISPATCH_RESPOND;
}

static void copy_link_health(
    const csp_rs485_health_t *link,
    sam_csp_health_t *health)
{
    health->link_state = (uint8_t) link->state;
    health->last_error = (uint8_t) link->last_error;
    health->counters[0] = link->uart_errors;
    health->counters[1] = link->dma_errors;
    health->counters[2] = link->tx_timeouts;
    health->counters[3] = link->tx_failures;
    health->counters[4] = link->protocol_errors;
    health->counters[5] = link->stream_dropped_bytes;
    health->counters[6] = link->stream_high_watermark;
    health->counters[7] = link->stream_discontinuities;
    health->counters[8] = link->recovery_attempts;
    health->counters[9] = link->recovery_successes;
    health->counters[10] = link->recovery_failures;
}

static sam_csp_dispatch_action_t dispatch_health(
    const uint8_t *request,
    size_t request_length,
    uint8_t *response,
    size_t response_capacity,
    size_t *response_length)
{
    const uint8_t opcode = request[1];
    const uint16_t transaction_id = request_transaction_id(request);

    if (request[0] != SAM_CSP_PROTOCOL_VERSION) {
        return emit_status(
            opcode, transaction_id, SAM_CSP_STATUS_BAD_VERSION, 0U,
            response, response_capacity, response_length);
    }
    if (opcode != SAM_CSP_OPCODE_GET_HEALTH) {
        return emit_status(
            opcode, transaction_id, SAM_CSP_STATUS_BAD_OPCODE, opcode,
            response, response_capacity, response_length);
    }
    if (request_length != SAM_CSP_REQUEST_HEADER_LENGTH) {
        return emit_status(
            opcode,
            transaction_id,
            SAM_CSP_STATUS_BAD_LENGTH,
            SAM_CSP_REQUEST_HEADER_LENGTH,
            response,
            response_capacity,
            response_length);
    }

    csp_rs485_health_t link_health;
    sam_csp_health_t health;
    memset(&link_health, 0, sizeof(link_health));
    memset(&health, 0, sizeof(health));
    dependencies->get_link_health(&link_health);
    health.uptime_ms = (uint32_t)
        (((uint64_t) dependencies->get_tick_count() * UINT64_C(1000))
            / (uint64_t) configTICK_RATE_HZ);
    copy_link_health(&link_health, &health);

    uint8_t encoded[SAM_CSP_HEALTH_RESPONSE_LENGTH];
    const size_t length = sam_csp_encode_health(
        opcode,
        transaction_id,
        &health,
        encoded,
        sizeof(encoded));
    if ((length == 0U) || (length > response_capacity)) {
        return SAM_CSP_DISPATCH_DROP;
    }
    memcpy(response, encoded, length);
    *response_length = length;
    return SAM_CSP_DISPATCH_RESPOND;
}

sam_csp_dispatch_action_t sam_csp_service_dispatch(
    uint8_t source_address,
    uint8_t destination_port,
    const uint8_t *request,
    size_t request_length,
    uint8_t *response,
    size_t response_capacity,
    size_t *response_length)
{
    if (source_address != SAM_CSP_PEER_ADDRESS) {
        return SAM_CSP_DISPATCH_DROP;
    }
    if (destination_port == CSP_PING) {
        return SAM_CSP_DISPATCH_DELEGATE_PING;
    }
    if (!is_application_port(destination_port)) {
        return SAM_CSP_DISPATCH_DROP;
    }
    if ((request == NULL) || (request_length < SAM_CSP_REQUEST_HEADER_LENGTH)) {
        return SAM_CSP_DISPATCH_DROP;
    }

    const size_t required_capacity = maximum_response_length(destination_port);
    if ((response == NULL) || (response_length == NULL)
        || (response_capacity < required_capacity)) {
        return SAM_CSP_DISPATCH_DROP;
    }

    switch (destination_port) {
        case SAM_CSP_COMMAND_PORT:
            return dispatch_command(
                request,
                request_length,
                response,
                response_capacity,
                response_length);
        case SAM_CSP_TELEMETRY_PORT:
            return dispatch_snapshot(
                request,
                request_length,
                response,
                response_capacity,
                response_length);
        case SAM_CSP_DIAGNOSTIC_PORT:
            return dispatch_health(
                request,
                request_length,
                response,
                response_capacity,
                response_length);
        default:
            return SAM_CSP_DISPATCH_DROP;
    }
}

static void increment_counter(_Atomic uint32_t *counter)
{
    (void) atomic_fetch_add_explicit(counter, 1U, memory_order_relaxed);
}

void sam_csp_service_get_counters(sam_csp_service_counters_t *counters)
{
    if (counters == NULL) {
        return;
    }
    counters->malformed_packets = atomic_load_explicit(
        &service_counters.malformed_packets, memory_order_relaxed);
    counters->allocation_failures = atomic_load_explicit(
        &service_counters.allocation_failures, memory_order_relaxed);
    counters->send_failures = atomic_load_explicit(
        &service_counters.send_failures, memory_order_relaxed);
    counters->rejected_peers = atomic_load_explicit(
        &service_counters.rejected_peers, memory_order_relaxed);
    counters->dropped_ports = atomic_load_explicit(
        &service_counters.dropped_ports, memory_order_relaxed);
}

void sam_csp_service_reset(void)
{
    atomic_store_explicit(
        &service_counters.malformed_packets, 0U, memory_order_relaxed);
    atomic_store_explicit(
        &service_counters.allocation_failures, 0U, memory_order_relaxed);
    atomic_store_explicit(
        &service_counters.send_failures, 0U, memory_order_relaxed);
    atomic_store_explicit(
        &service_counters.rejected_peers, 0U, memory_order_relaxed);
    atomic_store_explicit(
        &service_counters.dropped_ports, 0U, memory_order_relaxed);
#ifdef CSP_RS485_HOST_TEST
    dependencies = &default_dependencies;
    csp_ops = &default_csp_ops;
    service_socket = NULL;
#endif
}

int sam_csp_service_prepare(void)
{
    csp_socket_t *const socket = csp_ops->socket_create(CSP_SO_NONE);
    if (socket == NULL) {
        return SAM_CSP_SERVICE_ERR_SOCKET;
    }
    if (csp_ops->bind(socket, CSP_ANY) != CSP_ERR_NONE) {
        return SAM_CSP_SERVICE_ERR_BIND;
    }
    if (csp_ops->listen(socket, SAM_CSP_CONN_QUEUE_LENGTH) != CSP_ERR_NONE) {
        return SAM_CSP_SERVICE_ERR_LISTEN;
    }
    service_socket = socket;
    return CSP_ERR_NONE;
}

void sam_csp_service_process_connection(csp_conn_t *connection)
{
    if (connection == NULL) {
        return;
    }

    csp_packet_t *const request = csp_ops->read(connection, CSP_MAX_TIMEOUT);
    if (request == NULL) {
        (void) csp_ops->close(connection);
        return;
    }

    const int source = csp_ops->connection_source(connection);
    const int destination_port =
        csp_ops->connection_destination_port(connection);
    if (source != (int) SAM_CSP_PEER_ADDRESS) {
        increment_counter(&service_counters.rejected_peers);
        csp_ops->buffer_free(request);
        (void) csp_ops->close(connection);
        return;
    }

    if (destination_port == (int) CSP_PING) {
        csp_ops->service_handler(connection, request);
        (void) csp_ops->close(connection);
        return;
    }

    if ((destination_port < 0)
        || !is_application_port((uint8_t) destination_port)) {
        increment_counter(&service_counters.dropped_ports);
        csp_ops->buffer_free(request);
        (void) csp_ops->close(connection);
        return;
    }

    if (request->length < SAM_CSP_REQUEST_HEADER_LENGTH) {
        increment_counter(&service_counters.malformed_packets);
        csp_ops->buffer_free(request);
        (void) csp_ops->close(connection);
        return;
    }

    csp_packet_t *const response = csp_ops->buffer_get(0U);
    if (response == NULL) {
        increment_counter(&service_counters.allocation_failures);
        csp_ops->buffer_free(request);
        (void) csp_ops->close(connection);
        return;
    }

    size_t response_length = 0U;
    const sam_csp_dispatch_action_t action = sam_csp_service_dispatch(
        (uint8_t) source,
        (uint8_t) destination_port,
        request->data,
        request->length,
        response->data,
        SAM_CSP_BUFFER_DATA_SIZE,
        &response_length);
    if ((action != SAM_CSP_DISPATCH_RESPOND)
        || (response_length > UINT16_MAX)) {
        increment_counter(&service_counters.malformed_packets);
        csp_ops->buffer_free(response);
        csp_ops->buffer_free(request);
        (void) csp_ops->close(connection);
        return;
    }

    response->length = (uint16_t) response_length;
    if (csp_ops->send(connection, response, CSP_MAX_TIMEOUT) == 0) {
        increment_counter(&service_counters.send_failures);
        csp_ops->buffer_free(response);
    }
    csp_ops->buffer_free(request);
    (void) csp_ops->close(connection);
}

void sam_csp_service_task(void *argument)
{
    (void) argument;
    for (;;) {
        csp_conn_t *const connection =
            csp_ops->accept(service_socket, CSP_MAX_TIMEOUT);
        if (connection != NULL) {
            sam_csp_service_process_connection(connection);
        }
    }
}

#ifdef CSP_RS485_HOST_TEST
void sam_csp_service_test_bind_dependencies(
    const sam_csp_service_dependencies_t *test_dependencies)
{
    dependencies = (test_dependencies != NULL)
        ? test_dependencies : &default_dependencies;
}

void sam_csp_service_test_bind_csp_ops(
    const sam_csp_service_csp_ops_t *ops)
{
    csp_ops = (ops != NULL) ? ops : &default_csp_ops;
}
#endif
