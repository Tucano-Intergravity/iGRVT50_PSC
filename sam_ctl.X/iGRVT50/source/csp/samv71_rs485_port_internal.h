#ifndef SAMV71_RS485_PORT_INTERNAL_H
#define SAMV71_RS485_PORT_INTERNAL_H

#include <stdbool.h>
#include <stdint.h>

#include "csp/samv71_rs485_port.h"

typedef struct {
    uint32_t (*status)(void);
    uint8_t (*read_byte)(void);
    void (*write_byte)(uint8_t value);
    void (*set_direction_tx)(bool transmit);
    void (*set_rx_irq)(bool enabled);
    void (*clear_pending_irq)(void);
    void (*reset_status_and_flush)(void);
    void (*reset_rx)(void);
    void (*reset_tx)(void);
    uint32_t (*now_ms)(void);
    void (*delay_one_bit)(void);
} samv71_rs485_hw_ops_t;

#define SAMV71_RS485_STATUS_RX_READY (UINT32_C(1) << 0)
#define SAMV71_RS485_STATUS_TX_READY (UINT32_C(1) << 1)
#define SAMV71_RS485_STATUS_TX_EMPTY (UINT32_C(1) << 9)

struct samv71_rs485_port_context {
    const samv71_rs485_hw_ops_t *hw;
    bool initialized;
    bool rx_irq_enabled;
};

#ifdef CSP_RS485_HOST_TEST
void samv71_rs485_port_test_bind_hw(const samv71_rs485_hw_ops_t *hw);
#endif

#endif
