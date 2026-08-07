#ifndef SAM_CSP_SERVICE_H
#define SAM_CSP_SERVICE_H

#include <stddef.h>
#include <stdint.h>

typedef enum {
    SAM_CSP_DISPATCH_DROP = 0,
    SAM_CSP_DISPATCH_RESPOND,
    SAM_CSP_DISPATCH_DELEGATE_PING,
} sam_csp_dispatch_action_t;

typedef struct {
    uint32_t malformed_packets;
    uint32_t allocation_failures;
    uint32_t send_failures;
    uint32_t rejected_peers;
    uint32_t dropped_ports;
} sam_csp_service_counters_t;

#define SAM_CSP_SERVICE_ERR_SOCKET (-6)
#define SAM_CSP_SERVICE_ERR_BIND (-7)
#define SAM_CSP_SERVICE_ERR_LISTEN (-8)

sam_csp_dispatch_action_t sam_csp_service_dispatch(
    uint8_t source_address,
    uint8_t destination_port,
    const uint8_t *request,
    size_t request_length,
    uint8_t *response,
    size_t response_capacity,
    size_t *response_length);

void sam_csp_service_get_counters(sam_csp_service_counters_t *counters);
void sam_csp_service_reset(void);
int sam_csp_service_prepare(void);
void sam_csp_service_task(void *argument);

#endif
