/* Imported from C:\PSC\csp-rs485\tests\host (authorized subset). */
/* This import makes no assertion about upstream licensing. */
#include "fakes/fake_port.h"
#include "support/test.h"

#include <csp/csp_buffer.h>
#include <csp/csp_crc32.h>
#include <csp/csp_error.h>
#include <csp/csp_interface.h>
#include <csp/interfaces/csp_if_kiss.h>

#include <csp_rs485_link.h>
#include <csp_rs485_profile.h>

#include "csp_rs485_internal.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define RX_CAPTURE_CAPACITY 4U

typedef struct {
    csp_id_t id;
    uint8_t payload[CSP_RS485_INTERFACE_MTU];
    size_t payload_length;
} captured_packet_t;

static captured_packet_t captured_packets[RX_CAPTURE_CAPACITY];
static size_t captured_packet_count;
static size_t qfifo_write_call_count;
static CSP_BASE_TYPE *last_task_woken;
static bool capture_overflow;

void __wrap_csp_qfifo_write(
    csp_packet_t *packet,
    csp_iface_t *interface,
    CSP_BASE_TYPE *task_woken)
{
    (void) interface;

    ++qfifo_write_call_count;
    last_task_woken = task_woken;

    if ((packet == NULL)
        || (captured_packet_count >= RX_CAPTURE_CAPACITY)
        || (packet->length > CSP_RS485_INTERFACE_MTU)) {
        capture_overflow = true;
        csp_buffer_free(packet);
        return;
    }

    captured_packet_t *capture =
        &captured_packets[captured_packet_count];
    capture->id = packet->id;
    capture->payload_length = packet->length;
    if (packet->length > 0U) {
        memcpy(capture->payload, packet->data, packet->length);
    }
    ++captured_packet_count;
    csp_buffer_free(packet);
}

static void reset_capture(void)
{
    memset(captured_packets, 0, sizeof(captured_packets));
    captured_packet_count = 0U;
    qfifo_write_call_count = 0U;
    last_task_woken = NULL;
    capture_overflow = false;
}

static int initialize_rx_link(fake_port_t *fake)
{
    csp_rs485_link_deinit();
    fake_port_init(fake);
    reset_capture();

    const csp_rs485_link_config_t config = {
        .port_ops = fake_port_get_ops(),
        .port_context = fake,
        .tx_margin_ms = 5U,
        .recovery_retry_ms = 25U,
        .task_priority = 3U,
        .task_stack_words = 512U,
    };
    return csp_rs485_link_init(&config);
}

static bool build_frame(
    uint32_t id,
    const uint8_t *payload,
    size_t payload_length,
    uint8_t *frame,
    size_t *frame_length)
{
    csp_packet_t *packet =
        csp_buffer_get(payload_length + CSP_RS485_CSP_CRC_SIZE);
    if (packet == NULL) {
        return false;
    }

    packet->id.ext = id;
    packet->length = (uint16_t) payload_length;
    if (payload_length > 0U) {
        memcpy(packet->data, payload, payload_length);
    }

    if (csp_crc32_append(packet, false) != CSP_ERR_NONE) {
        csp_buffer_free(packet);
        return false;
    }

    const csp_rs485_kiss_encode_result_t encode_result =
        csp_rs485_kiss_encode(
            packet->id,
            packet->data,
            packet->length,
            frame,
            CSP_RS485_TX_FRAME_MAX,
            frame_length);
    csp_buffer_free(packet);
    return encode_result == CSP_RS485_KISS_ENCODE_OK;
}

static bool capture_matches(
    size_t capture_index,
    uint32_t id,
    const uint8_t *payload,
    size_t payload_length)
{
    if ((capture_index >= captured_packet_count)
        || (captured_packets[capture_index].id.ext != id)
        || (captured_packets[capture_index].payload_length
            != payload_length)) {
        return false;
    }

    return (payload_length == 0U)
        || (memcmp(
                captured_packets[capture_index].payload,
                payload,
                payload_length)
            == 0);
}

static void kiss_reset_frees_partial_packet_once(void)
{
    const size_t buffers_before = (size_t) csp_buffer_remaining();
    csp_packet_t *packet = csp_buffer_get(0U);

    TEST_ASSERT_TRUE(packet != NULL);
    TEST_ASSERT_EQ_SIZE(buffers_before - 1U, csp_buffer_remaining());

    csp_kiss_interface_data_t kiss_state = {
        .rx_mode = KISS_MODE_ESCAPED,
        .rx_length = 17U,
        .rx_first = true,
        .rx_packet = packet,
    };

    csp_rs485_kiss_reset(&kiss_state);
    const size_t buffers_after_first = (size_t) csp_buffer_remaining();
    csp_rs485_kiss_reset(&kiss_state);
    const size_t buffers_after_second = (size_t) csp_buffer_remaining();

    TEST_ASSERT_EQ_SIZE(buffers_before, buffers_after_first);
    TEST_ASSERT_EQ_SIZE(buffers_before, buffers_after_second);
    TEST_ASSERT_TRUE(kiss_state.rx_packet == NULL);
    TEST_ASSERT_EQ_SIZE(KISS_MODE_NOT_STARTED, kiss_state.rx_mode);
    TEST_ASSERT_EQ_SIZE(0U, kiss_state.rx_length);
    TEST_ASSERT_TRUE(!kiss_state.rx_first);
}

static void link_rx_accepts_frame_split_at_every_byte(void)
{
    static const uint8_t payload[] = {0x11U, 0xC0U, 0xDBU, 0x22U};
    uint8_t frame[CSP_RS485_TX_FRAME_MAX];
    size_t frame_length = 0U;
    fake_port_t fake;
    const int init_result = initialize_rx_link(&fake);
    const bool frame_result =
        build_frame(
            0x12345678U,
            payload,
            sizeof(payload),
            frame,
            &frame_length);

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_TRUE(frame_result);

    for (size_t index = 0U; index < frame_length; ++index) {
        csp_rs485_link_consume_rx_bytes(&frame[index], 1U);
    }

    const size_t count = captured_packet_count;
    const bool match =
        capture_matches(0U, 0x12345678U, payload, sizeof(payload));
    const bool overflow = capture_overflow;
    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(1U, count);
    TEST_ASSERT_TRUE(match);
    TEST_ASSERT_TRUE(!overflow);
}

static void link_rx_accepts_multiple_frames_in_one_chunk(void)
{
    static const uint8_t first_payload[] = {0x01U, 0x02U};
    static const uint8_t second_payload[] = {0xA0U, 0xDBU, 0xB0U};
    uint8_t first_frame[CSP_RS485_TX_FRAME_MAX];
    uint8_t second_frame[CSP_RS485_TX_FRAME_MAX];
    uint8_t combined[2U * CSP_RS485_TX_FRAME_MAX];
    size_t first_length = 0U;
    size_t second_length = 0U;
    fake_port_t fake;
    const int init_result = initialize_rx_link(&fake);
    const bool first_result =
        build_frame(
            0x01020304U,
            first_payload,
            sizeof(first_payload),
            first_frame,
            &first_length);
    const bool second_result =
        build_frame(
            0x50607080U,
            second_payload,
            sizeof(second_payload),
            second_frame,
            &second_length);

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_TRUE(first_result);
    TEST_ASSERT_TRUE(second_result);

    memcpy(combined, first_frame, first_length);
    memcpy(&combined[first_length], second_frame, second_length);
    csp_rs485_link_consume_rx_bytes(
        combined,
        first_length + second_length);

    const size_t count = captured_packet_count;
    const bool first_match =
        capture_matches(
            0U,
            0x01020304U,
            first_payload,
            sizeof(first_payload));
    const bool second_match =
        capture_matches(
            1U,
            0x50607080U,
            second_payload,
            sizeof(second_payload));
    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(2U, count);
    TEST_ASSERT_TRUE(first_match);
    TEST_ASSERT_TRUE(second_match);
}

static void link_rx_ignores_chunk_boundaries(void)
{
    static const uint8_t payload[] = {
        0x10U,
        0x20U,
        0x30U,
        0xC0U,
        0x40U,
        0xDBU,
    };
    static const size_t chunk_pattern[] = {2U, 1U, 4U, 3U};
    uint8_t frame[CSP_RS485_TX_FRAME_MAX];
    size_t frame_length = 0U;
    fake_port_t fake;
    const int init_result = initialize_rx_link(&fake);
    const bool frame_result =
        build_frame(
            0x89ABCDEFU,
            payload,
            sizeof(payload),
            frame,
            &frame_length);

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_TRUE(frame_result);

    size_t position = 0U;
    size_t pattern_index = 0U;
    while (position < frame_length) {
        size_t chunk =
            chunk_pattern[
                pattern_index
                % (sizeof(chunk_pattern) / sizeof(chunk_pattern[0]))];
        if (chunk > (frame_length - position)) {
            chunk = frame_length - position;
        }

        csp_rs485_link_consume_rx_bytes(&frame[position], chunk);
        position += chunk;
        ++pattern_index;
    }

    const size_t count = captured_packet_count;
    const bool match =
        capture_matches(0U, 0x89ABCDEFU, payload, sizeof(payload));
    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(1U, count);
    TEST_ASSERT_TRUE(match);
}

static void link_rx_discontinuity_frees_partial_packet(void)
{
    static const uint8_t partial_frame[] = {
        0xC0U,
        0x00U,
        0x12U,
        0x34U,
    };
    static const uint8_t post_drop_byte = 0x55U;
    fake_port_t fake;
    const int init_result = initialize_rx_link(&fake);
    const size_t buffers_before = (size_t) csp_buffer_remaining();

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);

    csp_rs485_link_consume_rx_bytes(
        partial_frame,
        sizeof(partial_frame));
    const size_t buffers_with_partial = (size_t) csp_buffer_remaining();
    csp_rs485_link_mark_rx_discontinuity();
    csp_rs485_link_consume_rx_bytes(&post_drop_byte, 1U);
    const size_t buffers_after_reset = (size_t) csp_buffer_remaining();
    csp_rs485_health_t health;
    csp_rs485_link_get_health(&health);
    csp_kiss_interface_data_t *kiss_state =
        csp_rs485_link_get_interface()->interface_data;
    const csp_kiss_mode_t mode = kiss_state->rx_mode;
    const csp_packet_t *rx_packet = kiss_state->rx_packet;

    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(buffers_before - 1U, buffers_with_partial);
    TEST_ASSERT_EQ_SIZE(buffers_before, buffers_after_reset);
    TEST_ASSERT_EQ_SIZE(1U, health.stream_discontinuities);
    TEST_ASSERT_EQ_SIZE(KISS_MODE_NOT_STARTED, mode);
    TEST_ASSERT_TRUE(rx_packet == NULL);
}

static void link_rx_resynchronizes_on_next_fend(void)
{
    static const uint8_t payload[] = {0xA1U, 0xC0U, 0xDBU, 0xB2U};
    uint8_t frame[CSP_RS485_TX_FRAME_MAX];
    size_t frame_length = 0U;
    fake_port_t fake;
    const int init_result = initialize_rx_link(&fake);
    const bool frame_result =
        build_frame(
            0x0A0B0C0DU,
            payload,
            sizeof(payload),
            frame,
            &frame_length);

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_TRUE(frame_result);

    const size_t split = frame_length / 2U;
    csp_rs485_link_consume_rx_bytes(frame, split);
    csp_rs485_link_mark_rx_discontinuity();
    csp_rs485_link_consume_rx_bytes(
        &frame[split],
        frame_length - split);
    csp_rs485_link_consume_rx_bytes(frame, frame_length);

    const size_t count = captured_packet_count;
    const bool match =
        capture_matches(0U, 0x0A0B0C0DU, payload, sizeof(payload));
    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(1U, count);
    TEST_ASSERT_TRUE(match);
}

static void link_rx_counts_each_discontinuity_before_consume(void)
{
    static const uint8_t post_drop_byte = 0x55U;
    fake_port_t fake;
    const int init_result = initialize_rx_link(&fake);

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);

    csp_rs485_link_mark_rx_discontinuity();
    csp_rs485_link_mark_rx_discontinuity();
    csp_rs485_link_consume_rx_bytes(&post_drop_byte, 1U);

    csp_rs485_health_t health;
    csp_rs485_link_get_health(&health);
    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(2U, health.stream_discontinuities);
}

static void link_rx_protocol_error_does_not_trigger_hardware_recovery(void)
{
    uint8_t valid_frame[CSP_RS485_TX_FRAME_MAX];
    uint8_t invalid_frame[CSP_RS485_TX_FRAME_MAX];
    size_t frame_length = 0U;
    fake_port_t fake;
    const int init_result = initialize_rx_link(&fake);
    const bool frame_result =
        build_frame(
            0x10203040U,
            NULL,
            0U,
            valid_frame,
            &frame_length);

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_TRUE(frame_result);
    TEST_ASSERT_TRUE(frame_length >= 3U);

    memcpy(invalid_frame, valid_frame, frame_length);
    invalid_frame[frame_length - 2U] ^= 0x01U;
    csp_rs485_link_consume_rx_bytes(invalid_frame, frame_length);

    csp_rs485_health_t after_error;
    csp_rs485_link_get_health(&after_error);
    const size_t count_after_error = captured_packet_count;

    csp_rs485_link_consume_rx_bytes(valid_frame, frame_length);
    const size_t count_after_valid = captured_packet_count;
    csp_rs485_health_t after_valid;
    csp_rs485_link_get_health(&after_valid);

    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(0U, count_after_error);
    TEST_ASSERT_EQ_SIZE(1U, count_after_valid);
    TEST_ASSERT_EQ_SIZE(1U, after_error.protocol_errors);
    TEST_ASSERT_EQ_SIZE(
        CSP_RS485_LINK_RUNNING,
        after_error.state);
    TEST_ASSERT_EQ_SIZE(CSP_RS485_FAULT_NONE, after_error.last_error);
    TEST_ASSERT_EQ_SIZE(1U, after_valid.protocol_errors);
}

static void link_rx_passes_null_task_woken_in_task_context(void)
{
    uint8_t frame[CSP_RS485_TX_FRAME_MAX];
    size_t frame_length = 0U;
    fake_port_t fake;
    const int init_result = initialize_rx_link(&fake);
    const bool frame_result =
        build_frame(
            0x11223344U,
            NULL,
            0U,
            frame,
            &frame_length);

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);
    TEST_ASSERT_TRUE(frame_result);

    csp_rs485_link_consume_rx_bytes(frame, frame_length);
    const size_t call_count = qfifo_write_call_count;
    CSP_BASE_TYPE *task_woken = last_task_woken;

    csp_rs485_link_deinit();

    TEST_ASSERT_EQ_SIZE(1U, call_count);
    TEST_ASSERT_TRUE(task_woken == NULL);
}

static void link_deinit_frees_partial_rx_packet(void)
{
    static const uint8_t partial_frame[] = {
        0xC0U,
        0x00U,
        0x12U,
    };
    fake_port_t fake;
    const int init_result = initialize_rx_link(&fake);
    const size_t buffers_before = (size_t) csp_buffer_remaining();

    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, init_result);

    csp_rs485_link_consume_rx_bytes(
        partial_frame,
        sizeof(partial_frame));
    const size_t buffers_with_partial = (size_t) csp_buffer_remaining();
    csp_rs485_link_deinit();
    const size_t buffers_after_deinit = (size_t) csp_buffer_remaining();

    TEST_ASSERT_EQ_SIZE(buffers_before - 1U, buffers_with_partial);
    TEST_ASSERT_EQ_SIZE(buffers_before, buffers_after_deinit);
}

const test_case_t link_rx_tests[] = {
    {
        "test_link_rx",
        "kiss_reset_frees_partial_packet_once",
        kiss_reset_frees_partial_packet_once,
    },
    {
        "test_link_rx",
        "link_rx_accepts_frame_split_at_every_byte",
        link_rx_accepts_frame_split_at_every_byte,
    },
    {
        "test_link_rx",
        "link_rx_accepts_multiple_frames_in_one_chunk",
        link_rx_accepts_multiple_frames_in_one_chunk,
    },
    {
        "test_link_rx",
        "link_rx_ignores_chunk_boundaries",
        link_rx_ignores_chunk_boundaries,
    },
    {
        "test_link_rx",
        "link_rx_discontinuity_frees_partial_packet",
        link_rx_discontinuity_frees_partial_packet,
    },
    {
        "test_link_rx",
        "link_rx_resynchronizes_on_next_fend",
        link_rx_resynchronizes_on_next_fend,
    },
    {
        "test_link_rx",
        "link_rx_counts_each_discontinuity_before_consume",
        link_rx_counts_each_discontinuity_before_consume,
    },
    {
        "test_link_rx",
        "link_rx_protocol_error_does_not_trigger_hardware_recovery",
        link_rx_protocol_error_does_not_trigger_hardware_recovery,
    },
    {
        "test_link_rx",
        "link_rx_passes_null_task_woken_in_task_context",
        link_rx_passes_null_task_woken_in_task_context,
    },
    {
        "test_link_rx",
        "link_deinit_frees_partial_rx_packet",
        link_deinit_frees_partial_rx_packet,
    },
};

const size_t link_rx_test_count =
    sizeof(link_rx_tests) / sizeof(link_rx_tests[0]);
