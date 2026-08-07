#ifndef SAM_CSP_CONFIG_H
#define SAM_CSP_CONFIG_H

#include <stdatomic.h>
#include <stdint.h>

#include <csp/sam_csp_protocol.h>

#define SAM_CSP_LOCAL_ADDRESS 1U
#define SAM_CSP_PEER_ADDRESS 2U

#define SAM_CSP_CONN_MAX 4U
#define SAM_CSP_CONN_QUEUE_LENGTH 10U
#define SAM_CSP_FIFO_LENGTH 25U
#define SAM_CSP_BUFFER_COUNT 20U
#define SAM_CSP_BUFFER_DATA_SIZE 300U
#define SAM_CSP_MAX_BIND_PORT 12U

#define SAM_CSP_ROUTER_TASK_PRIORITY 4U
#define SAM_CSP_LINK_TASK_PRIORITY 3U
#define SAM_CSP_SERVICE_TASK_PRIORITY 2U

#define SAM_CSP_ROUTER_TASK_STACK_WORDS 512U
#define SAM_CSP_LINK_TASK_STACK_WORDS 512U
#define SAM_CSP_SERVICE_TASK_STACK_WORDS 512U

#define SAM_CSP_TX_MARGIN_MS 5U
#define SAM_CSP_RECOVERY_RETRY_MS 25U

_Static_assert(SAM_CSP_LOCAL_ADDRESS != SAM_CSP_PEER_ADDRESS,
    "CSP local and peer addresses must differ");
_Static_assert(SAM_CSP_DIAGNOSTIC_PORT == 12U,
    "CSP max bind must cover diagnostics");
_Static_assert(SAM_CSP_MAX_BIND_PORT == SAM_CSP_DIAGNOSTIC_PORT,
    "CSP max bind must cover diagnostics");
_Static_assert(ATOMIC_BOOL_LOCK_FREE == 2,
    "CSP ISR atomics must be lock-free");
_Static_assert(ATOMIC_INT_LOCK_FREE == 2,
    "CSP 32-bit atomics must be lock-free");
_Static_assert(ATOMIC_LONG_LOCK_FREE == 2,
    "CSP long atomics must be lock-free");
_Static_assert(sizeof(uint_fast32_t) == 4U,
    "CSP health atomics require 32-bit uint_fast32_t");

#endif
