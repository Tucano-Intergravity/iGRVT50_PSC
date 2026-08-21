#include "csp/samv71_rs485_port.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "definitions.h"

#include "csp_rs485_internal.h"
#include <csp_rs485_profile.h>

#define SAMV71_RS485_RX_IRQ_MASK \
    (US_IER_USART_RXRDY_Msk | US_IER_USART_FRAME_Msk \
        | US_IER_USART_PARE_Msk | US_IER_USART_OVRE_Msk)
#define SAMV71_RS485_ERROR_MASK \
    (US_CSR_USART_OVRE_Msk | US_CSR_USART_FRAME_Msk | US_CSR_USART_PARE_Msk)
#define SAMV71_RS485_BRGR_CD                 10U
#define SAMV71_RS485_BRGR_FP                 1U

static samv71_rs485_port_context_t s_context;

static void force_receive_mode(void)
{
    UART1_DE_OutputEnable();
    UART1_nRE_OutputEnable();
    UART1_DE_Clear();
    UART1_nRE_Clear();
}

void Samv71Rs485Port_ForceReceiveMode(void)
{
    force_receive_mode();
}

static void flush_rx_and_status(void)
{
    uint32_t dummy = 0U;

    USART1_REGS->US_CR = US_CR_USART_RSTSTA_Msk;
    while ((USART1_REGS->US_CSR & US_CSR_USART_RXRDY_Msk) != 0U) {
        dummy = USART1_REGS->US_RHR & US_RHR_RXCHR_Msk;
    }
    (void)dummy;
}

static void enable_dwt_cycle_counter(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static void delay_one_bit(void)
{
    const uint32_t cycles =
        (CPU_CLOCK_FREQUENCY + CSP_RS485_BAUD_RATE - 1U)
        / CSP_RS485_BAUD_RATE;
    uint32_t start;

    enable_dwt_cycle_counter();
    start = DWT->CYCCNT;
    while ((uint32_t)(DWT->CYCCNT - start) < cycles) {
        __NOP();
    }
}

static uint8_t deadline_expired(TickType_t start, TickType_t timeout_ticks)
{
    return ((TickType_t)(xTaskGetTickCount() - start) >= timeout_ticks)
        ? 1U
        : 0U;
}

static csp_rs485_port_result_t wait_for_status(
    uint32_t mask,
    TickType_t start,
    TickType_t timeout_ticks)
{
    while ((USART1_REGS->US_CSR & mask) == 0U) {
        if (deadline_expired(start, timeout_ticks) != 0U) {
            return CSP_RS485_PORT_TIMEOUT;
        }
    }
    return CSP_RS485_PORT_OK;
}

static csp_rs485_port_result_t port_initialize(void *context)
{
    samv71_rs485_port_context_t *ctx =
        (samv71_rs485_port_context_t *)context;

    if (ctx == NULL) {
        return CSP_RS485_PORT_ERROR;
    }

    ctx->rx_irq_enabled = 0U;
    force_receive_mode();
    USART1_REGS->US_IDR = US_IDR_USART_Msk;
    USART1_REGS->US_CR =
        US_CR_USART_RSTRX_Msk | US_CR_USART_RSTTX_Msk | US_CR_USART_RSTSTA_Msk;
    USART1_REGS->US_MR =
        US_MR_USART_USCLKS_MCK
        | US_MR_USART_CHRL_8_BIT
        | US_MR_USART_PAR_NO
        | US_MR_USART_NBSTOP_1_BIT
        | US_MR_USART_OVER(0);
    USART1_REGS->US_BRGR =
        US_BRGR_CD(SAMV71_RS485_BRGR_CD) | US_BRGR_FP(SAMV71_RS485_BRGR_FP);
    USART1_REGS->US_CR = US_CR_USART_TXEN_Msk | US_CR_USART_RXEN_Msk;
    flush_rx_and_status();
    ctx->initialized = 1U;
    return CSP_RS485_PORT_OK;
}

static csp_rs485_port_result_t port_arm_receive(void *context)
{
    samv71_rs485_port_context_t *ctx =
        (samv71_rs485_port_context_t *)context;

    if ((ctx == NULL) || (ctx->initialized == 0U)) {
        return CSP_RS485_PORT_STATE_ERROR;
    }
    force_receive_mode();
    flush_rx_and_status();
    USART1_REGS->US_CR = US_CR_USART_RXEN_Msk;
    return CSP_RS485_PORT_OK;
}

static void port_enable_irqs(void *context)
{
    samv71_rs485_port_context_t *ctx =
        (samv71_rs485_port_context_t *)context;

    if ((ctx == NULL) || (ctx->initialized == 0U)) {
        return;
    }
    ctx->rx_irq_enabled = 1U;
    NVIC_ClearPendingIRQ(USART1_IRQn);
    NVIC_EnableIRQ(USART1_IRQn);
    USART1_REGS->US_IER = SAMV71_RS485_RX_IRQ_MASK;
}

static void port_disable_and_clear_irqs(void *context)
{
    samv71_rs485_port_context_t *ctx =
        (samv71_rs485_port_context_t *)context;

    if (ctx != NULL) {
        ctx->rx_irq_enabled = 0U;
    }
    USART1_REGS->US_IDR = SAMV71_RS485_RX_IRQ_MASK;
    NVIC_ClearPendingIRQ(USART1_IRQn);
}

static csp_rs485_port_result_t port_abort_receive(void *context)
{
    samv71_rs485_port_context_t *ctx =
        (samv71_rs485_port_context_t *)context;

    if (ctx == NULL) {
        return CSP_RS485_PORT_ERROR;
    }
    USART1_REGS->US_IDR = SAMV71_RS485_RX_IRQ_MASK;
    USART1_REGS->US_CR = US_CR_USART_RSTRX_Msk | US_CR_USART_RSTSTA_Msk;
    flush_rx_and_status();
    force_receive_mode();
    ctx->rx_irq_enabled = 0U;
    return CSP_RS485_PORT_OK;
}

static csp_rs485_port_result_t port_deinitialize(void *context)
{
    samv71_rs485_port_context_t *ctx =
        (samv71_rs485_port_context_t *)context;

    USART1_REGS->US_IDR = US_IDR_USART_Msk;
    USART1_REGS->US_CR =
        US_CR_USART_RXDIS_Msk | US_CR_USART_TXDIS_Msk | US_CR_USART_RSTSTA_Msk;
    force_receive_mode();
    if (ctx != NULL) {
        ctx->initialized = 0U;
        ctx->rx_irq_enabled = 0U;
    }
    return CSP_RS485_PORT_OK;
}

static void port_force_receive_mode(void *context)
{
    (void)context;
    force_receive_mode();
}

static void port_reset_rx_position(void *context)
{
    (void)context;
    flush_rx_and_status();
}

static csp_rs485_port_result_t port_transmit_frame(
    void *context,
    const uint8_t *frame,
    size_t frame_length,
    uint32_t timeout_ms)
{
    samv71_rs485_port_context_t *ctx =
        (samv71_rs485_port_context_t *)context;
    const uint8_t restore_rx_irq =
        ((ctx != NULL) && (ctx->rx_irq_enabled != 0U)) ? 1U : 0U;
    TickType_t timeout_ticks = pdMS_TO_TICKS(timeout_ms);
    const TickType_t start = xTaskGetTickCount();
    size_t i;

    if ((ctx == NULL) || (ctx->initialized == 0U)
        || (frame == NULL) || (frame_length == 0U)) {
        force_receive_mode();
        return CSP_RS485_PORT_STATE_ERROR;
    }
    if (timeout_ticks == 0U) {
        timeout_ticks = 1U;
    }

    USART1_REGS->US_IDR = SAMV71_RS485_RX_IRQ_MASK;
    ctx->rx_irq_enabled = 0U;
    USART1_REGS->US_CR = US_CR_USART_TXEN_Msk;
    UART1_nRE_Set();
    UART1_DE_Set();
    delay_one_bit();

    for (i = 0U; i < frame_length; i++) {
        if (wait_for_status(US_CSR_USART_TXRDY_Msk, start, timeout_ticks)
            != CSP_RS485_PORT_OK) {
            USART1_REGS->US_CR = US_CR_USART_RSTTX_Msk;
            force_receive_mode();
            flush_rx_and_status();
            if (restore_rx_irq != 0U) {
                port_enable_irqs(ctx);
            }
            return CSP_RS485_PORT_TIMEOUT;
        }
        USART1_REGS->US_THR = (uint32_t)frame[i] & US_THR_TXCHR_Msk;
    }

    if (wait_for_status(US_CSR_USART_TXEMPTY_Msk, start, timeout_ticks)
        != CSP_RS485_PORT_OK) {
        USART1_REGS->US_CR = US_CR_USART_RSTTX_Msk;
        force_receive_mode();
        flush_rx_and_status();
        if (restore_rx_irq != 0U) {
            port_enable_irqs(ctx);
        }
        return CSP_RS485_PORT_TIMEOUT;
    }

    force_receive_mode();
    flush_rx_and_status();
    if (restore_rx_irq != 0U) {
        port_enable_irqs(ctx);
    }
    return CSP_RS485_PORT_OK;
}

static const csp_rs485_port_ops_t s_ops = {
    .initialize = port_initialize,
    .arm_receive = port_arm_receive,
    .enable_irqs = port_enable_irqs,
    .disable_and_clear_irqs = port_disable_and_clear_irqs,
    .abort_receive = port_abort_receive,
    .deinitialize = port_deinitialize,
    .force_receive_mode = port_force_receive_mode,
    .reset_rx_position = port_reset_rx_position,
    .transmit_frame = port_transmit_frame,
};

const csp_rs485_port_ops_t *Samv71Rs485Port_GetOps(void)
{
    return &s_ops;
}

samv71_rs485_port_context_t *Samv71Rs485Port_GetContext(void)
{
    return &s_context;
}

bool USART1_UartCommRxReadyHook(void)
{
    uint8_t byte;

    if ((s_context.initialized == 0U) || (s_context.rx_irq_enabled == 0U)) {
        return false;
    }

    while ((USART1_REGS->US_CSR & US_CSR_USART_RXRDY_Msk) != 0U) {
        byte = (uint8_t)(USART1_REGS->US_RHR & US_RHR_RXCHR_Msk);
        csp_rs485_freertos_rx_from_isr(&byte, 1U);
    }
    return true;
}

bool USART1_UartCommErrorHook(uint32_t errorStatus)
{
    if ((s_context.initialized == 0U) || (s_context.rx_irq_enabled == 0U)) {
        return false;
    }
    if ((errorStatus & SAMV71_RS485_ERROR_MASK) == 0U) {
        return true;
    }
    flush_rx_and_status();
    csp_rs485_link_mark_rx_discontinuity_from_isr();
    csp_rs485_link_report_fault_from_isr(CSP_RS485_FAULT_UART);
    return true;
}
