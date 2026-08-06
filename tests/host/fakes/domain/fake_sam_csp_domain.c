#include "fake_sam_csp_domain.h"

#include "hpsolvalve.h"
#include "lpsolvalve.h"

#include <string.h>

static fake_sam_csp_domain_observations_t observations;
static UInt8 state_request_result;
static sStateMachineSnapshot state_snapshot;
static sSensorScan sensor_snapshot;
static UInt32 pt_scan_count;
static UInt32 tc_scan_count;
static TickType_t tick_count;

static void record_actuator_call(
    fake_sam_csp_actuator_t actuator,
    UInt8 channel,
    UInt8 value)
{
    if (observations.actuator_call_count
        < FAKE_SAM_CSP_DOMAIN_MAX_ACTUATOR_CALLS) {
        fake_sam_csp_actuator_call_t *call =
            &observations.actuator_calls[observations.actuator_call_count];
        call->actuator = actuator;
        call->channel = channel;
        call->value = value;
    }
    ++observations.actuator_call_count;
}

void fake_sam_csp_domain_reset(void)
{
    memset(&observations, 0, sizeof(observations));
    state_request_result = 1U;
    memset(&state_snapshot, 0, sizeof(state_snapshot));
    memset(&sensor_snapshot, 0, sizeof(sensor_snapshot));
    pt_scan_count = 0U;
    tc_scan_count = 0U;
    tick_count = 0U;
}

void fake_sam_csp_domain_get_observations(
    fake_sam_csp_domain_observations_t *result)
{
    if (result != NULL) {
        *result = observations;
    }
}

void fake_sam_csp_domain_set_state_request_result(UInt8 result)
{
    state_request_result = result;
}

void fake_sam_csp_domain_set_state_snapshot(
    const sStateMachineSnapshot *snapshot)
{
    if (snapshot != NULL) {
        state_snapshot = *snapshot;
    }
}

void fake_sam_csp_domain_set_sensor_snapshot(
    const sSensorScan *scan,
    UInt32 pt_count,
    UInt32 tc_count)
{
    if (scan != NULL) {
        sensor_snapshot = *scan;
    }
    pt_scan_count = pt_count;
    tc_scan_count = tc_count;
}

void fake_sam_csp_domain_set_tick_count(TickType_t ticks)
{
    tick_count = ticks;
}

void LpSolValve_Set(UInt8 ch, UInt8 on)
{
    record_actuator_call(FAKE_SAM_CSP_ACTUATOR_LPV, ch, on);
}

void HpSolValve_Set(UInt8 ch, UInt8 on)
{
    record_actuator_call(FAKE_SAM_CSP_ACTUATOR_HPV, ch, on);
}

void Heater_SetDuty(UInt8 ch, UInt8 dutyPct)
{
    record_actuator_call(FAKE_SAM_CSP_ACTUATOR_HEATER, ch, dutyPct);
}

void SparkPlug_Set(UInt8 on)
{
    record_actuator_call(FAKE_SAM_CSP_ACTUATOR_SPARK, 0U, on);
}

UInt8 StateMachine_RequestMode(eStateMachineMode mode)
{
    ++observations.state_request_calls;
    observations.last_requested_mode = mode;
    return state_request_result;
}

void StateMachine_GetSnapshot(sStateMachineSnapshot *snapshot)
{
    ++observations.state_snapshot_calls;
    if (snapshot != NULL) {
        *snapshot = state_snapshot;
    }
}

void Sensor_GetScan(sSensorScan *scan)
{
    ++observations.sensor_scan_calls;
    if (scan != NULL) {
        *scan = sensor_snapshot;
    }
}

UInt32 Sensor_GetPtScanCount(void)
{
    ++observations.pt_count_calls;
    return pt_scan_count;
}

UInt32 Sensor_GetTcScanCount(void)
{
    ++observations.tc_count_calls;
    return tc_scan_count;
}

TickType_t xTaskGetTickCount(void)
{
    ++observations.tick_count_calls;
    return tick_count;
}
