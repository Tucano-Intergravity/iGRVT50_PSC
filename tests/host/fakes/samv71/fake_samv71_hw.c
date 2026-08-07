#include "fake_samv71_hw.h"

#include <string.h>

static fake_samv71_hw_t fake_hw;
fake_samv71_usart_registers_t fake_samv71_usart1_registers;

static void record_event(fake_samv71_event_kind_t kind, uint8_t value)
{
    if (fake_hw.event_count >= FAKE_SAMV71_CAPACITY) {
        fake_hw.event_overflow = true;
        return;
    }

    fake_hw.events[fake_hw.event_count].kind = kind;
    fake_hw.events[fake_hw.event_count].value = value;
    ++fake_hw.event_count;
}

static uint32_t fake_status(void)
{
    uint32_t status = fake_hw.status_default;
    if (fake_hw.status_position < fake_hw.status_count) {
        status = fake_hw.status_values[fake_hw.status_position];
        ++fake_hw.status_position;
    } else if (fake_hw.rx_position < fake_hw.rx_count) {
        status |= SAMV71_RS485_STATUS_RX_READY;
    }

    if (!fake_hw.wait_txempty_recorded
        && (fake_hw.expected_frame_length > 0U)
        && (fake_hw.write_count == fake_hw.expected_frame_length)) {
        record_event(FAKE_SAMV71_EVENT_WAIT_TXEMPTY, 0U);
        fake_hw.wait_txempty_recorded = true;
    }

    return status;
}

static uint8_t fake_read_byte(void)
{
    if (fake_hw.rx_position >= fake_hw.rx_count) {
        return 0U;
    }
    const uint8_t value = fake_hw.rx_bytes[fake_hw.rx_position];
    ++fake_hw.rx_position;
    return value;
}

static void fake_write_byte(uint8_t value)
{
    record_event(FAKE_SAMV71_EVENT_WRITE, value);
    ++fake_hw.write_count;
}

static void fake_set_direction_tx(bool transmit)
{
    if (transmit) {
        fake_hw.nre_high = true;
        record_event(FAKE_SAMV71_EVENT_NRE_HIGH, 0U);
        fake_hw.de_high = true;
        record_event(FAKE_SAMV71_EVENT_DE_HIGH, 0U);
    } else {
        fake_hw.de_high = false;
        record_event(FAKE_SAMV71_EVENT_DE_LOW, 0U);
        fake_hw.nre_high = false;
        record_event(FAKE_SAMV71_EVENT_NRE_LOW, 0U);
    }
}

static void fake_set_rx_irq(bool enabled)
{
    fake_hw.rx_irq_enabled = enabled;
    record_event(
        enabled ? FAKE_SAMV71_EVENT_RX_IRQ_ON
                : FAKE_SAMV71_EVENT_RX_IRQ_OFF,
        0U);
}

static void fake_clear_pending_irq(void)
{
    record_event(FAKE_SAMV71_EVENT_CLEAR_PENDING, 0U);
}

static void fake_reset_status_and_flush(void)
{
    fake_hw.rx_position = fake_hw.rx_count;
    record_event(FAKE_SAMV71_EVENT_RESET_STATUS_AND_FLUSH, 0U);
}

static void fake_reset_rx(void)
{
    record_event(FAKE_SAMV71_EVENT_RESET_RX, 0U);
}

static void fake_reset_tx(void)
{
    ++fake_hw.reset_tx_count;
    record_event(FAKE_SAMV71_EVENT_RESET_TX, 0U);
}

static uint32_t fake_now_ms(void)
{
    if (fake_hw.time_count == 0U) {
        return 0U;
    }
    if (fake_hw.time_position < fake_hw.time_count) {
        const uint32_t value = fake_hw.time_values[fake_hw.time_position];
        ++fake_hw.time_position;
        return value;
    }
    return fake_hw.time_values[fake_hw.time_count - 1U];
}

static void fake_delay_one_bit(void)
{
    ++fake_hw.guard_count;
    record_event(FAKE_SAMV71_EVENT_GUARD_1BIT, 0U);
}

static const samv71_rs485_hw_ops_t fake_ops = {
    .status = fake_status,
    .read_byte = fake_read_byte,
    .write_byte = fake_write_byte,
    .set_direction_tx = fake_set_direction_tx,
    .set_rx_irq = fake_set_rx_irq,
    .clear_pending_irq = fake_clear_pending_irq,
    .reset_status_and_flush = fake_reset_status_and_flush,
    .reset_rx = fake_reset_rx,
    .reset_tx = fake_reset_tx,
    .now_ms = fake_now_ms,
    .delay_one_bit = fake_delay_one_bit,
};

void fake_samv71_hw_reset(void)
{
    memset(&fake_hw, 0, sizeof(fake_hw));
    memset(&fake_samv71_usart1_registers, 0, sizeof(fake_samv71_usart1_registers));
    samv71_rs485_port_test_bind_hw(&fake_ops);
}

fake_samv71_hw_t *fake_samv71_hw_state(void)
{
    return &fake_hw;
}

const samv71_rs485_hw_ops_t *fake_samv71_hw_ops(void)
{
    return &fake_ops;
}

void fake_samv71_hw_clear_events(void)
{
    fake_hw.event_count = 0U;
    fake_hw.event_overflow = false;
    fake_hw.write_count = 0U;
    fake_hw.guard_count = 0U;
    fake_hw.reset_tx_count = 0U;
    fake_hw.wait_txempty_recorded = false;
}

void fake_samv71_hw_set_status_script(
    const uint32_t *values,
    size_t count,
    uint32_t default_status)
{
    if (count > FAKE_SAMV71_CAPACITY) {
        count = FAKE_SAMV71_CAPACITY;
        fake_hw.event_overflow = true;
    }
    fake_hw.status_count = count;
    fake_hw.status_position = 0U;
    fake_hw.status_default = default_status;
    if ((values != NULL) && (count > 0U)) {
        memcpy(fake_hw.status_values, values, count * sizeof(values[0]));
    }
}

void fake_samv71_hw_set_time_script(const uint32_t *values, size_t count)
{
    if (count > FAKE_SAMV71_CAPACITY) {
        count = FAKE_SAMV71_CAPACITY;
        fake_hw.event_overflow = true;
    }
    fake_hw.time_count = count;
    fake_hw.time_position = 0U;
    if ((values != NULL) && (count > 0U)) {
        memcpy(fake_hw.time_values, values, count * sizeof(values[0]));
    }
}

void fake_samv71_hw_set_rx_bytes(const uint8_t *bytes, size_t count)
{
    if (count > FAKE_SAMV71_CAPACITY) {
        count = FAKE_SAMV71_CAPACITY;
        fake_hw.event_overflow = true;
    }
    fake_hw.rx_count = count;
    fake_hw.rx_position = 0U;
    if ((bytes != NULL) && (count > 0U)) {
        memcpy(fake_hw.rx_bytes, bytes, count);
    }
}

void fake_samv71_hw_expect_frame_length(size_t length)
{
    fake_hw.expected_frame_length = length;
    fake_hw.wait_txempty_recorded = false;
}
