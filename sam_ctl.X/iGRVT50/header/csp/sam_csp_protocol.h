#ifndef SAM_CSP_PROTOCOL_H
#define SAM_CSP_PROTOCOL_H

#include <stddef.h>
#include <stdint.h>

#define SAM_CSP_PROTOCOL_VERSION 1U

#define SAM_CSP_COMMAND_PORT 10U
#define SAM_CSP_TELEMETRY_PORT 11U
#define SAM_CSP_DIAGNOSTIC_PORT 12U

#define SAM_CSP_OPCODE_SET_OUTPUTS 1U
#define SAM_CSP_OPCODE_SET_MODE 2U
#define SAM_CSP_OPCODE_GET_SNAPSHOT 1U
#define SAM_CSP_OPCODE_GET_HEALTH 1U

#define SAM_CSP_REQUEST_HEADER_LENGTH 4U
#define SAM_CSP_RESPONSE_HEADER_LENGTH 6U
#define SAM_CSP_SET_OUTPUTS_REQUEST_LENGTH 10U
#define SAM_CSP_SET_OUTPUTS_RESPONSE_LENGTH 6U
#define SAM_CSP_SNAPSHOT_PT_COUNT 9U
#define SAM_CSP_SNAPSHOT_TC_COUNT 4U
#define SAM_CSP_SNAPSHOT_RESPONSE_LENGTH 66U
#define SAM_CSP_HEALTH_COUNTER_COUNT 11U
#define SAM_CSP_HEALTH_RESPONSE_LENGTH 58U

typedef enum {
    SAM_CSP_STATUS_OK = 0,
    SAM_CSP_STATUS_BAD_VERSION = 1,
    SAM_CSP_STATUS_BAD_LENGTH = 2,
    SAM_CSP_STATUS_BAD_OPCODE = 3,
    SAM_CSP_STATUS_INVALID_ARGUMENT = 4,
    SAM_CSP_STATUS_INVALID_STATE = 5,
    SAM_CSP_STATUS_APPLY_FAILED = 6,
    SAM_CSP_STATUS_INTERNAL_ERROR = 7,
    SAM_CSP_STATUS_BUSY = 8,
    SAM_CSP_DECODE_DROP = 9,
} sam_csp_status_t;

typedef struct {
    uint16_t transaction_id;
    uint16_t lpv_on_mask;
    uint8_t hpv_on_mask;
    uint8_t heater_on_mask;
    uint8_t spark_on;
} sam_csp_set_outputs_request_t;

typedef struct {
    uint32_t sample_time_ms;
    uint8_t current_mode;
    uint8_t requested_mode;
    uint16_t validity_mask;
    int32_t pt_millivolt[SAM_CSP_SNAPSHOT_PT_COUNT];
    int32_t tc_microvolt[SAM_CSP_SNAPSHOT_TC_COUNT];
} sam_csp_snapshot_t;

typedef struct {
    uint32_t uptime_ms;
    uint8_t link_state;
    uint8_t last_error;
    uint32_t counters[SAM_CSP_HEALTH_COUNTER_COUNT];
} sam_csp_health_t;

_Static_assert(SAM_CSP_REQUEST_HEADER_LENGTH == 4U,
    "CSP v1 request header must be four bytes");
_Static_assert(SAM_CSP_RESPONSE_HEADER_LENGTH == 6U,
    "CSP v1 response header must be six bytes");
_Static_assert(SAM_CSP_SET_OUTPUTS_REQUEST_LENGTH == 10U,
    "CSP v1 SET_OUTPUTS request must be ten bytes");
_Static_assert(SAM_CSP_SET_OUTPUTS_RESPONSE_LENGTH
        == SAM_CSP_RESPONSE_HEADER_LENGTH,
    "CSP v1 SET_OUTPUTS response must be a status response");
_Static_assert(SAM_CSP_SNAPSHOT_RESPONSE_LENGTH
        == (SAM_CSP_RESPONSE_HEADER_LENGTH + 4U + 1U + 1U + 2U
            + (SAM_CSP_SNAPSHOT_PT_COUNT * 4U)
            + (SAM_CSP_SNAPSHOT_TC_COUNT * 4U)),
    "CSP v1 snapshot response length mismatch");
_Static_assert(SAM_CSP_HEALTH_RESPONSE_LENGTH
        == (SAM_CSP_RESPONSE_HEADER_LENGTH + 4U + 1U + 1U + 2U
            + (SAM_CSP_HEALTH_COUNTER_COUNT * 4U)),
    "CSP v1 health response length mismatch");

sam_csp_status_t sam_csp_decode_set_outputs(
    const uint8_t *data, size_t length,
    sam_csp_set_outputs_request_t *request, uint8_t *detail);
size_t sam_csp_encode_status(
    uint8_t opcode, uint16_t transaction_id,
    sam_csp_status_t status, uint8_t detail,
    uint8_t *output, size_t capacity);
size_t sam_csp_encode_snapshot(
    uint8_t opcode, uint16_t transaction_id,
    const sam_csp_snapshot_t *snapshot,
    uint8_t *output, size_t capacity);
size_t sam_csp_encode_health(
    uint8_t opcode, uint16_t transaction_id,
    const sam_csp_health_t *health,
    uint8_t *output, size_t capacity);

#endif
