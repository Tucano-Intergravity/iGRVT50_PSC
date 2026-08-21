#ifndef SAM_CSP_SERVICE_H
#define SAM_CSP_SERVICE_H

#include <stddef.h>
#include <stdint.h>

#include <csp/csp.h>

typedef struct {
    uint32_t malformed_packets;
    uint32_t allocation_failures;
    uint32_t send_failures;
    uint32_t rejected_peers;
    uint32_t dropped_ports;
} sam_csp_service_counters_t;

typedef enum {
    SAM_CSP_DISPATCH_DROP = 0,
    SAM_CSP_DISPATCH_RESPOND,
    SAM_CSP_DISPATCH_STANDARD_SERVICE
} sam_csp_dispatch_action_t;

int SamCspService_Open(void);
void SamCspService_Task(void *argument);
void SamCspService_GetCounters(sam_csp_service_counters_t *counters);
sam_csp_dispatch_action_t SamCspService_Dispatch(
    uint8_t source_address,
    uint8_t destination_address,
    uint8_t destination_port,
    const uint8_t *request,
    size_t request_length,
    uint8_t *response,
    size_t response_capacity,
    size_t *response_length);

#endif /* SAM_CSP_SERVICE_H */
