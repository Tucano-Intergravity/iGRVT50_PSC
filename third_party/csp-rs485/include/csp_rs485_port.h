#ifndef CSP_RS485_PORT_H
#define CSP_RS485_PORT_H

#include <stddef.h>
#include <stdint.h>

typedef enum {
    CSP_RS485_PORT_OK = 0,
    CSP_RS485_PORT_ERROR,
    CSP_RS485_PORT_TIMEOUT,
    CSP_RS485_PORT_STATE_ERROR,
} csp_rs485_port_result_t;

typedef enum {
    CSP_RS485_FAULT_NONE = 0,
    CSP_RS485_FAULT_UART,
    CSP_RS485_FAULT_DMA,
    CSP_RS485_FAULT_TX_TIMEOUT,
    CSP_RS485_FAULT_TX_STATE,
} csp_rs485_fault_t;

typedef struct {
    csp_rs485_port_result_t (*initialize)(void *context);
    csp_rs485_port_result_t (*arm_receive)(void *context);
    void (*enable_irqs)(void *context);
    void (*disable_and_clear_irqs)(void *context);
    csp_rs485_port_result_t (*abort_receive)(void *context);
    csp_rs485_port_result_t (*deinitialize)(void *context);
    void (*force_receive_mode)(void *context);
    void (*reset_rx_position)(void *context);
    csp_rs485_port_result_t (*transmit_frame)(
        void *context,
        const uint8_t *frame,
        size_t frame_length,
        uint32_t timeout_ms);
} csp_rs485_port_ops_t;

#endif
