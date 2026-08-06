/* Domain-boundary contract tests. The test-local RED seam is removed when
 * sam_csp_domain.c is present in the host build. */
#include "support/test.h"

#include "fakes/domain/fake_sam_csp_domain.h"

#include <csp/sam_csp_domain.h>

#include <FreeRTOS.h>

#include <stdint.h>
#include <string.h>

#if !defined(SAM_CSP_DOMAIN_HAVE_IMPLEMENTATION)
sam_csp_domain_result_t sam_csp_domain_apply_outputs(
    const sam_csp_set_outputs_request_t *request)
{
    (void) request;
    return SAM_CSP_DOMAIN_INVALID_STATE;
}

sam_csp_domain_result_t sam_csp_domain_request_mode(uint8_t mode)
{
    (void) mode;
    return SAM_CSP_DOMAIN_INVALID_STATE;
}

sam_csp_domain_result_t sam_csp_domain_get_snapshot(
    sam_csp_snapshot_t *snapshot)
{
    (void) snapshot;
    return SAM_CSP_DOMAIN_SNAPSHOT_FAILED;
}
#endif

static void get_observations(fake_sam_csp_domain_observations_t *observations)
{
    memset(observations, 0, sizeof(*observations));
    fake_sam_csp_domain_get_observations(observations);
}

static void apply_outputs_rejects_null_without_actuator_calls(void)
{
    fake_sam_csp_domain_observations_t observations;

    fake_sam_csp_domain_reset();
    TEST_ASSERT_EQ_SIZE(
        SAM_CSP_DOMAIN_INVALID_STATE,
        sam_csp_domain_apply_outputs(NULL));
    get_observations(&observations);
    TEST_ASSERT_EQ_SIZE(0U, observations.actuator_call_count);
}

static void apply_outputs_prevalidates_every_field_before_hardware(void)
{
    const sam_csp_set_outputs_request_t invalid[] = {
        {.transaction_id = 1U, .lpv_on_mask = 0x1000U},
        {.transaction_id = 2U, .heater_on_mask = 0x10U},
        {.transaction_id = 3U, .spark_on = 2U},
        {
            .transaction_id = 4U,
            .lpv_on_mask = 0x1FFFU,
            .heater_on_mask = 0x10U,
            .spark_on = 2U,
        },
    };

    for (size_t index = 0U;
         index < (sizeof(invalid) / sizeof(invalid[0]));
         ++index) {
        fake_sam_csp_domain_observations_t observations;

        fake_sam_csp_domain_reset();
        TEST_ASSERT_EQ_SIZE(
            SAM_CSP_DOMAIN_INVALID_STATE,
            sam_csp_domain_apply_outputs(&invalid[index]));
        get_observations(&observations);
        TEST_ASSERT_EQ_SIZE(0U, observations.actuator_call_count);
    }
}

static void apply_outputs_maps_every_bit_in_exact_hardware_order(void)
{
    static const fake_sam_csp_actuator_call_t expected[] = {
        {FAKE_SAM_CSP_ACTUATOR_LPV, 1U, 1U},
        {FAKE_SAM_CSP_ACTUATOR_LPV, 2U, 0U},
        {FAKE_SAM_CSP_ACTUATOR_LPV, 3U, 1U},
        {FAKE_SAM_CSP_ACTUATOR_LPV, 4U, 0U},
        {FAKE_SAM_CSP_ACTUATOR_LPV, 5U, 1U},
        {FAKE_SAM_CSP_ACTUATOR_LPV, 6U, 0U},
        {FAKE_SAM_CSP_ACTUATOR_LPV, 7U, 1U},
        {FAKE_SAM_CSP_ACTUATOR_LPV, 8U, 0U},
        {FAKE_SAM_CSP_ACTUATOR_LPV, 9U, 0U},
        {FAKE_SAM_CSP_ACTUATOR_LPV, 10U, 1U},
        {FAKE_SAM_CSP_ACTUATOR_LPV, 11U, 0U},
        {FAKE_SAM_CSP_ACTUATOR_LPV, 12U, 1U},
        {FAKE_SAM_CSP_ACTUATOR_HPV, 1U, 0U},
        {FAKE_SAM_CSP_ACTUATOR_HPV, 2U, 1U},
        {FAKE_SAM_CSP_ACTUATOR_HPV, 3U, 1U},
        {FAKE_SAM_CSP_ACTUATOR_HPV, 4U, 0U},
        {FAKE_SAM_CSP_ACTUATOR_HPV, 5U, 0U},
        {FAKE_SAM_CSP_ACTUATOR_HPV, 6U, 1U},
        {FAKE_SAM_CSP_ACTUATOR_HPV, 7U, 0U},
        {FAKE_SAM_CSP_ACTUATOR_HPV, 8U, 1U},
        {FAKE_SAM_CSP_ACTUATOR_HEATER, 1U, 100U},
        {FAKE_SAM_CSP_ACTUATOR_HEATER, 2U, 0U},
        {FAKE_SAM_CSP_ACTUATOR_HEATER, 3U, 0U},
        {FAKE_SAM_CSP_ACTUATOR_HEATER, 4U, 100U},
        {FAKE_SAM_CSP_ACTUATOR_SPARK, 0U, 1U},
    };
    const sam_csp_set_outputs_request_t request = {
        .transaction_id = 0xBEEFU,
        .lpv_on_mask = 0x0A55U,
        .hpv_on_mask = 0xA6U,
        .heater_on_mask = 0x09U,
        .spark_on = 1U,
    };
    fake_sam_csp_domain_observations_t observations;

    fake_sam_csp_domain_reset();
    TEST_ASSERT_EQ_SIZE(
        SAM_CSP_DOMAIN_OK,
        sam_csp_domain_apply_outputs(&request));
    get_observations(&observations);
    TEST_ASSERT_EQ_SIZE(
        sizeof(expected) / sizeof(expected[0]),
        observations.actuator_call_count);
    for (size_t index = 0U;
         index < (sizeof(expected) / sizeof(expected[0]));
         ++index) {
        TEST_ASSERT_EQ_SIZE(
            expected[index].actuator,
            observations.actuator_calls[index].actuator);
        TEST_ASSERT_EQ_SIZE(
            expected[index].channel,
            observations.actuator_calls[index].channel);
        TEST_ASSERT_EQ_SIZE(
            expected[index].value,
            observations.actuator_calls[index].value);
    }
}

static void apply_outputs_sends_absolute_off_to_every_output(void)
{
    const sam_csp_set_outputs_request_t request = {
        .transaction_id = 0xCAFEU,
    };
    fake_sam_csp_domain_observations_t observations;

    fake_sam_csp_domain_reset();
    TEST_ASSERT_EQ_SIZE(
        SAM_CSP_DOMAIN_OK,
        sam_csp_domain_apply_outputs(&request));
    get_observations(&observations);
    TEST_ASSERT_EQ_SIZE(25U, observations.actuator_call_count);
    for (size_t index = 0U; index < observations.actuator_call_count; ++index) {
        TEST_ASSERT_EQ_SIZE(0U, observations.actuator_calls[index].value);
    }
    TEST_ASSERT_EQ_SIZE(
        FAKE_SAM_CSP_ACTUATOR_SPARK,
        observations.actuator_calls[24].actuator);
}

static void request_mode_accepts_and_forwards_all_defined_modes(void)
{
    static const uint8_t modes[] = {
        STATE_MACHINE_INIT_MODE,
        STATE_MACHINE_NORMAL_MODE,
        STATE_MACHINE_RUN_MODE,
        STATE_MACHINE_DIAGNOSTIC_MODE,
    };

    for (size_t index = 0U; index < (sizeof(modes) / sizeof(modes[0])); ++index) {
        fake_sam_csp_domain_observations_t observations;

        fake_sam_csp_domain_reset();
        fake_sam_csp_domain_set_state_request_result(1U);
        TEST_ASSERT_EQ_SIZE(
            SAM_CSP_DOMAIN_OK,
            sam_csp_domain_request_mode(modes[index]));
        get_observations(&observations);
        TEST_ASSERT_EQ_SIZE(1U, observations.state_request_calls);
        TEST_ASSERT_EQ_SIZE(modes[index], observations.last_requested_mode);
    }
}

static void request_mode_maps_state_machine_rejection_to_apply_failed(void)
{
    fake_sam_csp_domain_observations_t observations;

    fake_sam_csp_domain_reset();
    fake_sam_csp_domain_set_state_request_result(0U);
    TEST_ASSERT_EQ_SIZE(
        SAM_CSP_DOMAIN_APPLY_FAILED,
        sam_csp_domain_request_mode(STATE_MACHINE_RUN_MODE));
    get_observations(&observations);
    TEST_ASSERT_EQ_SIZE(1U, observations.state_request_calls);
    TEST_ASSERT_EQ_SIZE(
        STATE_MACHINE_RUN_MODE,
        observations.last_requested_mode);
}

static void request_mode_rejects_out_of_range_without_state_machine_call(void)
{
    static const uint8_t invalid_modes[] = {
        STATE_MACHINE_MODE_COUNT,
        UINT8_MAX,
    };

    for (size_t index = 0U;
         index < (sizeof(invalid_modes) / sizeof(invalid_modes[0]));
         ++index) {
        fake_sam_csp_domain_observations_t observations;

        fake_sam_csp_domain_reset();
        TEST_ASSERT_EQ_SIZE(
            SAM_CSP_DOMAIN_INVALID_STATE,
            sam_csp_domain_request_mode(invalid_modes[index]));
        get_observations(&observations);
        TEST_ASSERT_EQ_SIZE(0U, observations.state_request_calls);
    }
}

static void get_snapshot_rejects_null_without_dependency_calls(void)
{
    fake_sam_csp_domain_observations_t observations;

    fake_sam_csp_domain_reset();
    TEST_ASSERT_EQ_SIZE(
        SAM_CSP_DOMAIN_SNAPSHOT_FAILED,
        sam_csp_domain_get_snapshot(NULL));
    get_observations(&observations);
    TEST_ASSERT_EQ_SIZE(0U, observations.state_snapshot_calls);
    TEST_ASSERT_EQ_SIZE(0U, observations.sensor_scan_calls);
    TEST_ASSERT_EQ_SIZE(0U, observations.pt_count_calls);
    TEST_ASSERT_EQ_SIZE(0U, observations.tc_count_calls);
    TEST_ASSERT_EQ_SIZE(0U, observations.tick_count_calls);
}

static sSensorScan populated_sensor_scan(void)
{
    const sSensorScan scan = {
        .pt = {
            .adcMilliVolt = {
                -9000, -1, 0, 1, 125, 3300, 4567, 123456, 2000000000,
            },
        },
        .tc = {
            .microVolt = {-2000000000, -12345, 67890, 2000000000},
        },
    };

    return scan;
}

static void assert_snapshot_dependencies_called_once(void)
{
    fake_sam_csp_domain_observations_t observations;

    get_observations(&observations);
    TEST_ASSERT_EQ_SIZE(1U, observations.state_snapshot_calls);
    TEST_ASSERT_EQ_SIZE(1U, observations.sensor_scan_calls);
    TEST_ASSERT_EQ_SIZE(1U, observations.pt_count_calls);
    TEST_ASSERT_EQ_SIZE(1U, observations.tc_count_calls);
    TEST_ASSERT_EQ_SIZE(1U, observations.tick_count_calls);
}

static void get_snapshot_zeros_unpublished_sensor_families(void)
{
    const sStateMachineSnapshot state = {
        .currentMode = STATE_MACHINE_DIAGNOSTIC_MODE,
        .requestedMode = STATE_MACHINE_RUN_MODE,
    };
    const sSensorScan scan = populated_sensor_scan();
    const int32_t zero_pt[SAM_CSP_SNAPSHOT_PT_COUNT] = {0};
    const int32_t zero_tc[SAM_CSP_SNAPSHOT_TC_COUNT] = {0};
    sam_csp_snapshot_t snapshot;

    fake_sam_csp_domain_reset();
    fake_sam_csp_domain_set_state_snapshot(&state);
    fake_sam_csp_domain_set_sensor_snapshot(&scan, 0U, 0U);
    fake_sam_csp_domain_set_tick_count(123U);
    memset(&snapshot, 0xA5, sizeof(snapshot));

    TEST_ASSERT_EQ_SIZE(
        SAM_CSP_DOMAIN_OK,
        sam_csp_domain_get_snapshot(&snapshot));
    TEST_ASSERT_EQ_SIZE(1230U, snapshot.sample_time_ms);
    TEST_ASSERT_EQ_SIZE(STATE_MACHINE_DIAGNOSTIC_MODE, snapshot.current_mode);
    TEST_ASSERT_EQ_SIZE(STATE_MACHINE_RUN_MODE, snapshot.requested_mode);
    TEST_ASSERT_EQ_SIZE(0U, snapshot.validity_mask);
    TEST_ASSERT_TRUE(
        memcmp(snapshot.pt_millivolt, zero_pt, sizeof(zero_pt)) == 0);
    TEST_ASSERT_TRUE(
        memcmp(snapshot.tc_microvolt, zero_tc, sizeof(zero_tc)) == 0);
    assert_snapshot_dependencies_called_once();
}

static void get_snapshot_publishes_only_pt_after_first_pt_scan(void)
{
    const sSensorScan scan = populated_sensor_scan();
    const int32_t expected_pt[SAM_CSP_SNAPSHOT_PT_COUNT] = {
        -9000, -1, 0, 1, 125, 3300, 4567, 123456, 2000000000,
    };
    const int32_t zero_tc[SAM_CSP_SNAPSHOT_TC_COUNT] = {0};
    sam_csp_snapshot_t snapshot = {0};

    fake_sam_csp_domain_reset();
    fake_sam_csp_domain_set_sensor_snapshot(&scan, 7U, 0U);

    TEST_ASSERT_EQ_SIZE(
        SAM_CSP_DOMAIN_OK,
        sam_csp_domain_get_snapshot(&snapshot));
    TEST_ASSERT_EQ_SIZE(0x01FFU, snapshot.validity_mask);
    TEST_ASSERT_TRUE(
        memcmp(snapshot.pt_millivolt, expected_pt, sizeof(expected_pt)) == 0);
    TEST_ASSERT_TRUE(
        memcmp(snapshot.tc_microvolt, zero_tc, sizeof(zero_tc)) == 0);
    assert_snapshot_dependencies_called_once();
}

static void get_snapshot_publishes_only_tc_after_first_tc_scan(void)
{
    const sSensorScan scan = populated_sensor_scan();
    const int32_t zero_pt[SAM_CSP_SNAPSHOT_PT_COUNT] = {0};
    const int32_t expected_tc[SAM_CSP_SNAPSHOT_TC_COUNT] = {
        -2000000000, -12345, 67890, 2000000000,
    };
    sam_csp_snapshot_t snapshot = {0};

    fake_sam_csp_domain_reset();
    fake_sam_csp_domain_set_sensor_snapshot(&scan, 0U, 9U);

    TEST_ASSERT_EQ_SIZE(
        SAM_CSP_DOMAIN_OK,
        sam_csp_domain_get_snapshot(&snapshot));
    TEST_ASSERT_EQ_SIZE(0x1E00U, snapshot.validity_mask);
    TEST_ASSERT_TRUE(
        memcmp(snapshot.pt_millivolt, zero_pt, sizeof(zero_pt)) == 0);
    TEST_ASSERT_TRUE(
        memcmp(snapshot.tc_microvolt, expected_tc, sizeof(expected_tc)) == 0);
    assert_snapshot_dependencies_called_once();
}

static void get_snapshot_maps_all_published_values_modes_and_time(void)
{
    const sStateMachineSnapshot state = {
        .currentMode = STATE_MACHINE_NORMAL_MODE,
        .requestedMode = STATE_MACHINE_DIAGNOSTIC_MODE,
    };
    const sSensorScan scan = populated_sensor_scan();
    const int32_t expected_pt[SAM_CSP_SNAPSHOT_PT_COUNT] = {
        -9000, -1, 0, 1, 125, 3300, 4567, 123456, 2000000000,
    };
    const int32_t expected_tc[SAM_CSP_SNAPSHOT_TC_COUNT] = {
        -2000000000, -12345, 67890, 2000000000,
    };
    sam_csp_snapshot_t snapshot = {0};

    fake_sam_csp_domain_reset();
    fake_sam_csp_domain_set_state_snapshot(&state);
    fake_sam_csp_domain_set_sensor_snapshot(&scan, 1U, 1U);
    fake_sam_csp_domain_set_tick_count(0x00123456U);

    TEST_ASSERT_EQ_SIZE(
        SAM_CSP_DOMAIN_OK,
        sam_csp_domain_get_snapshot(&snapshot));
    TEST_ASSERT_EQ_SIZE(0x00B60B5CU, snapshot.sample_time_ms);
    TEST_ASSERT_EQ_SIZE(STATE_MACHINE_NORMAL_MODE, snapshot.current_mode);
    TEST_ASSERT_EQ_SIZE(
        STATE_MACHINE_DIAGNOSTIC_MODE,
        snapshot.requested_mode);
    TEST_ASSERT_EQ_SIZE(0x1FFFU, snapshot.validity_mask);
    TEST_ASSERT_TRUE(
        memcmp(snapshot.pt_millivolt, expected_pt, sizeof(expected_pt)) == 0);
    TEST_ASSERT_TRUE(
        memcmp(snapshot.tc_microvolt, expected_tc, sizeof(expected_tc)) == 0);
    assert_snapshot_dependencies_called_once();
}

const test_case_t sam_csp_domain_tests[] = {
    {"sam_csp_domain", "apply_outputs_rejects_null_without_actuator_calls", apply_outputs_rejects_null_without_actuator_calls},
    {"sam_csp_domain", "apply_outputs_prevalidates_every_field_before_hardware", apply_outputs_prevalidates_every_field_before_hardware},
    {"sam_csp_domain", "apply_outputs_maps_every_bit_in_exact_hardware_order", apply_outputs_maps_every_bit_in_exact_hardware_order},
    {"sam_csp_domain", "apply_outputs_sends_absolute_off_to_every_output", apply_outputs_sends_absolute_off_to_every_output},
    {"sam_csp_domain", "request_mode_accepts_and_forwards_all_defined_modes", request_mode_accepts_and_forwards_all_defined_modes},
    {"sam_csp_domain", "request_mode_maps_state_machine_rejection_to_apply_failed", request_mode_maps_state_machine_rejection_to_apply_failed},
    {"sam_csp_domain", "request_mode_rejects_out_of_range_without_state_machine_call", request_mode_rejects_out_of_range_without_state_machine_call},
    {"sam_csp_domain", "get_snapshot_rejects_null_without_dependency_calls", get_snapshot_rejects_null_without_dependency_calls},
    {"sam_csp_domain", "get_snapshot_zeros_unpublished_sensor_families", get_snapshot_zeros_unpublished_sensor_families},
    {"sam_csp_domain", "get_snapshot_publishes_only_pt_after_first_pt_scan", get_snapshot_publishes_only_pt_after_first_pt_scan},
    {"sam_csp_domain", "get_snapshot_publishes_only_tc_after_first_tc_scan", get_snapshot_publishes_only_tc_after_first_tc_scan},
    {"sam_csp_domain", "get_snapshot_maps_all_published_values_modes_and_time", get_snapshot_maps_all_published_values_modes_and_time},
};

const size_t sam_csp_domain_test_count =
    sizeof(sam_csp_domain_tests) / sizeof(sam_csp_domain_tests[0]);
