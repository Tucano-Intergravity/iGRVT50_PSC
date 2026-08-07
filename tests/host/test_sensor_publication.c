#include "support/test.h"

#include "fakes/fake_opu_tc_scan.h"
#include "fakes/freertos/fake_freertos.h"
#include "support/test_sensor_under_test.h"

#include <stdint.h>

static UInt32 hook_pt_count;
static UInt32 hook_tc_count;
static UInt16 hook_pt_raw[SENSOR_PT_CHANNEL_COUNT];
static int32_t hook_tc_raw[SENSOR_TC_CHANNEL_COUNT];

static void capture_pt_at_critical_exit(void)
{
    hook_pt_count = test_sensor_peek_pt_scan_count();
    for (UInt8 index = 0U; index < SENSOR_PT_CHANNEL_COUNT; ++index) {
        hook_pt_raw[index] = test_sensor_peek_pt_raw(index);
    }
}

static void capture_tc_at_critical_exit(void)
{
    hook_tc_count = test_sensor_peek_tc_scan_count();
    for (UInt8 index = 0U; index < SENSOR_TC_CHANNEL_COUNT; ++index) {
        hook_tc_raw[index] = test_sensor_peek_tc_raw(index);
    }
}

static void incomplete_sensor_arrays_are_rejected_without_publication(void)
{
    const UInt16 pt_raw[SENSOR_PT_CHANNEL_COUNT] = {
        11U, 22U, 33U, 44U, 55U, 66U, 77U, 88U, 99U,
    };
    const int32_t tc_raw[SENSOR_TC_CHANNEL_COUNT] = {
        111, 222, 333, 444,
    };
    fake_freertos_observations_t freertos;

    test_sensor_reset();
    fake_freertos_reset();

    test_sensor_update_pt_raw_scan(pt_raw, SENSOR_PT_CHANNEL_COUNT - 1U);
    test_sensor_update_tc_raw_scan(tc_raw, SENSOR_TC_CHANNEL_COUNT - 1U);
    fake_freertos_get_observations(&freertos);

    TEST_ASSERT_EQ_SIZE(0U, freertos.task_critical_enter_calls);
    TEST_ASSERT_EQ_SIZE(0U, freertos.task_critical_exit_calls);
    TEST_ASSERT_EQ_SIZE(0U, test_sensor_peek_pt_scan_count());
    TEST_ASSERT_EQ_SIZE(0U, test_sensor_peek_tc_scan_count());
    for (UInt8 index = 0U; index < SENSOR_PT_CHANNEL_COUNT; ++index) {
        TEST_ASSERT_EQ_SIZE(0U, test_sensor_peek_pt_raw(index));
    }
    for (UInt8 index = 0U; index < SENSOR_TC_CHANNEL_COUNT; ++index) {
        TEST_ASSERT_TRUE(test_sensor_peek_tc_raw(index) == 0);
    }
}

static void complete_writers_publish_array_and_counter_before_critical_exit(void)
{
    const UInt16 pt_raw[SENSOR_PT_CHANNEL_COUNT] = {
        101U, 202U, 303U, 404U, 505U, 606U, 707U, 808U, 909U,
    };
    const int32_t tc_raw[SENSOR_TC_CHANNEL_COUNT] = {
        INT32_C(-100001), INT32_C(200002),
        INT32_C(-300003), INT32_C(400004),
    };
    fake_freertos_observations_t freertos;

    test_sensor_reset();
    fake_freertos_reset();
    hook_pt_count = 0U;
    fake_freertos_set_critical_exit_hook(capture_pt_at_critical_exit);

    test_sensor_update_pt_raw_scan(pt_raw, SENSOR_PT_CHANNEL_COUNT);
    fake_freertos_get_observations(&freertos);

    TEST_ASSERT_EQ_SIZE(1U, freertos.task_critical_enter_calls);
    TEST_ASSERT_EQ_SIZE(1U, freertos.task_critical_exit_calls);
    TEST_ASSERT_EQ_SIZE(0U, freertos.task_critical_nesting);
    TEST_ASSERT_EQ_SIZE(1U, freertos.task_critical_max_nesting);
    TEST_ASSERT_EQ_SIZE(1U, freertos.critical_exit_hook_calls);
    TEST_ASSERT_EQ_SIZE(1U, hook_pt_count);
    for (UInt8 index = 0U; index < SENSOR_PT_CHANNEL_COUNT; ++index) {
        TEST_ASSERT_EQ_SIZE(pt_raw[index], hook_pt_raw[index]);
    }

    fake_freertos_clear_observations();
    hook_tc_count = 0U;
    fake_freertos_set_critical_exit_hook(capture_tc_at_critical_exit);

    test_sensor_update_tc_raw_scan(tc_raw, SENSOR_TC_CHANNEL_COUNT);
    fake_freertos_get_observations(&freertos);

    TEST_ASSERT_EQ_SIZE(1U, freertos.task_critical_enter_calls);
    TEST_ASSERT_EQ_SIZE(1U, freertos.task_critical_exit_calls);
    TEST_ASSERT_EQ_SIZE(0U, freertos.task_critical_nesting);
    TEST_ASSERT_EQ_SIZE(1U, freertos.task_critical_max_nesting);
    TEST_ASSERT_EQ_SIZE(1U, freertos.critical_exit_hook_calls);
    TEST_ASSERT_EQ_SIZE(1U, hook_tc_count);
    for (UInt8 index = 0U; index < SENSOR_TC_CHANNEL_COUNT; ++index) {
        TEST_ASSERT_TRUE(tc_raw[index] == hook_tc_raw[index]);
    }
}

static void published_count_getters_use_balanced_critical_sections(void)
{
    const UInt16 pt_raw[SENSOR_PT_CHANNEL_COUNT] = {
        101U, 202U, 303U, 404U, 505U, 606U, 707U, 808U, 909U,
    };
    const int32_t tc_raw[SENSOR_TC_CHANNEL_COUNT] = {
        INT32_C(-1073741824), INT32_C(-536870912),
        INT32_C(536870912), INT32_C(1073741824),
    };
    fake_freertos_observations_t freertos;

    test_sensor_reset();
    test_sensor_update_pt_raw_scan(pt_raw, SENSOR_PT_CHANNEL_COUNT);
    test_sensor_update_tc_raw_scan(tc_raw, SENSOR_TC_CHANNEL_COUNT);

    fake_freertos_reset();
    TEST_ASSERT_EQ_SIZE(1U, test_sensor_get_pt_scan_count());
    fake_freertos_get_observations(&freertos);
    TEST_ASSERT_EQ_SIZE(1U, freertos.task_critical_enter_calls);
    TEST_ASSERT_EQ_SIZE(1U, freertos.task_critical_exit_calls);
    TEST_ASSERT_EQ_SIZE(0U, freertos.task_critical_nesting);
    TEST_ASSERT_EQ_SIZE(1U, freertos.task_critical_max_nesting);

    fake_freertos_reset();
    TEST_ASSERT_EQ_SIZE(1U, test_sensor_get_tc_scan_count());
    fake_freertos_get_observations(&freertos);
    TEST_ASSERT_EQ_SIZE(1U, freertos.task_critical_enter_calls);
    TEST_ASSERT_EQ_SIZE(1U, freertos.task_critical_exit_calls);
    TEST_ASSERT_EQ_SIZE(0U, freertos.task_critical_nesting);
    TEST_ASSERT_EQ_SIZE(1U, freertos.task_critical_max_nesting);
}

static void get_scan_copies_both_families_once_then_converts_after_exit(void)
{
    const UInt16 pt_raw[SENSOR_PT_CHANNEL_COUNT] = {
        101U, 202U, 303U, 404U, 505U, 606U, 707U, 808U, 909U,
    };
    const int32_t tc_raw[SENSOR_TC_CHANNEL_COUNT] = {
        INT32_C(-1073741824), INT32_C(-536870912),
        INT32_C(536870912), INT32_C(1073741824),
    };
    const float expected_tc_millivolt[SENSOR_TC_CHANNEL_COUNT] = {
        -1250.0f, -625.0f, 625.0f, 1250.0f,
    };
    const SInt32 expected_tc_microvolt[SENSOR_TC_CHANNEL_COUNT] = {
        -1250000, -625000, 625000, 1250000,
    };
    fake_freertos_observations_t freertos;
    fake_opu_tc_scan_observations_t ads;
    test_sensor_dependency_observations_t dependencies;
    sSensorScan scan = {0};

    test_sensor_reset();
    test_sensor_update_pt_raw_scan(pt_raw, SENSOR_PT_CHANNEL_COUNT);
    test_sensor_update_tc_raw_scan(tc_raw, SENSOR_TC_CHANNEL_COUNT);
    fake_freertos_reset();
    fake_opu_tc_scan_reset();
    test_sensor_reset_dependency_observations();
    test_sensor_set_tc_bypass(1U);

    test_sensor_get_scan(&scan);
    fake_freertos_get_observations(&freertos);
    fake_opu_tc_scan_get_observations(&ads);
    test_sensor_get_dependency_observations(&dependencies);

    TEST_ASSERT_EQ_SIZE(1U, freertos.task_critical_enter_calls);
    TEST_ASSERT_EQ_SIZE(1U, freertos.task_critical_exit_calls);
    TEST_ASSERT_EQ_SIZE(0U, freertos.task_critical_nesting);
    TEST_ASSERT_EQ_SIZE(1U, freertos.task_critical_max_nesting);
    TEST_ASSERT_EQ_SIZE(SENSOR_PT_CHANNEL_COUNT, dependencies.afec_conversion_calls);
    TEST_ASSERT_EQ_SIZE(1U, dependencies.tc_gain_calls);
    TEST_ASSERT_EQ_SIZE(0U, dependencies.conversion_calls_in_critical);
    TEST_ASSERT_EQ_SIZE(0U, ads.raw_code_calls);
    for (UInt8 index = 0U; index < SENSOR_PT_CHANNEL_COUNT; ++index) {
        TEST_ASSERT_EQ_SIZE(pt_raw[index], scan.pt.rawAdc[index]);
        TEST_ASSERT_TRUE(scan.pt.adcMilliVolt[index] == (SInt32) pt_raw[index]);
    }
    for (UInt8 index = 0U; index < SENSOR_TC_CHANNEL_COUNT; ++index) {
        TEST_ASSERT_TRUE(tc_raw[index] == scan.tc.rawCode[index]);
        TEST_ASSERT_TRUE(
            expected_tc_millivolt[index] == scan.tc.milliVolt[index]);
        TEST_ASSERT_TRUE(
            expected_tc_microvolt[index] == scan.tc.microVolt[index]);
    }
}

const test_case_t sensor_publication_tests[] = {
    {"sensor_publication", "incomplete_sensor_arrays_are_rejected_without_publication", incomplete_sensor_arrays_are_rejected_without_publication},
    {"sensor_publication", "complete_writers_publish_array_and_counter_before_critical_exit", complete_writers_publish_array_and_counter_before_critical_exit},
    {"sensor_publication", "published_count_getters_use_balanced_critical_sections", published_count_getters_use_balanced_critical_sections},
    {"sensor_publication", "get_scan_copies_both_families_once_then_converts_after_exit", get_scan_copies_both_families_once_then_converts_after_exit},
};

const size_t sensor_publication_test_count =
    sizeof(sensor_publication_tests) / sizeof(sensor_publication_tests[0]);
