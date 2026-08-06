/* Binary v1 codec contract tests.  The test-local RED seam is removed when
 * sam_csp_codec.c is present in the host build. */
#include "support/test.h"

#include <csp/sam_csp_protocol.h>

#include <stdint.h>
#include <string.h>

#if !defined(SAM_CSP_CODEC_HAVE_IMPLEMENTATION)
sam_csp_status_t sam_csp_decode_set_outputs(
    const uint8_t *data,
    size_t length,
    sam_csp_set_outputs_request_t *request,
    uint8_t *detail)
{
    (void) data;
    (void) length;
    (void) request;
    (void) detail;
    return SAM_CSP_STATUS_INTERNAL_ERROR;
}

size_t sam_csp_encode_status(
    uint8_t opcode,
    uint16_t transaction_id,
    sam_csp_status_t status,
    uint8_t detail,
    uint8_t *output,
    size_t capacity)
{
    (void) opcode;
    (void) transaction_id;
    (void) status;
    (void) detail;
    (void) output;
    (void) capacity;
    return 0U;
}

size_t sam_csp_encode_snapshot(
    uint8_t opcode,
    uint16_t transaction_id,
    const sam_csp_snapshot_t *snapshot,
    uint8_t *output,
    size_t capacity)
{
    (void) opcode;
    (void) transaction_id;
    (void) snapshot;
    (void) output;
    (void) capacity;
    return 0U;
}

size_t sam_csp_encode_health(
    uint8_t opcode,
    uint16_t transaction_id,
    const sam_csp_health_t *health,
    uint8_t *output,
    size_t capacity)
{
    (void) opcode;
    (void) transaction_id;
    (void) health;
    (void) output;
    (void) capacity;
    return 0U;
}
#endif

static void decoder_accepts_valid_set_outputs(void)
{
    static const uint8_t packet[] = {
        0x01U, 0x01U, 0x12U, 0x34U, 0x0AU,
        0x55U, 0xA5U, 0x09U, 0x01U, 0x00U,
    };
    sam_csp_set_outputs_request_t request = {0};
    uint8_t detail = 0xA5U;

    TEST_ASSERT_EQ_SIZE(
        SAM_CSP_STATUS_OK,
        sam_csp_decode_set_outputs(packet, sizeof(packet), &request, &detail));
    TEST_ASSERT_EQ_SIZE(0U, detail);
    TEST_ASSERT_EQ_SIZE(0x1234U, request.transaction_id);
    TEST_ASSERT_EQ_SIZE(0x0A55U, request.lpv_on_mask);
    TEST_ASSERT_EQ_SIZE(0xA5U, request.hpv_on_mask);
    TEST_ASSERT_EQ_SIZE(0x09U, request.heater_on_mask);
    TEST_ASSERT_EQ_SIZE(0x01U, request.spark_on);
}

static void status_encoder_matches_exact_six_byte_response(void)
{
    static const uint8_t expected[] = {
        0x01U, 0x01U, 0x12U, 0x34U, 0x00U, 0x00U,
    };
    uint8_t output[SAM_CSP_RESPONSE_HEADER_LENGTH] = {0};

    TEST_ASSERT_EQ_SIZE(
        sizeof(expected),
        sam_csp_encode_status(
            SAM_CSP_OPCODE_SET_OUTPUTS,
            0x1234U,
            SAM_CSP_STATUS_OK,
            0U,
            output,
            sizeof(output)));
    TEST_ASSERT_TRUE(memcmp(expected, output, sizeof(expected)) == 0);
}

static void decoder_rejects_bad_request_vectors_with_exact_details(void)
{
    static const uint8_t bad_version[] = {0x02U, 0x01U, 0x12U, 0x34U};
    static const uint8_t bad_length[] = {
        0x01U, 0x01U, 0x12U, 0x34U, 0x0AU,
        0x55U, 0xA5U, 0x09U, 0x01U,
    };
    static const uint8_t bad_opcode[] = {
        0x01U, 0x7FU, 0x12U, 0x34U, 0x0AU,
        0x55U, 0xA5U, 0x09U, 0x01U, 0x00U,
    };
    static const uint8_t lpv_upper_bits[] = {
        0x01U, 0x01U, 0x12U, 0x34U, 0x10U,
        0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    };
    static const uint8_t heater_upper_bits[] = {
        0x01U, 0x01U, 0x12U, 0x34U, 0x00U,
        0x00U, 0x00U, 0x10U, 0x00U, 0x00U,
    };
    static const uint8_t spark_two[] = {
        0x01U, 0x01U, 0x12U, 0x34U, 0x00U,
        0x00U, 0x00U, 0x00U, 0x02U, 0x00U,
    };
    static const uint8_t reserved_one[] = {
        0x01U, 0x01U, 0x12U, 0x34U, 0x00U,
        0x00U, 0x00U, 0x00U, 0x00U, 0x01U,
    };
    static const uint8_t bad_version_response[] = {
        0x01U, 0x01U, 0x12U, 0x34U, 0x01U, 0x00U,
    };
    static const uint8_t bad_length_response[] = {
        0x01U, 0x01U, 0x12U, 0x34U, 0x02U, 0x0AU,
    };
    static const uint8_t bad_opcode_response[] = {
        0x01U, 0x7FU, 0x12U, 0x34U, 0x03U, 0x7FU,
    };
    static const uint8_t lpv_upper_bits_response[] = {
        0x01U, 0x01U, 0x12U, 0x34U, 0x04U, 0x04U,
    };
    static const uint8_t heater_upper_bits_response[] = {
        0x01U, 0x01U, 0x12U, 0x34U, 0x04U, 0x07U,
    };
    static const uint8_t spark_two_response[] = {
        0x01U, 0x01U, 0x12U, 0x34U, 0x04U, 0x08U,
    };
    static const uint8_t reserved_one_response[] = {
        0x01U, 0x01U, 0x12U, 0x34U, 0x04U, 0x09U,
    };
    const struct {
        const uint8_t *packet;
        size_t length;
        sam_csp_status_t status;
        uint8_t detail;
        const uint8_t *response;
    } cases[] = {
        {bad_version, sizeof(bad_version), SAM_CSP_STATUS_BAD_VERSION, 0U, bad_version_response},
        {bad_length, sizeof(bad_length), SAM_CSP_STATUS_BAD_LENGTH, 0x0AU, bad_length_response},
        {bad_opcode, sizeof(bad_opcode), SAM_CSP_STATUS_BAD_OPCODE, 0x7FU, bad_opcode_response},
        {lpv_upper_bits, sizeof(lpv_upper_bits), SAM_CSP_STATUS_INVALID_ARGUMENT, 0x04U, lpv_upper_bits_response},
        {heater_upper_bits, sizeof(heater_upper_bits), SAM_CSP_STATUS_INVALID_ARGUMENT, 0x07U, heater_upper_bits_response},
        {spark_two, sizeof(spark_two), SAM_CSP_STATUS_INVALID_ARGUMENT, 0x08U, spark_two_response},
        {reserved_one, sizeof(reserved_one), SAM_CSP_STATUS_INVALID_ARGUMENT, 0x09U, reserved_one_response},
    };

    for (size_t index = 0U; index < (sizeof(cases) / sizeof(cases[0])); ++index) {
        sam_csp_set_outputs_request_t request = {
            .transaction_id = 0xFFFFU,
            .lpv_on_mask = 0xFFFFU,
            .hpv_on_mask = 0xFFU,
            .heater_on_mask = 0xFFU,
            .spark_on = 0xFFU,
        };
        uint8_t detail = 0xA5U;
        uint8_t response[SAM_CSP_RESPONSE_HEADER_LENGTH] = {0};

        TEST_ASSERT_EQ_SIZE(
            cases[index].status,
            sam_csp_decode_set_outputs(
                cases[index].packet,
                cases[index].length,
                &request,
                &detail));
        TEST_ASSERT_EQ_SIZE(cases[index].detail, detail);
        TEST_ASSERT_EQ_SIZE(
            sizeof(response),
            sam_csp_encode_status(
                cases[index].packet[1],
                0x1234U,
                cases[index].status,
                cases[index].detail,
                response,
                sizeof(response)));
        TEST_ASSERT_TRUE(memcmp(cases[index].response, response, sizeof(response)) == 0);
    }
}

static void decoder_drops_packets_shorter_than_request_header(void)
{
    static const uint8_t packet[] = {0x01U, 0x01U, 0x12U};

    for (size_t length = 0U; length < SAM_CSP_REQUEST_HEADER_LENGTH; ++length) {
        sam_csp_set_outputs_request_t request = {
            .transaction_id = 0xBEEFU,
            .lpv_on_mask = 0xCAFEU,
            .hpv_on_mask = 0xA5U,
            .heater_on_mask = 0x5AU,
            .spark_on = 0x01U,
        };
        uint8_t detail = 0xA5U;

        TEST_ASSERT_EQ_SIZE(
            SAM_CSP_DECODE_DROP,
            sam_csp_decode_set_outputs(packet, length, &request, &detail));
        TEST_ASSERT_EQ_SIZE(0xBEEFU, request.transaction_id);
        TEST_ASSERT_EQ_SIZE(0xA5U, detail);
    }
}

static void snapshot_encoder_matches_all_bytes_and_signed_big_endian_values(void)
{
    static const uint8_t expected[] = {
        0x01U, 0x01U, 0xBEU, 0xEFU, 0x00U, 0x00U,
        0x12U, 0x34U, 0x56U, 0x78U, 0x02U, 0x03U, 0x1FU, 0xFFU,
        0xFFU, 0xFFU, 0xFFU, 0xFFU, 0x00U, 0x00U, 0x00U, 0x00U,
        0x00U, 0x00U, 0x00U, 0x01U, 0x10U, 0x20U, 0x30U, 0x40U,
        0xFEU, 0xFDU, 0xFCU, 0xFCU, 0x80U, 0x00U, 0x00U, 0x00U,
        0x7FU, 0xFFU, 0xFFU, 0xFFU, 0xF8U, 0xA4U, 0x32U, 0xEBU,
        0x07U, 0x5BU, 0xCDU, 0x15U, 0xFFU, 0xFFU, 0xFFU, 0xFEU,
        0x00U, 0x00U, 0x00U, 0x02U, 0x88U, 0xCAU, 0x6CU, 0x00U,
        0x77U, 0x35U, 0x94U, 0x00U,
    };
    const sam_csp_snapshot_t snapshot = {
        .sample_time_ms = 0x12345678U,
        .current_mode = 2U,
        .requested_mode = 3U,
        .validity_mask = 0x1FFFU,
        .pt_millivolt = {
            -1, 0, 1, 0x10203040, -0x01020304,
            INT32_MIN, INT32_MAX, -123456789, 123456789,
        },
        .tc_microvolt = {-2, 2, -2000000000, 2000000000},
    };
    uint8_t output[SAM_CSP_SNAPSHOT_RESPONSE_LENGTH] = {0};

    TEST_ASSERT_EQ_SIZE(
        sizeof(expected),
        sam_csp_encode_snapshot(
            SAM_CSP_OPCODE_GET_SNAPSHOT,
            0xBEEFU,
            &snapshot,
            output,
            sizeof(output)));
    TEST_ASSERT_TRUE(memcmp(expected, output, sizeof(expected)) == 0);
}

static void snapshot_encoder_does_not_write_when_capacity_is_too_small(void)
{
    const sam_csp_snapshot_t snapshot = {0};
    uint8_t output[SAM_CSP_SNAPSHOT_RESPONSE_LENGTH];
    uint8_t expected[SAM_CSP_SNAPSHOT_RESPONSE_LENGTH];

    memset(output, 0xA5, sizeof(output));
    memset(expected, 0xA5, sizeof(expected));
    TEST_ASSERT_EQ_SIZE(
        0U,
        sam_csp_encode_snapshot(
            SAM_CSP_OPCODE_GET_SNAPSHOT,
            0x1234U,
            &snapshot,
            output,
            sizeof(output) - 1U));
    TEST_ASSERT_TRUE(memcmp(expected, output, sizeof(output)) == 0);
}

static void health_encoder_matches_all_bytes_and_big_endian_counters(void)
{
    static const uint8_t expected[] = {
        0x01U, 0x01U, 0xCAU, 0xFEU, 0x00U, 0x00U,
        0x11U, 0x22U, 0x33U, 0x44U, 0x02U, 0x04U, 0x00U, 0x00U,
        0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x01U,
        0x10U, 0x20U, 0x30U, 0x40U, 0xFFU, 0xFFU, 0xFFU, 0xFFU,
        0x80U, 0x00U, 0x00U, 0x00U, 0x7FU, 0xFFU, 0xFFU, 0xFFU,
        0xDEU, 0xADU, 0xBEU, 0xEFU, 0x01U, 0x02U, 0x03U, 0x04U,
        0xA5U, 0xA5U, 0xA5U, 0xA5U, 0x00U, 0x00U, 0xFFU, 0xFFU,
        0xCAU, 0xFEU, 0xBAU, 0xBEU,
    };
    const sam_csp_health_t health = {
        .uptime_ms = 0x11223344U,
        .link_state = 2U,
        .last_error = 4U,
        .counters = {
            0U, 1U, 0x10203040U, UINT32_MAX, 0x80000000U,
            0x7FFFFFFFU, 0xDEADBEEFU, 0x01020304U, 0xA5A5A5A5U,
            0x0000FFFFU, 0xCAFEBABEU,
        },
    };
    uint8_t output[SAM_CSP_HEALTH_RESPONSE_LENGTH] = {0};

    TEST_ASSERT_EQ_SIZE(
        sizeof(expected),
        sam_csp_encode_health(
            SAM_CSP_OPCODE_GET_HEALTH,
            0xCAFEU,
            &health,
            output,
            sizeof(output)));
    TEST_ASSERT_TRUE(memcmp(expected, output, sizeof(expected)) == 0);
}

static void health_encoder_does_not_write_when_capacity_is_too_small(void)
{
    const sam_csp_health_t health = {0};
    uint8_t output[SAM_CSP_HEALTH_RESPONSE_LENGTH];
    uint8_t expected[SAM_CSP_HEALTH_RESPONSE_LENGTH];

    memset(output, 0xA5, sizeof(output));
    memset(expected, 0xA5, sizeof(expected));
    TEST_ASSERT_EQ_SIZE(
        0U,
        sam_csp_encode_health(
            SAM_CSP_OPCODE_GET_HEALTH,
            0x1234U,
            &health,
            output,
            sizeof(output) - 1U));
    TEST_ASSERT_TRUE(memcmp(expected, output, sizeof(output)) == 0);
}

const test_case_t sam_csp_codec_tests[] = {
    {"sam_csp_codec", "decoder_accepts_valid_set_outputs", decoder_accepts_valid_set_outputs},
    {"sam_csp_codec", "status_encoder_matches_exact_six_byte_response", status_encoder_matches_exact_six_byte_response},
    {"sam_csp_codec", "decoder_rejects_bad_request_vectors_with_exact_details", decoder_rejects_bad_request_vectors_with_exact_details},
    {"sam_csp_codec", "decoder_drops_packets_shorter_than_request_header", decoder_drops_packets_shorter_than_request_header},
    {"sam_csp_codec", "snapshot_encoder_matches_all_bytes_and_signed_big_endian_values", snapshot_encoder_matches_all_bytes_and_signed_big_endian_values},
    {"sam_csp_codec", "snapshot_encoder_does_not_write_when_capacity_is_too_small", snapshot_encoder_does_not_write_when_capacity_is_too_small},
    {"sam_csp_codec", "health_encoder_matches_all_bytes_and_big_endian_counters", health_encoder_matches_all_bytes_and_big_endian_counters},
    {"sam_csp_codec", "health_encoder_does_not_write_when_capacity_is_too_small", health_encoder_does_not_write_when_capacity_is_too_small},
};

const size_t sam_csp_codec_test_count =
    sizeof(sam_csp_codec_tests) / sizeof(sam_csp_codec_tests[0]);
