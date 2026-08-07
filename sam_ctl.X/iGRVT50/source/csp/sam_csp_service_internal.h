#ifndef SAM_CSP_SERVICE_INTERNAL_H
#define SAM_CSP_SERVICE_INTERNAL_H

#include <csp/csp.h>

#include <csp/sam_csp_domain.h>

#include <csp_rs485_link.h>

#include <stddef.h>
#include <stdint.h>

typedef struct {
    sam_csp_domain_result_t (*apply_outputs)(
        const sam_csp_set_outputs_request_t *request);
    sam_csp_domain_result_t (*request_mode)(uint8_t mode);
    sam_csp_domain_result_t (*get_snapshot)(sam_csp_snapshot_t *snapshot);
    sam_csp_domain_result_t (*get_current_mode)(uint8_t *mode);
    uint32_t (*get_tick_count)(void);
    void (*get_link_health)(csp_rs485_health_t *health);
} sam_csp_service_dependencies_t;

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

void sam_csp_service_process_connection(csp_conn_t *connection);

#ifdef CSP_RS485_HOST_TEST
void sam_csp_service_test_bind_dependencies(
    const sam_csp_service_dependencies_t *dependencies);
void sam_csp_service_test_bind_csp_ops(
    const sam_csp_service_csp_ops_t *ops);
#endif

#endif
