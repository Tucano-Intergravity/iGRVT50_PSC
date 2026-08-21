#ifndef SAMV71_RS485_PORT_H
#define SAMV71_RS485_PORT_H

#include <stdint.h>

#include <csp_rs485_port.h>

typedef struct {
    volatile uint8_t initialized;
    volatile uint8_t rx_irq_enabled;
} samv71_rs485_port_context_t;

const csp_rs485_port_ops_t *Samv71Rs485Port_GetOps(void);
samv71_rs485_port_context_t *Samv71Rs485Port_GetContext(void);
void Samv71Rs485Port_ForceReceiveMode(void);

#endif /* SAMV71_RS485_PORT_H */
