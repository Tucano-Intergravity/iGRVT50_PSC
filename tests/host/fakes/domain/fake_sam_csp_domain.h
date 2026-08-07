#ifndef TEST_FAKE_SAM_CSP_DOMAIN_H
#define TEST_FAKE_SAM_CSP_DOMAIN_H

#include <FreeRTOS.h>

#include "sensor.h"
#include "statemachine.h"

#include <stddef.h>
#include <stdint.h>

#define FAKE_SAM_CSP_DOMAIN_MAX_ACTUATOR_CALLS 32U
#define FAKE_SAM_CSP_DOMAIN_MAX_DEPENDENCY_EVENTS 8U

typedef enum {
    FAKE_SAM_CSP_ACTUATOR_LPV = 0,
    FAKE_SAM_CSP_ACTUATOR_HPV,
    FAKE_SAM_CSP_ACTUATOR_HEATER,
    FAKE_SAM_CSP_ACTUATOR_SPARK,
} fake_sam_csp_actuator_t;

typedef struct {
    fake_sam_csp_actuator_t actuator;
    uint8_t channel;
    uint8_t value;
} fake_sam_csp_actuator_call_t;

typedef enum {
    FAKE_SAM_CSP_DEPENDENCY_PT_COUNT = 0,
    FAKE_SAM_CSP_DEPENDENCY_TC_COUNT,
    FAKE_SAM_CSP_DEPENDENCY_SENSOR_SCAN,
    FAKE_SAM_CSP_DEPENDENCY_STATE_SNAPSHOT,
    FAKE_SAM_CSP_DEPENDENCY_TICK_COUNT,
} fake_sam_csp_dependency_event_t;

typedef struct {
    fake_sam_csp_actuator_call_t
        actuator_calls[FAKE_SAM_CSP_DOMAIN_MAX_ACTUATOR_CALLS];
    size_t actuator_call_count;
    size_t state_request_calls;
    eStateMachineMode last_requested_mode;
    size_t state_snapshot_calls;
    size_t sensor_scan_calls;
    size_t pt_count_calls;
    size_t tc_count_calls;
    size_t tick_count_calls;
    fake_sam_csp_dependency_event_t
        dependency_events[FAKE_SAM_CSP_DOMAIN_MAX_DEPENDENCY_EVENTS];
    size_t dependency_event_count;
} fake_sam_csp_domain_observations_t;

void fake_sam_csp_domain_reset(void);
void fake_sam_csp_domain_get_observations(
    fake_sam_csp_domain_observations_t *observations);
void fake_sam_csp_domain_set_state_request_result(UInt8 result);
void fake_sam_csp_domain_set_state_snapshot(
    const sStateMachineSnapshot *snapshot);
void fake_sam_csp_domain_set_sensor_snapshot(
    const sSensorScan *scan,
    UInt32 pt_count,
    UInt32 tc_count);
void fake_sam_csp_domain_set_tick_count(TickType_t ticks);

#endif
