/* Imported from C:\PSC\csp-rs485\tests\host (authorized subset). */
/* This import makes no assertion about upstream licensing. */
#include "support/test.h"

#include <csp/csp.h>
#include <csp/csp_buffer.h>
#include <csp/csp_crc32.h>

#include <csp_rs485_profile.h>

#include <stdint.h>
#include <string.h>

static void host_profile_uses_pinned_buffer_capacity(void)
{
    const csp_conf_t *config = csp_get_conf();

    TEST_ASSERT_EQ_SIZE(10U, csp_get_address());
    TEST_ASSERT_EQ_SIZE(20U, config->buffers);
    TEST_ASSERT_EQ_SIZE(300U, csp_buffer_data_size());
    TEST_ASSERT_EQ_SIZE(20U, csp_buffer_remaining());
}

static void host_profile_enables_required_crc_behavior(void)
{
    static const uint8_t payload[] = {0x01U, 0x23U, 0x45U};
    csp_packet_t *packet = csp_buffer_get(sizeof(payload) + sizeof(uint32_t));

    TEST_ASSERT_TRUE(packet != NULL);
    memcpy(packet->data, payload, sizeof(payload));
    packet->length = sizeof(payload);

    TEST_ASSERT_EQ_SIZE(1U, CSP_USE_CRC32);
    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, csp_crc32_append(packet, false));
    TEST_ASSERT_EQ_SIZE(sizeof(payload) + sizeof(uint32_t), packet->length);
    TEST_ASSERT_EQ_SIZE(CSP_ERR_NONE, csp_crc32_verify(packet, false));
    TEST_ASSERT_EQ_SIZE(sizeof(payload), packet->length);

    csp_buffer_free(packet);
}

static void profile_sizes_cover_worst_case_kiss_expansion(void)
{
    TEST_ASSERT_EQ_SIZE(304U, CSP_RS485_TX_RAW_MAX);
    TEST_ASSERT_EQ_SIZE(611U, CSP_RS485_TX_FRAME_MAX);
    TEST_ASSERT_EQ_SIZE(
        CSP_RS485_CSP_HEADER_SIZE + CSP_RS485_CSP_BUFFER_DATA_SIZE,
        CSP_RS485_TX_RAW_MAX);
    TEST_ASSERT_EQ_SIZE(
        3U + (2U * CSP_RS485_TX_RAW_MAX),
        CSP_RS485_TX_FRAME_MAX);
}

static void profile_mtu_reserves_crc_space(void)
{
    TEST_ASSERT_EQ_SIZE(296U, CSP_RS485_INTERFACE_MTU);
    TEST_ASSERT_EQ_SIZE(
        CSP_RS485_CSP_BUFFER_DATA_SIZE - CSP_RS485_CSP_CRC_SIZE,
        CSP_RS485_INTERFACE_MTU);
}

const test_case_t host_profile_tests[] = {
    {
        "test_host_profile",
        "host_profile_uses_pinned_buffer_capacity",
        host_profile_uses_pinned_buffer_capacity,
    },
    {
        "test_host_profile",
        "host_profile_enables_required_crc_behavior",
        host_profile_enables_required_crc_behavior,
    },
    {
        "test_host_profile",
        "profile_sizes_cover_worst_case_kiss_expansion",
        profile_sizes_cover_worst_case_kiss_expansion,
    },
    {
        "test_host_profile",
        "profile_mtu_reserves_crc_space",
        profile_mtu_reserves_crc_space,
    },
};

const size_t host_profile_test_count =
    sizeof(host_profile_tests) / sizeof(host_profile_tests[0]);
