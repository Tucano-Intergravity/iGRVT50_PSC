#ifndef CSP_RS485_LINK_H
#define CSP_RS485_LINK_H

#include <stdint.h>

#include <csp/csp_interface.h>

#include <csp_rs485_port.h>

typedef enum {
    CSP_RS485_LINK_STOPPED = 0,
    CSP_RS485_LINK_RUNNING,
    CSP_RS485_LINK_RECOVERING,
} csp_rs485_link_state_t;

typedef struct {
    csp_rs485_link_state_t state;
    csp_rs485_fault_t last_error;
    uint32_t uart_errors;
    uint32_t dma_errors;
    uint32_t tx_timeouts;
    uint32_t tx_failures;
    uint32_t protocol_errors;
    uint32_t stream_dropped_bytes;
    uint32_t stream_high_watermark;
    uint32_t stream_discontinuities;
    uint32_t recovery_attempts;
    uint32_t recovery_successes;
    uint32_t recovery_failures;
} csp_rs485_health_t;

typedef struct {
    const csp_rs485_port_ops_t *port_ops;
    void *port_context;
    uint32_t tx_margin_ms;
    uint32_t recovery_retry_ms;
    uint32_t task_priority;
    uint32_t task_stack_words;
} csp_rs485_link_config_t;

/*
 * Returns success once the software lifecycle is established. Hardware may
 * initially be RECOVERING; inspect health for readiness and fault status.
 */
int csp_rs485_link_init(const csp_rs485_link_config_t *config);
void csp_rs485_link_deinit(void);
csp_iface_t *csp_rs485_link_get_interface(void);
void csp_rs485_link_get_health(csp_rs485_health_t *health);

#endif
