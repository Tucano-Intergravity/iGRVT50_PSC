#ifndef FAKE_SAMV71_HW_H
#define FAKE_SAMV71_HW_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "csp/samv71_rs485_port.h"
#include "csp/samv71_rs485_port_internal.h"

#define FAKE_SAMV71_CAPACITY 128U

typedef struct {
    volatile uint32_t US_CR;
    volatile uint32_t US_MR;
    volatile uint32_t US_IER;
    volatile uint32_t US_IDR;
    volatile uint32_t US_IMR;
    volatile uint32_t US_CSR;
    volatile uint32_t US_RHR;
    volatile uint32_t US_THR;
    volatile uint32_t US_BRGR;
} fake_samv71_usart_registers_t;

extern fake_samv71_usart_registers_t fake_samv71_usart1_registers;
#define USART1_REGS (&fake_samv71_usart1_registers)

#define US_CR_USART_RSTRX_Msk (UINT32_C(1) << 2)
#define US_CR_USART_RSTTX_Msk (UINT32_C(1) << 3)
#define US_CR_USART_RXEN_Msk (UINT32_C(1) << 4)
#define US_CR_USART_RXDIS_Msk (UINT32_C(1) << 5)
#define US_CR_USART_TXEN_Msk (UINT32_C(1) << 6)
#define US_CR_USART_TXDIS_Msk (UINT32_C(1) << 7)
#define US_CR_USART_RSTSTA_Msk (UINT32_C(1) << 8)
#define US_MR_USART_USCLKS_MCK UINT32_C(0)
#define US_MR_USART_CHRL_8_BIT (UINT32_C(3) << 6)
#define US_MR_USART_PAR_NO (UINT32_C(4) << 9)
#define US_MR_USART_NBSTOP_1_BIT UINT32_C(0)
#define US_MR_USART_OVER(value) (((uint32_t) (value) & UINT32_C(1)) << 19)
#define US_BRGR_CD(value) ((uint32_t) (value) & UINT32_C(0xffff))
#define US_BRGR_FP(value) (((uint32_t) (value) & UINT32_C(7)) << 16)

typedef enum {
    FAKE_SAMV71_EVENT_RX_IRQ_OFF = 0,
    FAKE_SAMV71_EVENT_NRE_HIGH,
    FAKE_SAMV71_EVENT_DE_HIGH,
    FAKE_SAMV71_EVENT_GUARD_1BIT,
    FAKE_SAMV71_EVENT_WRITE,
    FAKE_SAMV71_EVENT_WAIT_TXEMPTY,
    FAKE_SAMV71_EVENT_DE_LOW,
    FAKE_SAMV71_EVENT_NRE_LOW,
    FAKE_SAMV71_EVENT_RX_IRQ_ON,
    FAKE_SAMV71_EVENT_CLEAR_PENDING,
    FAKE_SAMV71_EVENT_RESET_STATUS_AND_FLUSH,
    FAKE_SAMV71_EVENT_RESET_RX,
    FAKE_SAMV71_EVENT_RESET_TX,
} fake_samv71_event_kind_t;

typedef struct {
    fake_samv71_event_kind_t kind;
    uint8_t value;
} fake_samv71_event_t;

typedef struct {
    fake_samv71_event_t events[FAKE_SAMV71_CAPACITY];
    size_t event_count;
    bool event_overflow;
    uint32_t status_values[FAKE_SAMV71_CAPACITY];
    size_t status_count;
    size_t status_position;
    uint32_t status_default;
    uint32_t time_values[FAKE_SAMV71_CAPACITY];
    size_t time_count;
    size_t time_position;
    uint8_t rx_bytes[FAKE_SAMV71_CAPACITY];
    size_t rx_count;
    size_t rx_position;
    size_t expected_frame_length;
    size_t write_count;
    size_t guard_count;
    size_t reset_tx_count;
    bool wait_txempty_recorded;
    bool rx_irq_enabled;
    bool de_high;
    bool nre_high;
} fake_samv71_hw_t;

void fake_samv71_hw_reset(void);
fake_samv71_hw_t *fake_samv71_hw_state(void);
const samv71_rs485_hw_ops_t *fake_samv71_hw_ops(void);
void fake_samv71_hw_clear_events(void);
void fake_samv71_hw_set_status_script(
    const uint32_t *values,
    size_t count,
    uint32_t default_status);
void fake_samv71_hw_set_time_script(const uint32_t *values, size_t count);
void fake_samv71_hw_set_rx_bytes(const uint8_t *bytes, size_t count);
void fake_samv71_hw_expect_frame_length(size_t length);

#endif
