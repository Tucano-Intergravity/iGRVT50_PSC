/* Imported from C:\PSC\csp-rs485\tests\host (authorized subset). */
/* This import makes no assertion about upstream licensing. */
#include "support/test.h"

#include <csp/arch/csp_semaphore.h>
#include <csp/csp_buffer.h>
#include <csp/csp_crc32.h>
#include <csp/csp_error.h>
#include <csp/csp_rtable.h>
#include <csp/interfaces/csp_if_kiss.h>

#include <csp_rs485_profile.h>

#include "csp_rs485_internal.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    uint8_t bytes[CSP_RS485_TX_FRAME_MAX];
    size_t length;
    bool overflow;
} kiss_collector_t;

typedef struct {
    uint8_t reference[CSP_RS485_TX_FRAME_MAX];
    size_t reference_length;
    uint8_t local[CSP_RS485_TX_FRAME_MAX];
    size_t local_length;
    int reference_result;
    csp_rs485_kiss_encode_result_t local_result;
    bool collector_overflow;
} kiss_comparison_t;

static int collect_kiss_bytes(
    void *driver_data,
    const uint8_t *data,
    size_t length)
{
    kiss_collector_t *collector = driver_data;

    if (length > (sizeof(collector->bytes) - collector->length)) {
        collector->overflow = true;
        return CSP_ERR_NOMEM;
    }

    memcpy(&collector->bytes[collector->length], data, length);
    collector->length += length;
    return CSP_ERR_NONE;
}

static bool build_comparison(
    csp_id_t id,
    const uint8_t *payload,
    size_t payload_length,
    kiss_comparison_t *comparison)
{
    csp_packet_t *reference_packet =
        csp_buffer_get(payload_length + CSP_RS485_CSP_CRC_SIZE);
    csp_packet_t *local_packet =
        csp_buffer_get(payload_length + CSP_RS485_CSP_CRC_SIZE);

    memset(comparison, 0, sizeof(*comparison));

    if ((reference_packet == NULL) || (local_packet == NULL)) {
        csp_buffer_free(reference_packet);
        csp_buffer_free(local_packet);
        return false;
    }

    if (payload_length > 0U) {
        memcpy(reference_packet->data, payload, payload_length);
        memcpy(local_packet->data, payload, payload_length);
    }
    reference_packet->length = (uint16_t) payload_length;
    reference_packet->id = id;
    local_packet->length = (uint16_t) payload_length;
    local_packet->id = id;

    kiss_collector_t collector = {0};
    csp_kiss_interface_data_t interface_data = {
        .tx_func = collect_kiss_bytes,
    };
    csp_iface_t interface = {
        .interface_data = &interface_data,
        .driver_data = &collector,
    };
    csp_route_t route = {
        .iface = &interface,
        .via = CSP_NO_VIA_ADDRESS,
    };

    if (csp_mutex_create(&interface_data.lock) != CSP_MUTEX_OK) {
        csp_buffer_free(reference_packet);
        csp_buffer_free(local_packet);
        return false;
    }

    comparison->reference_result = csp_kiss_tx(&route, reference_packet);
    if (comparison->reference_result != CSP_ERR_NONE) {
        csp_buffer_free(reference_packet);
    }
    (void) csp_mutex_remove(&interface_data.lock);

    if (csp_crc32_append(local_packet, false) != CSP_ERR_NONE) {
        csp_buffer_free(local_packet);
        return false;
    }

    comparison->local_result = csp_rs485_kiss_encode(
        local_packet->id,
        local_packet->data,
        local_packet->length,
        comparison->local,
        sizeof(comparison->local),
        &comparison->local_length);
    csp_buffer_free(local_packet);

    memcpy(
        comparison->reference,
        collector.bytes,
        collector.length);
    comparison->reference_length = collector.length;
    comparison->collector_overflow = collector.overflow;
    return true;
}

static bool matches_reference(
    csp_id_t id,
    const uint8_t *payload,
    size_t payload_length,
    size_t vector_index)
{
    kiss_comparison_t comparison;

    if (!build_comparison(id, payload, payload_length, &comparison)) {
        fprintf(
            stderr,
            "Unable to build KISS comparison for vector %zu\n",
            vector_index);
        return false;
    }

    if ((comparison.reference_result != CSP_ERR_NONE)
        || comparison.collector_overflow
        || (comparison.local_result != CSP_RS485_KISS_ENCODE_OK)
        || (comparison.reference_length != comparison.local_length)
        || (memcmp(
                comparison.reference,
                comparison.local,
                comparison.reference_length)
            != 0)) {
        fprintf(
            stderr,
            "KISS oracle mismatch for vector %zu: reference=%zu local=%zu\n",
            vector_index,
            comparison.reference_length,
            comparison.local_length);
        return false;
    }

    return true;
}

static csp_id_t make_id(uint32_t value)
{
    csp_id_t id = {.ext = value};
    return id;
}

static bool pinned_reference_matches_golden(
    csp_id_t id,
    const uint8_t *payload,
    size_t payload_length,
    const uint8_t *expected,
    size_t expected_length)
{
    kiss_comparison_t comparison;

    if (!build_comparison(id, payload, payload_length, &comparison)) {
        return false;
    }

    return (comparison.reference_result == CSP_ERR_NONE)
        && !comparison.collector_overflow
        && (comparison.reference_length == expected_length)
        && (memcmp(comparison.reference, expected, expected_length) == 0);
}

static void kiss_encoder_matches_python_peer_golden_vectors(void)
{
    static const uint8_t baseline_payload[] = {
        0x43U, 0x53U, 0x50U, 0x52U, 0x00U, 0x00U, 0x00U, 0x01U,
    };
    static const uint8_t escape_payload[] = {
        0x43U, 0x53U, 0x50U, 0x52U, 0xC0U, 0xDBU, 0x00U, 0x01U,
    };
    static const uint8_t request_baseline[] = {
        0xC0U, 0x00U, 0x94U, 0xB2U, 0x90U, 0x00U, 0x43U, 0x53U,
        0x50U, 0x52U, 0x00U, 0x00U, 0x00U, 0x01U, 0xE7U, 0xB7U,
        0x4AU, 0xA6U, 0xC0U,
    };
    static const uint8_t reply_baseline[] = {
        0xC0U, 0x00U, 0x96U, 0xA4U, 0x0AU, 0x00U, 0x43U, 0x53U,
        0x50U, 0x52U, 0x00U, 0x00U, 0x00U, 0x01U, 0xE7U, 0xB7U,
        0x4AU, 0xA6U, 0xC0U,
    };
    static const uint8_t request_escape[] = {
        0xC0U, 0x00U, 0x94U, 0xB2U, 0x90U, 0x00U, 0x43U, 0x53U,
        0x50U, 0x52U, 0xDBU, 0xDCU, 0xDBU, 0xDDU, 0x00U, 0x01U,
        0xF0U, 0x2EU, 0xA8U, 0xD8U, 0xC0U,
    };
    static const uint8_t reply_escape[] = {
        0xC0U, 0x00U, 0x96U, 0xA4U, 0x0AU, 0x00U, 0x43U, 0x53U,
        0x50U, 0x52U, 0xDBU, 0xDCU, 0xDBU, 0xDDU, 0x00U, 0x01U,
        0xF0U, 0x2EU, 0xA8U, 0xD8U, 0xC0U,
    };

    TEST_ASSERT_TRUE(pinned_reference_matches_golden(
        make_id(0x94B29000U),
        baseline_payload,
        sizeof(baseline_payload),
        request_baseline,
        sizeof(request_baseline)));
    TEST_ASSERT_TRUE(pinned_reference_matches_golden(
        make_id(0x96A40A00U),
        baseline_payload,
        sizeof(baseline_payload),
        reply_baseline,
        sizeof(reply_baseline)));
    TEST_ASSERT_TRUE(pinned_reference_matches_golden(
        make_id(0x94B29000U),
        escape_payload,
        sizeof(escape_payload),
        request_escape,
        sizeof(request_escape)));
    TEST_ASSERT_TRUE(pinned_reference_matches_golden(
        make_id(0x96A40A00U),
        escape_payload,
        sizeof(escape_payload),
        reply_escape,
        sizeof(reply_escape)));
}

static void kiss_encoder_matches_empty_payload(void)
{
    TEST_ASSERT_TRUE(matches_reference(make_id(0x12345678U), NULL, 0U, 0U));
}

static void kiss_encoder_matches_single_byte_payload(void)
{
    static const uint8_t payload[] = {0x42U};

    TEST_ASSERT_TRUE(
        matches_reference(make_id(0x01020304U), payload, sizeof(payload), 1U));
}

static void kiss_encoder_matches_fend_escape(void)
{
    static const uint8_t payload[] = {0x11U, 0xC0U, 0x22U};

    TEST_ASSERT_TRUE(
        matches_reference(make_id(0xC00000C0U), payload, sizeof(payload), 2U));
}

static void kiss_encoder_matches_fesc_escape(void)
{
    static const uint8_t payload[] = {0x11U, 0xDBU, 0x22U};

    TEST_ASSERT_TRUE(
        matches_reference(make_id(0xDB0000DBU), payload, sizeof(payload), 3U));
}

static void kiss_encoder_matches_mixed_header_payload_and_crc(void)
{
    static const uint8_t payload[] = {
        0xC0U,
        0xDBU,
        0x00U,
        0x7EU,
        0xDBU,
        0xC0U,
    };

    TEST_ASSERT_TRUE(
        matches_reference(make_id(0xC0DB42DBU), payload, sizeof(payload), 4U));
}

static void kiss_encoder_matches_maximum_packet(void)
{
    uint8_t payload[CSP_RS485_INTERFACE_MTU];

    for (size_t index = 0U; index < sizeof(payload); ++index) {
        payload[index] = (uint8_t) ((index * 37U) + 0x5AU);
    }
    payload[0] = 0xC0U;
    payload[sizeof(payload) - 1U] = 0xDBU;

    TEST_ASSERT_TRUE(
        matches_reference(
            make_id(0xC0DBDBC0U),
            payload,
            sizeof(payload),
            5U));
}

static uint32_t next_random(uint32_t *state)
{
    uint32_t value = *state;

    value ^= value << 13U;
    value ^= value >> 17U;
    value ^= value << 5U;
    *state = value;
    return value;
}

static void kiss_encoder_matches_generated_corpus(void)
{
    uint8_t payload[CSP_RS485_INTERFACE_MTU];
    size_t vector_index = 0U;

    for (size_t length = 0U; length <= sizeof(payload); ++length) {
        for (size_t index = 0U; index < length; ++index) {
            payload[index] =
                (uint8_t) ((length * 29U) + (index * 43U) + 0x31U);
        }

        TEST_ASSERT_TRUE(
            matches_reference(
                make_id(0xA5C000DBU ^ (uint32_t) length),
                payload,
                length,
                vector_index));
        ++vector_index;
    }

    uint32_t random_state = 0xC0DEC0DEU;
    for (size_t mixed = 0U; mixed < 256U; ++mixed) {
        const size_t length =
            (size_t) (next_random(&random_state)
                % (CSP_RS485_INTERFACE_MTU + 1U));
        const csp_id_t id = make_id(next_random(&random_state));

        for (size_t index = 0U; index < length; ++index) {
            payload[index] = (uint8_t) next_random(&random_state);
        }

        TEST_ASSERT_TRUE(
            matches_reference(id, payload, length, vector_index));
        ++vector_index;
    }

    TEST_ASSERT_EQ_SIZE(553U, vector_index);
}

static void kiss_encoder_rejects_small_output_without_partial_success(void)
{
    static const uint8_t packet_data[] = {0x11U, 0xC0U, 0xDBU, 0x22U};
    uint8_t output[CSP_RS485_TX_FRAME_MAX];
    uint8_t expected[CSP_RS485_TX_FRAME_MAX];
    size_t output_length = 0x1234U;
    const size_t required_capacity =
        3U
        + (2U
            * (CSP_RS485_CSP_HEADER_SIZE + sizeof(packet_data)));

    memset(output, 0xA5, sizeof(output));
    memset(expected, 0xA5, sizeof(expected));

    const csp_rs485_kiss_encode_result_t result = csp_rs485_kiss_encode(
        make_id(0x12345678U),
        packet_data,
        sizeof(packet_data),
        output,
        required_capacity - 1U,
        &output_length);

    TEST_ASSERT_EQ_SIZE(CSP_RS485_KISS_ENCODE_OUTPUT_TOO_SMALL, result);
    TEST_ASSERT_EQ_SIZE(0x1234U, output_length);
    TEST_ASSERT_TRUE(memcmp(output, expected, sizeof(output)) == 0);
}

static void kiss_encoder_rejects_oversize_before_write(void)
{
    uint8_t packet_data[CSP_RS485_CSP_BUFFER_DATA_SIZE + 1U] = {0};
    uint8_t output[CSP_RS485_TX_FRAME_MAX];
    uint8_t expected[CSP_RS485_TX_FRAME_MAX];
    size_t output_length = 0x5678U;

    memset(output, 0x5A, sizeof(output));
    memset(expected, 0x5A, sizeof(expected));

    const csp_rs485_kiss_encode_result_t result = csp_rs485_kiss_encode(
        make_id(0x12345678U),
        packet_data,
        sizeof(packet_data),
        output,
        sizeof(output),
        &output_length);

    TEST_ASSERT_EQ_SIZE(CSP_RS485_KISS_ENCODE_INVALID_ARGUMENT, result);
    TEST_ASSERT_EQ_SIZE(0x5678U, output_length);
    TEST_ASSERT_TRUE(memcmp(output, expected, sizeof(output)) == 0);
}

const test_case_t kiss_encoder_tests[] = {
    {
        "test_kiss_encoder",
        "kiss_encoder_matches_python_peer_golden_vectors",
        kiss_encoder_matches_python_peer_golden_vectors,
    },
    {
        "test_kiss_encoder",
        "kiss_encoder_matches_empty_payload",
        kiss_encoder_matches_empty_payload,
    },
    {
        "test_kiss_encoder",
        "kiss_encoder_matches_single_byte_payload",
        kiss_encoder_matches_single_byte_payload,
    },
    {
        "test_kiss_encoder",
        "kiss_encoder_matches_fend_escape",
        kiss_encoder_matches_fend_escape,
    },
    {
        "test_kiss_encoder",
        "kiss_encoder_matches_fesc_escape",
        kiss_encoder_matches_fesc_escape,
    },
    {
        "test_kiss_encoder",
        "kiss_encoder_matches_mixed_header_payload_and_crc",
        kiss_encoder_matches_mixed_header_payload_and_crc,
    },
    {
        "test_kiss_encoder",
        "kiss_encoder_matches_maximum_packet",
        kiss_encoder_matches_maximum_packet,
    },
    {
        "test_kiss_encoder",
        "kiss_encoder_matches_generated_corpus",
        kiss_encoder_matches_generated_corpus,
    },
    {
        "test_kiss_encoder",
        "kiss_encoder_rejects_small_output_without_partial_success",
        kiss_encoder_rejects_small_output_without_partial_success,
    },
    {
        "test_kiss_encoder",
        "kiss_encoder_rejects_oversize_before_write",
        kiss_encoder_rejects_oversize_before_write,
    },
};

const size_t kiss_encoder_test_count =
    sizeof(kiss_encoder_tests) / sizeof(kiss_encoder_tests[0]);
