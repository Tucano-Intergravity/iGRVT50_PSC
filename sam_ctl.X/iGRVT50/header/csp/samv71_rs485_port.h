#ifndef SAMV71_RS485_PORT_H
#define SAMV71_RS485_PORT_H

#include <stdbool.h>
#include <stdint.h>

#include <csp_rs485_port.h>

typedef struct samv71_rs485_port_context samv71_rs485_port_context_t;

extern samv71_rs485_port_context_t samv71_rs485_port_context;
extern const csp_rs485_port_ops_t samv71_rs485_port_ops;

bool USART1_UartCommRxReadyHook(void);
bool USART1_UartCommErrorHook(uint32_t error_status);

#endif
