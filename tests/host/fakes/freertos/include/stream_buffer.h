/* Imported from C:\PSC\csp-rs485\tests\host (authorized subset). */
/* This import makes no assertion about upstream licensing. */
#ifndef STREAM_BUFFER_H
#define STREAM_BUFFER_H

#include <FreeRTOS.h>

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t *storage;
    size_t storage_size;
    size_t read_index;
    size_t write_index;
    size_t count;
} StaticStreamBuffer_t;

typedef StaticStreamBuffer_t *StreamBufferHandle_t;

StreamBufferHandle_t xStreamBufferCreateStatic(size_t buffer_size_bytes,
    size_t trigger_level_bytes, uint8_t *storage_area,
    StaticStreamBuffer_t *stream_buffer);
size_t xStreamBufferSendFromISR(StreamBufferHandle_t stream_buffer,
    const void *data, size_t data_length_bytes,
    BaseType_t *higher_priority_task_woken);
size_t xStreamBufferReceive(StreamBufferHandle_t stream_buffer, void *data,
    size_t buffer_length_bytes, TickType_t ticks_to_wait);
size_t xStreamBufferBytesAvailable(StreamBufferHandle_t stream_buffer);

#endif
