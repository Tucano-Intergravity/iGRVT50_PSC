#ifndef SAM_CSP_DOMAIN_H
#define SAM_CSP_DOMAIN_H

#include <stdint.h>

#include "sam_csp_protocol.h"

typedef enum {
    SAM_CSP_DOMAIN_OK = 0,
    SAM_CSP_DOMAIN_INVALID_STATE,
    SAM_CSP_DOMAIN_APPLY_FAILED,
    SAM_CSP_DOMAIN_SNAPSHOT_FAILED,
} sam_csp_domain_result_t;

sam_csp_domain_result_t sam_csp_domain_apply_outputs(
    const sam_csp_set_outputs_request_t *request);
sam_csp_domain_result_t sam_csp_domain_request_mode(uint8_t mode);
sam_csp_domain_result_t sam_csp_domain_get_snapshot(
    sam_csp_snapshot_t *snapshot);

#endif
