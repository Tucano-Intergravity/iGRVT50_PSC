/* Imported from C:\PSC\csp-rs485\tests\host (authorized subset). */
/* This import makes no assertion about upstream licensing. */
#include "fake_port.h"

#include <string.h>

static void record_operation(fake_port_t *fake, const char *name)
{
    csp_rs485_health_t health;

    csp_rs485_link_get_health(&health);
    fake->last_operation_name = name;

    if (fake->operation_count >= FAKE_PORT_OPERATION_CAPACITY) {
        fake->operation_overflow = true;
        return;
    }

    fake_port_operation_t *operation =
        &fake->operations[fake->operation_count];
    operation->name = name;
    operation->link_state = health.state;
    ++fake->operation_count;
}

static csp_rs485_port_result_t fake_initialize(void *context)
{
    fake_port_t *fake = context;

    record_operation(fake, "initialize");
    return fake->initialize_result;
}

static csp_rs485_port_result_t fake_arm_receive(void *context)
{
    fake_port_t *fake = context;

    record_operation(fake, "arm_receive");
    return fake->arm_receive_result;
}

static void fake_enable_irqs(void *context)
{
    record_operation(context, "enable_irqs");
}

static void fake_disable_and_clear_irqs(void *context)
{
    record_operation(context, "disable_and_clear_irqs");
}

static csp_rs485_port_result_t fake_abort_receive(void *context)
{
    fake_port_t *fake = context;

    record_operation(fake, "abort_receive");
    return fake->abort_receive_result;
}

static csp_rs485_port_result_t fake_deinitialize(void *context)
{
    fake_port_t *fake = context;

    record_operation(fake, "deinitialize");
    return fake->deinitialize_result;
}

static void fake_force_receive_mode(void *context)
{
    record_operation(context, "force_receive_mode");
}

static void fake_reset_rx_position(void *context)
{
    record_operation(context, "reset_rx_position");
}

static csp_rs485_port_result_t fake_transmit_frame(
    void *context,
    const uint8_t *frame,
    size_t frame_length,
    uint32_t timeout_ms)
{
    fake_port_t *fake = context;

    ++fake->transmit_call_count;
    if (fake->transmit_call_count > 1U) {
        fake->second_transmit_detected = true;
    }

    record_operation(fake, "transmit_frame");
    fake->frame_length = frame_length;
    fake->timeout_ms = timeout_ms;

    if (frame_length > sizeof(fake->frame)) {
        fake->frame_overflow = true;
    } else {
        memcpy(fake->frame, frame, frame_length);
    }

    return fake->transmit_result;
}

static const csp_rs485_port_ops_t fake_port_ops = {
    .initialize = fake_initialize,
    .arm_receive = fake_arm_receive,
    .enable_irqs = fake_enable_irqs,
    .disable_and_clear_irqs = fake_disable_and_clear_irqs,
    .abort_receive = fake_abort_receive,
    .deinitialize = fake_deinitialize,
    .force_receive_mode = fake_force_receive_mode,
    .reset_rx_position = fake_reset_rx_position,
    .transmit_frame = fake_transmit_frame,
};

void fake_port_init(fake_port_t *fake)
{
    memset(fake, 0, sizeof(*fake));
    fake->initialize_result = CSP_RS485_PORT_OK;
    fake->arm_receive_result = CSP_RS485_PORT_OK;
    fake->abort_receive_result = CSP_RS485_PORT_OK;
    fake->deinitialize_result = CSP_RS485_PORT_OK;
    fake->transmit_result = CSP_RS485_PORT_OK;
}

void fake_port_clear_calls(fake_port_t *fake)
{
    const csp_rs485_port_result_t initialize_result =
        fake->initialize_result;
    const csp_rs485_port_result_t arm_receive_result =
        fake->arm_receive_result;
    const csp_rs485_port_result_t abort_receive_result =
        fake->abort_receive_result;
    const csp_rs485_port_result_t deinitialize_result =
        fake->deinitialize_result;
    const csp_rs485_port_result_t transmit_result = fake->transmit_result;

    memset(fake, 0, sizeof(*fake));
    fake->initialize_result = initialize_result;
    fake->arm_receive_result = arm_receive_result;
    fake->abort_receive_result = abort_receive_result;
    fake->deinitialize_result = deinitialize_result;
    fake->transmit_result = transmit_result;
}

const csp_rs485_port_ops_t *fake_port_get_ops(void)
{
    return &fake_port_ops;
}
