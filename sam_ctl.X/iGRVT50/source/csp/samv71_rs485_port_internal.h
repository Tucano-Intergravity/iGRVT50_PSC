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

typedef struct {
    void (*enable_trace)(void);
    uint32_t (*read_control)(void);
    void (*write_control)(uint32_t value);
    uint32_t (*read_counter)(void);
    void (*fallback_delay_cycles)(uint32_t cycles);
} samv71_rs485_guard_hw_t;

#define SAMV71_RS485_STATUS_RX_READY (UINT32_C(1) << 0)
#define SAMV71_RS485_STATUS_TX_READY (UINT32_C(1) << 1)
#define SAMV71_RS485_STATUS_TX_EMPTY (UINT32_C(1) << 9)
#define SAMV71_RS485_DWT_CTRL_CYCCNTENA (UINT32_C(1) << 0)
#define SAMV71_RS485_DWT_CTRL_NOCYCCNT (UINT32_C(1) << 25)

void samv71_rs485_delay_one_bit_with_guard_hw(
    const samv71_rs485_guard_hw_t *guard_hw,
    uint32_t system_core_clock);

struct samv71_rs485_port_context {
    const samv71_rs485_hw_ops_t *hw;
    bool initialized;
    bool rx_irq_enabled;
};

#ifdef CSP_RS485_HOST_TEST
void samv71_rs485_port_test_bind_hw(const samv71_rs485_hw_ops_t *hw);
#endif

#endif
