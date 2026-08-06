#include "csp_rs485_internal.h"

#include <stddef.h>

static void record_recovery_failure(csp_rs485_link_context_t *context)
{
    ++context->health.recovery_failures;
}

csp_rs485_port_result_t csp_rs485_supervisor_start(
    csp_rs485_link_context_t *context)
{
    if ((context == NULL) || (context->config.port_ops == NULL)) {
        return CSP_RS485_PORT_ERROR;
    }

    const csp_rs485_port_ops_t *port = context->config.port_ops;
    void *const port_context = context->config.port_context;

    const csp_rs485_port_result_t initialize_result =
        port->initialize(port_context);
    if (initialize_result != CSP_RS485_PORT_OK) {
        port->force_receive_mode(port_context);
        context->health.state = CSP_RS485_LINK_RECOVERING;
        return initialize_result;
    }

    const csp_rs485_port_result_t arm_result =
        port->arm_receive(port_context);
    if (arm_result != CSP_RS485_PORT_OK) {
        port->force_receive_mode(port_context);
        context->health.state = CSP_RS485_LINK_RECOVERING;
        return arm_result;
    }

    port->enable_irqs(port_context);
    context->health.state = CSP_RS485_LINK_RUNNING;
    return CSP_RS485_PORT_OK;
}

void csp_rs485_supervisor_stop(csp_rs485_link_context_t *context)
{
    if ((context == NULL)
        || (context->health.state == CSP_RS485_LINK_STOPPED)
        || (context->config.port_ops == NULL)) {
        return;
    }

    const csp_rs485_port_ops_t *port = context->config.port_ops;
    void *const port_context = context->config.port_context;

    /*
     * Publish STOPPED before touching hardware so TX is blocked even if a
     * cleanup operation reports an error.
     */
    context->health.state = CSP_RS485_LINK_STOPPED;
    port->force_receive_mode(port_context);
    port->disable_and_clear_irqs(port_context);
    (void) port->abort_receive(port_context);
    (void) port->deinitialize(port_context);
    port->reset_rx_position(port_context);
}

csp_rs485_recovery_result_t csp_rs485_supervisor_recovery_step(
    csp_rs485_link_context_t *context)
{
    if ((context == NULL)
        || (context->health.state != CSP_RS485_LINK_RECOVERING)
        || (context->config.port_ops == NULL)) {
        return CSP_RS485_RECOVERY_CANCELLED;
    }

    const csp_rs485_port_ops_t *port = context->config.port_ops;
    void *const port_context = context->config.port_context;

    ++context->health.recovery_attempts;

    /*
     * RECOVERING already blocks TX. Cleanup failures remain observable, but
     * later cleanup and reinitialization steps still run.
     */
    port->force_receive_mode(port_context);
    port->disable_and_clear_irqs(port_context);
    if (port->abort_receive(port_context) != CSP_RS485_PORT_OK) {
        record_recovery_failure(context);
    }
    if (port->deinitialize(port_context) != CSP_RS485_PORT_OK) {
        record_recovery_failure(context);
    }
    port->reset_rx_position(port_context);
    csp_rs485_link_mark_rx_discontinuity();

    if (port->initialize(port_context) != CSP_RS485_PORT_OK) {
        record_recovery_failure(context);
        port->force_receive_mode(port_context);
        return CSP_RS485_RECOVERY_RETRY_AFTER_WAIT;
    }

    if (port->arm_receive(port_context) != CSP_RS485_PORT_OK) {
        record_recovery_failure(context);
        port->force_receive_mode(port_context);
        return CSP_RS485_RECOVERY_RETRY_AFTER_WAIT;
    }

    port->enable_irqs(port_context);
    context->health.state = CSP_RS485_LINK_RUNNING;
    ++context->health.recovery_successes;
    return CSP_RS485_RECOVERY_COMPLETE;
}
