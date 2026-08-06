#ifndef CSP_RS485_INTERNAL_H
#define CSP_RS485_INTERNAL_H

#include <stddef.h>
#include <stdint.h>

#include <stdbool.h>

#include <csp/csp_types.h>
#include <csp/interfaces/csp_if_kiss.h>

#include <csp_rs485_link.h>

typedef enum {
    CSP_RS485_KISS_ENCODE_OK = 0,
    CSP_RS485_KISS_ENCODE_INVALID_ARGUMENT,
    CSP_RS485_KISS_ENCODE_OUTPUT_TOO_SMALL,
} csp_rs485_kiss_encode_result_t;

typedef enum {
    CSP_RS485_RECOVERY_COMPLETE = 0,
    CSP_RS485_RECOVERY_RETRY_AFTER_WAIT,
    CSP_RS485_RECOVERY_CANCELLED,
} csp_rs485_recovery_result_t;

typedef enum {
    CSP_RS485_DMA_CURSOR_OK = 0,
    CSP_RS485_DMA_CURSOR_DISCONTINUITY,
    CSP_RS485_DMA_CURSOR_INVALID_POSITION,
} csp_rs485_dma_cursor_status_t;

typedef struct {
    const uint8_t *data;
    size_t length;
} csp_rs485_dma_span_t;

typedef struct {
    csp_rs485_dma_cursor_status_t status;
    size_t next_cursor;
    size_t span_count;
    csp_rs485_dma_span_t spans[2];
} csp_rs485_dma_cursor_result_t;

typedef struct {
    csp_rs485_link_config_t config;
    csp_rs485_health_t health;
} csp_rs485_link_context_t;

#define CSP_RS485_TASK_STACK_CAPACITY_WORDS 512U

/*
 * Encodes a complete KISS data frame into caller-owned storage.
 *
 * packet_data already includes the CSP CRC. The caller must provide enough
 * capacity for the worst-case escaped form. On failure, output and
 * output_length are left unchanged.
 */
csp_rs485_kiss_encode_result_t csp_rs485_kiss_encode(
    csp_id_t id,
    const uint8_t *packet_data,
    size_t packet_data_length,
    uint8_t *output,
    size_t output_capacity,
    size_t *output_length);

/*
 * Converts one serialized DMA callback position into immutable unread spans.
 * dma_buffer references CSP_RS485_DMA_BUFFER_SIZE bytes, previous_cursor is
 * the previous raw HAL position in 0..4096, and callback_position uses the
 * same inclusive range. Preserving 4096 deduplicates consecutive TC and IDLE
 * callbacks at the physical end of the circular buffer. A discontinuity
 * advances next_cursor so the following callback can resume.
 */
csp_rs485_dma_cursor_result_t csp_rs485_dma_cursor_calculate(
    const uint8_t *dma_buffer,
    size_t previous_cursor,
    size_t callback_position);

void csp_rs485_link_report_fault(csp_rs485_fault_t fault);
void csp_rs485_link_report_fault_from_isr(csp_rs485_fault_t fault);
void csp_rs485_link_process_isr_faults(void);
void csp_rs485_kiss_reset(csp_kiss_interface_data_t *kiss_state);
void csp_rs485_link_reset_rx_parser(void);
void csp_rs485_link_mark_rx_discontinuity(void);
void csp_rs485_link_mark_rx_discontinuity_once(void);
void csp_rs485_link_mark_rx_discontinuity_from_isr(void);
void csp_rs485_link_record_rx_stream_from_isr(
    size_t dropped_bytes,
    size_t high_watermark,
    bool discontinuity);
void csp_rs485_link_health_task_lock(void);
void csp_rs485_link_health_task_unlock(void);
void csp_rs485_link_consume_rx_bytes(
    const uint8_t *bytes,
    size_t length);
csp_rs485_recovery_result_t csp_rs485_link_recovery_step(void);
csp_rs485_port_result_t csp_rs485_supervisor_start(
    csp_rs485_link_context_t *context);
void csp_rs485_supervisor_stop(csp_rs485_link_context_t *context);
csp_rs485_recovery_result_t csp_rs485_supervisor_recovery_step(
    csp_rs485_link_context_t *context);

bool csp_rs485_freertos_start(const csp_rs485_link_config_t *config);
void csp_rs485_freertos_activate(void);
void csp_rs485_freertos_stop(void);
void csp_rs485_freertos_quiesce_producer(void);
bool csp_rs485_freertos_take_tx_mutex(void);
void csp_rs485_freertos_give_tx_mutex(void);
void csp_rs485_freertos_rx_from_isr(const uint8_t *bytes, size_t length);
void csp_rs485_freertos_mark_rx_discontinuity_from_isr(void);
void csp_rs485_freertos_notify_fault(void);
void csp_rs485_freertos_notify_fault_from_isr(void);
void csp_rs485_freertos_request_stop(void);
size_t csp_rs485_freertos_run_once(void);
#ifdef CSP_RS485_HOST_TEST
void csp_rs485_freertos_test_set_rx_sequence(size_t sequence);
#endif

#endif
