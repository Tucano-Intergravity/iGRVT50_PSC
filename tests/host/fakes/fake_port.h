/* Imported from C:\PSC\csp-rs485\tests\host (authorized subset). */
/* This import makes no assertion about upstream licensing. */
#ifndef CSP_RS485_FAKE_PORT_H
#define CSP_RS485_FAKE_PORT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <csp_rs485_link.h>
#include <csp_rs485_port.h>
#include <csp_rs485_profile.h>

#define FAKE_PORT_OPERATION_CAPACITY 32U

typedef struct {
    const char *name;
    csp_rs485_link_state_t link_state;
} fake_port_operation_t;

typedef struct {
    csp_rs485_port_result_t initialize_result;
    csp_rs485_port_result_t arm_receive_result;
    csp_rs485_port_result_t abort_receive_result;
    csp_rs485_port_result_t deinitialize_result;
    csp_rs485_port_result_t transmit_result;
    const char *last_operation_name;
    fake_port_operation_t operations[FAKE_PORT_OPERATION_CAPACITY];
    size_t operation_count;
    uint8_t frame[CSP_RS485_TX_FRAME_MAX];
    size_t frame_length;
    uint32_t timeout_ms;
    size_t transmit_call_count;
    bool operation_overflow;
    bool frame_overflow;
    bool second_transmit_detected;
} fake_port_t;

void fake_port_init(fake_port_t *fake);
void fake_port_clear_calls(fake_port_t *fake);
const csp_rs485_port_ops_t *fake_port_get_ops(void);

#endif
