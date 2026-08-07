#include "support/test.h"

#include "fakes/fake_opu_tc_scan.h"

#include <opu_tc_scan.h>

#include <stdint.h>

static void tc_scan_converts_four_channels_then_reads_and_publishes_once(void)
{
    static const fake_opu_tc_event_t expected_events[] = {
        {FAKE_OPU_TC_EVENT_TEMPERATURE, 1U, 0U},
        {FAKE_OPU_TC_EVENT_TEMPERATURE, 1U, 1U},
        {FAKE_OPU_TC_EVENT_TEMPERATURE, 1U, 2U},
        {FAKE_OPU_TC_EVENT_TEMPERATURE, 1U, 3U},
        {FAKE_OPU_TC_EVENT_RAW_CODE, 1U, 0U},
        {FAKE_OPU_TC_EVENT_RAW_CODE, 1U, 1U},
        {FAKE_OPU_TC_EVENT_RAW_CODE, 1U, 2U},
        {FAKE_OPU_TC_EVENT_RAW_CODE, 1U, 3U},
        {FAKE_OPU_TC_EVENT_PUBLISH, 0U, 0U},
    };
    static const float expected_temperatures[] = {
        10.0f, -20.0f, 30.0f, -40.0f,
    };
    static const int32_t expected_raw[] = {
        INT32_C(100000), INT32_C(-200000),
        INT32_C(300000), INT32_C(-400000),
    };
    fake_opu_tc_scan_observations_t observations;
    sTcTemp temperature = {0};

    fake_opu_tc_scan_reset();
    for (uint8_t channel = 0U; channel < SENSOR_TC_CHANNEL_COUNT; ++channel) {
        fake_opu_tc_scan_set_temperature(
            channel,
            expected_temperatures[channel]);
        fake_opu_tc_scan_set_raw_code(channel, expected_raw[channel]);
    }

    OpuTcScan_AcquireAndPublish(&temperature);
    fake_opu_tc_scan_get_observations(&observations);

    TEST_ASSERT_EQ_SIZE(4U, observations.temperature_calls);
    TEST_ASSERT_EQ_SIZE(4U, observations.raw_code_calls);
    TEST_ASSERT_EQ_SIZE(1U, observations.publish_calls);
    TEST_ASSERT_EQ_SIZE(
        sizeof(expected_events) / sizeof(expected_events[0]),
        observations.event_count);
    for (size_t index = 0U;
         index < (sizeof(expected_events) / sizeof(expected_events[0]));
         ++index) {
        TEST_ASSERT_EQ_SIZE(
            expected_events[index].type,
            observations.events[index].type);
        TEST_ASSERT_EQ_SIZE(
            expected_events[index].device,
            observations.events[index].device);
        TEST_ASSERT_EQ_SIZE(
            expected_events[index].channel,
            observations.events[index].channel);
    }
    TEST_ASSERT_TRUE(temperature.fTempCh1 == expected_temperatures[0]);
    TEST_ASSERT_TRUE(temperature.fTempCh2 == expected_temperatures[1]);
    TEST_ASSERT_TRUE(temperature.fTempCh3 == expected_temperatures[2]);
    TEST_ASSERT_TRUE(temperature.fTempCh4 == expected_temperatures[3]);
    TEST_ASSERT_EQ_SIZE(SENSOR_TC_CHANNEL_COUNT, observations.published_count);
    for (size_t index = 0U; index < SENSOR_TC_CHANNEL_COUNT; ++index) {
        TEST_ASSERT_TRUE(observations.published_raw[index] == expected_raw[index]);
    }
}

const test_case_t opu_tc_scan_tests[] = {
    {
        "opu_tc_scan",
        "tc_scan_converts_four_channels_then_reads_and_publishes_once",
        tc_scan_converts_four_channels_then_reads_and_publishes_once,
    },
};

const size_t opu_tc_scan_test_count =
    sizeof(opu_tc_scan_tests) / sizeof(opu_tc_scan_tests[0]);
