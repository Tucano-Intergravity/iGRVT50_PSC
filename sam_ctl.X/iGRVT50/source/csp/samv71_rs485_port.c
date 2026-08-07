#include "samv71_rs485_port_internal.h"

#include "csp_rs485_internal.h"

#include <stddef.h>
#include <stdint.h>

#ifdef CSP_RS485_HOST_TEST
#include "fake_samv71_hw.h"
#else
#include "definitions.h"
#include "FreeRTOS.h"
#include "task.h"
#endif

#define SAMV71_RS485_RX_IRQ_MASK                                           \
    (US_IER_USART_RXRDY_Msk | US_IER_USART_FRAME_Msk                      \
     | US_IER_USART_PARE_Msk | US_IER_USART_OVRE_Msk)
#define SAMV71_RS485_DE_MASK (UINT32_C(1) << 22)
#define SAMV71_RS485_NRE_MASK (UINT32_C(1) << 24)
#define SAMV71_RS485_DIRECTION_MASK                                       \
    (SAMV71_RS485_DE_MASK | SAMV71_RS485_NRE_MASK)
#define SAMV71_RS485_BAUD UINT32_C(921600)
#define SAMV71_RS485_DWT_UNLOCK_KEY UINT32_C(0xc5acce55)

#ifndef CSP_RS485_HOST_TEST
#ifndef SystemCoreClock
#define SystemCoreClock CPU_CLOCK_FREQUENCY
#endif

_Static_assert(
    SAMV71_RS485_DWT_CTRL_CYCCNTENA == DWT_CTRL_CYCCNTENA_Msk,
    "DWT cycle-counter enable mask mismatch");
_Static_assert(
    SAMV71_RS485_DWT_CTRL_NOCYCCNT == DWT_CTRL_NOCYCCNT_Msk,
    "DWT unavailable mask mismatch");
#endif

static uint32_t target_status(void)
{
    return USART1_REGS->US_CSR;
}

static uint8_t target_read_byte(void)
{
    return (uint8_t) (USART1_REGS->US_RHR & UINT32_C(0xff));
}

static void target_write_byte(uint8_t value)
{
    USART1_REGS->US_THR = (uint32_t) value;
}

static void target_set_direction_tx(bool transmit)
{
#ifndef CSP_RS485_HOST_TEST
    if (transmit) {
        PIOA_REGS->PIO_SODR = SAMV71_RS485_NRE_MASK;
        PIOA_REGS->PIO_SODR = SAMV71_RS485_DE_MASK;
    } else {
        PIOA_REGS->PIO_CODR = SAMV71_RS485_DE_MASK;
        PIOA_REGS->PIO_CODR = SAMV71_RS485_NRE_MASK;
    }
#else
    (void) transmit;
#endif
}

static void target_set_rx_irq(bool enabled)
{
    if (enabled) {
        USART1_REGS->US_CR = US_CR_USART_RXEN_Msk;
        USART1_REGS->US_IER = SAMV71_RS485_RX_IRQ_MASK;
    } else {
        USART1_REGS->US_IDR = SAMV71_RS485_RX_IRQ_MASK;
    }
}

static void target_clear_pending_irq(void)
{
#ifndef CSP_RS485_HOST_TEST
    NVIC_ClearPendingIRQ(USART1_IRQn);
#endif
}

static void target_reset_status_and_flush(void)
{
    uint32_t discarded = 0U;
    USART1_REGS->US_CR = US_CR_USART_RSTSTA_Msk;
    while ((USART1_REGS->US_CSR & SAMV71_RS485_STATUS_RX_READY) != 0U) {
        discarded = USART1_REGS->US_RHR;
    }
    (void) discarded;
}

static void target_reset_rx(void)
{
    USART1_REGS->US_CR = US_CR_USART_RSTRX_Msk | US_CR_USART_RXEN_Msk;
}

static void target_reset_tx(void)
{
    USART1_REGS->US_CR = US_CR_USART_RSTTX_Msk | US_CR_USART_TXEN_Msk;
}

static uint32_t target_now_ms(void)
{
#ifdef CSP_RS485_HOST_TEST
    return 0U;
#else
    return (uint32_t) xTaskGetTickCount()
        * (uint32_t) portTICK_PERIOD_MS;
#endif
}

void samv71_rs485_delay_one_bit_with_guard_hw(
    const samv71_rs485_guard_hw_t *guard_hw,
    uint32_t system_core_clock)
{
    const uint32_t guard_cycles =
        (system_core_clock / SAMV71_RS485_BAUD)
        + (((system_core_clock % SAMV71_RS485_BAUD) != 0U) ? 1U : 0U);

    guard_hw->enable_trace();
    uint32_t control = guard_hw->read_control();
    if ((control & SAMV71_RS485_DWT_CTRL_NOCYCCNT) != 0U) {
        guard_hw->fallback_delay_cycles(guard_cycles);
        return;
    }

    control |= SAMV71_RS485_DWT_CTRL_CYCCNTENA;
    guard_hw->write_control(control);
    if ((guard_hw->read_control() & SAMV71_RS485_DWT_CTRL_CYCCNTENA) == 0U) {
        guard_hw->fallback_delay_cycles(guard_cycles);
        return;
    }

    const uint32_t start = guard_hw->read_counter();
    uint32_t previous = start;
    for (uint32_t poll = 0U; poll < guard_cycles; ++poll) {
        const uint32_t current = guard_hw->read_counter();
        if ((uint32_t) (current - start) >= guard_cycles) {
            return;
        }
        if (current == previous) {
            break;
        }
        previous = current;
    }

    guard_hw->fallback_delay_cycles(guard_cycles);
}

#ifndef CSP_RS485_HOST_TEST
static void target_guard_enable_trace(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->LAR = SAMV71_RS485_DWT_UNLOCK_KEY;
}

static uint32_t target_guard_read_control(void)
{
    return DWT->CTRL;
}

static void target_guard_write_control(uint32_t value)
{
    DWT->CTRL = value;
}

static uint32_t target_guard_read_counter(void)
{
    return DWT->CYCCNT;
}

static void target_guard_fallback_delay_cycles(uint32_t cycles)
{
    while (cycles > 0U) {
        __NOP();
        --cycles;
    }
}

static const samv71_rs485_guard_hw_t target_guard_hw = {
    .enable_trace = target_guard_enable_trace,
    .read_control = target_guard_read_control,
    .write_control = target_guard_write_control,
    .read_counter = target_guard_read_counter,
    .fallback_delay_cycles = target_guard_fallback_delay_cycles,
};
#endif

static void target_delay_one_bit(void)
{
#ifndef CSP_RS485_HOST_TEST
    samv71_rs485_delay_one_bit_with_guard_hw(
        &target_guard_hw,
        (uint32_t) SystemCoreClock);
#endif
}

static const samv71_rs485_hw_ops_t target_hw_ops = {
    .status = target_status,
    .read_byte = target_read_byte,
    .write_byte = target_write_byte,
    .set_direction_tx = target_set_direction_tx,
    .set_rx_irq = target_set_rx_irq,
    .clear_pending_irq = target_clear_pending_irq,
    .reset_status_and_flush = target_reset_status_and_flush,
    .reset_rx = target_reset_rx,
    .reset_tx = target_reset_tx,
    .now_ms = target_now_ms,
    .delay_one_bit = target_delay_one_bit,
};

samv71_rs485_port_context_t samv71_rs485_port_context = {
    .hw = &target_hw_ops,
    .initialized = false,
    .rx_irq_enabled = false,
};

static bool context_has_complete_hw(
    const samv71_rs485_port_context_t *context)
{
    return (context != NULL) && (context->hw != NULL)
        && (context->hw->status != NULL)
        && (context->hw->read_byte != NULL)
        && (context->hw->write_byte != NULL)
        && (context->hw->set_direction_tx != NULL)
        && (context->hw->set_rx_irq != NULL)
        && (context->hw->clear_pending_irq != NULL)
        && (context->hw->reset_status_and_flush != NULL)
        && (context->hw->reset_rx != NULL)
        && (context->hw->reset_tx != NULL)
        && (context->hw->now_ms != NULL)
        && (context->hw->delay_one_bit != NULL);
}

static void configure_usart(void)
{
#ifndef CSP_RS485_HOST_TEST
    PIOA_REGS->PIO_PER = SAMV71_RS485_DIRECTION_MASK;
    PIOA_REGS->PIO_OER = SAMV71_RS485_DIRECTION_MASK;
    PIOA_REGS->PIO_PUDR = SAMV71_RS485_DIRECTION_MASK;
    PIOA_REGS->PIO_PPDDR = SAMV71_RS485_DIRECTION_MASK;
#endif

    USART1_REGS->US_CR = US_CR_USART_RSTRX_Msk | US_CR_USART_RSTTX_Msk
        | US_CR_USART_RSTSTA_Msk;
    USART1_REGS->US_MR = US_MR_USART_USCLKS_MCK
        | US_MR_USART_CHRL_8_BIT | US_MR_USART_PAR_NO
        | US_MR_USART_NBSTOP_1_BIT | US_MR_USART_OVER(0U);
    USART1_REGS->US_BRGR = US_BRGR_CD(10U) | US_BRGR_FP(1U);
    USART1_REGS->US_CR = US_CR_USART_TXEN_Msk | US_CR_USART_RXEN_Msk;
}

static void force_receive(samv71_rs485_port_context_t *context)
{
    if ((context != NULL) && (context->hw != NULL)
        && (context->hw->set_direction_tx != NULL)) {
        context->hw->set_direction_tx(false);
    }
}

static csp_rs485_port_result_t port_initialize(void *opaque_context)
{
    samv71_rs485_port_context_t *const context = opaque_context;
    if (!context_has_complete_hw(context)) {
        force_receive(context);
        return CSP_RS485_PORT_ERROR;
    }

    context->initialized = false;
    context->rx_irq_enabled = false;
    context->hw->set_rx_irq(false);
    force_receive(context);
    configure_usart();
    context->hw->reset_rx();
    context->hw->reset_tx();
    context->hw->reset_status_and_flush();
    context->hw->clear_pending_irq();
    context->initialized = true;
    return CSP_RS485_PORT_OK;
}

static csp_rs485_port_result_t port_arm_receive(void *opaque_context)
{
    samv71_rs485_port_context_t *const context = opaque_context;
    if (!context_has_complete_hw(context) || !context->initialized) {
        force_receive(context);
        return CSP_RS485_PORT_STATE_ERROR;
    }

    context->hw->set_rx_irq(false);
    context->rx_irq_enabled = false;
    context->hw->reset_status_and_flush();
    context->hw->reset_rx();
    context->hw->clear_pending_irq();
    return CSP_RS485_PORT_OK;
}

static void port_enable_irqs(void *opaque_context)
{
    samv71_rs485_port_context_t *const context = opaque_context;
    if (context_has_complete_hw(context) && context->initialized) {
        context->hw->set_rx_irq(true);
        context->rx_irq_enabled = true;
    }
}

static void port_disable_and_clear_irqs(void *opaque_context)
{
    samv71_rs485_port_context_t *const context = opaque_context;
    if (context_has_complete_hw(context)) {
        context->hw->set_rx_irq(false);
        context->rx_irq_enabled = false;
        context->hw->clear_pending_irq();
    }
}

static csp_rs485_port_result_t port_abort_receive(void *opaque_context)
{
    samv71_rs485_port_context_t *const context = opaque_context;
    if (!context_has_complete_hw(context)) {
        return CSP_RS485_PORT_ERROR;
    }

    context->hw->set_rx_irq(false);
    context->rx_irq_enabled = false;
    context->hw->reset_rx();
    context->hw->reset_status_and_flush();
    context->hw->clear_pending_irq();
    force_receive(context);
    return CSP_RS485_PORT_OK;
}

static csp_rs485_port_result_t port_deinitialize(void *opaque_context)
{
    samv71_rs485_port_context_t *const context = opaque_context;
    if (!context_has_complete_hw(context)) {
        return CSP_RS485_PORT_ERROR;
    }

    context->hw->set_rx_irq(false);
    context->rx_irq_enabled = false;
    context->hw->clear_pending_irq();
    context->hw->reset_rx();
    context->hw->reset_tx();
    USART1_REGS->US_CR = US_CR_USART_RXDIS_Msk | US_CR_USART_TXDIS_Msk
        | US_CR_USART_RSTRX_Msk | US_CR_USART_RSTTX_Msk
        | US_CR_USART_RSTSTA_Msk;
    force_receive(context);
    context->initialized = false;
    return CSP_RS485_PORT_OK;
}

static void port_force_receive_mode(void *opaque_context)
{
    force_receive(opaque_context);
}

static void port_reset_rx_position(void *opaque_context)
{
    (void) opaque_context;
}

static bool timeout_expired(
    const samv71_rs485_port_context_t *context,
    uint32_t start,
    uint32_t timeout_ms)
{
    return (uint32_t) (context->hw->now_ms() - start) >= timeout_ms;
}

static csp_rs485_port_result_t port_transmit_frame(
    void *opaque_context,
    const uint8_t *frame,
    size_t frame_length,
    uint32_t timeout_ms)
{
    samv71_rs485_port_context_t *const context = opaque_context;
    if (!context_has_complete_hw(context)) {
        force_receive(context);
        return CSP_RS485_PORT_ERROR;
    }
    if (!context->initialized) {
        force_receive(context);
        return CSP_RS485_PORT_STATE_ERROR;
    }
    if ((frame == NULL) || (frame_length == 0U)) {
        force_receive(context);
        return CSP_RS485_PORT_ERROR;
    }

    const bool restore_rx_irq = context->rx_irq_enabled;
    const uint32_t start = context->hw->now_ms();
    csp_rs485_port_result_t result = CSP_RS485_PORT_OK;

    context->hw->set_rx_irq(false);
    context->rx_irq_enabled = false;
    context->hw->set_direction_tx(true);
    context->hw->delay_one_bit();

    for (size_t index = 0U; index < frame_length; ++index) {
        while ((context->hw->status() & SAMV71_RS485_STATUS_TX_READY) == 0U) {
            if (timeout_expired(context, start, timeout_ms)) {
                result = CSP_RS485_PORT_TIMEOUT;
                goto cleanup;
            }
        }
        context->hw->write_byte(frame[index]);
    }

    while ((context->hw->status() & SAMV71_RS485_STATUS_TX_EMPTY) == 0U) {
        if (timeout_expired(context, start, timeout_ms)) {
            result = CSP_RS485_PORT_TIMEOUT;
            goto cleanup;
        }
    }

cleanup:
    if (result == CSP_RS485_PORT_TIMEOUT) {
        context->hw->reset_tx();
    }
    force_receive(context);
    if (restore_rx_irq) {
        context->hw->set_rx_irq(true);
        context->rx_irq_enabled = true;
    }
    return result;
}

const csp_rs485_port_ops_t samv71_rs485_port_ops = {
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

bool USART1_UartCommRxReadyHook(void)
{
    samv71_rs485_port_context_t *const context = &samv71_rs485_port_context;
    if (!context->initialized || !context->rx_irq_enabled) {
        return false;
    }

    while ((context->hw->status() & SAMV71_RS485_STATUS_RX_READY) != 0U) {
        const uint8_t byte = context->hw->read_byte();
        csp_rs485_freertos_rx_from_isr(&byte, 1U);
    }
    return true;
}

bool USART1_UartCommErrorHook(uint32_t error_status)
{
    samv71_rs485_port_context_t *const context = &samv71_rs485_port_context;
    if (!context->initialized || !context->rx_irq_enabled) {
        return false;
    }

    (void) error_status;
    context->hw->reset_status_and_flush();
    csp_rs485_freertos_mark_rx_discontinuity_from_isr();
    csp_rs485_link_report_fault_from_isr(CSP_RS485_FAULT_UART);
    return true;
}

#ifdef CSP_RS485_HOST_TEST
void samv71_rs485_port_test_bind_hw(const samv71_rs485_hw_ops_t *hw)
{
    samv71_rs485_port_context.hw = hw;
    samv71_rs485_port_context.initialized = false;
    samv71_rs485_port_context.rx_irq_enabled = false;
}
#endif
