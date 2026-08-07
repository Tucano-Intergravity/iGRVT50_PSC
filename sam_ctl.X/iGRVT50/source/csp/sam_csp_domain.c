#include <csp/sam_csp_domain.h>

#include "FreeRTOS.h"
#include "task.h"

#include "hpsolvalve.h"
#include "lpsolvalve.h"
#include "sam_ctl.h"
#include "sensor.h"
#include "statemachine.h"

#include <stdint.h>
#include <string.h>

#define SAM_CSP_DOMAIN_LPV_MASK       0x0FFFU
#define SAM_CSP_DOMAIN_HEATER_MASK    0x0FU
#define SAM_CSP_DOMAIN_LPV_COUNT      12U
#define SAM_CSP_DOMAIN_HPV_COUNT      8U
#define SAM_CSP_DOMAIN_HEATER_COUNT   4U
#define SAM_CSP_DOMAIN_PT_VALIDITY    0x01FFU
#define SAM_CSP_DOMAIN_TC_VALIDITY    0x1E00U

sam_csp_domain_result_t sam_csp_domain_apply_outputs(
    const sam_csp_set_outputs_request_t *request)
{
    uint8_t channel;

    if ((request == NULL)
        || (request->lpv_on_mask > SAM_CSP_DOMAIN_LPV_MASK)
        || (request->heater_on_mask > SAM_CSP_DOMAIN_HEATER_MASK)
        || (request->spark_on > 1U)) {
        return SAM_CSP_DOMAIN_INVALID_STATE;
    }

    for (channel = 0U; channel < SAM_CSP_DOMAIN_LPV_COUNT; ++channel) {
        const uint8_t on = (uint8_t) ((request->lpv_on_mask >> channel) & 1U);
        LpSolValve_Set((UInt8) (channel + 1U), (UInt8) on);
    }

    for (channel = 0U; channel < SAM_CSP_DOMAIN_HPV_COUNT; ++channel) {
        const uint8_t on = (uint8_t) ((request->hpv_on_mask >> channel) & 1U);
        HpSolValve_Set((UInt8) (channel + 1U), (UInt8) on);
    }

    for (channel = 0U; channel < SAM_CSP_DOMAIN_HEATER_COUNT; ++channel) {
        const uint8_t on = (uint8_t) ((request->heater_on_mask >> channel) & 1U);
        const uint8_t duty = (on != 0U) ? 100U : 0U;
        Heater_SetDuty((UInt8) (channel + 1U), (UInt8) duty);
    }

    SparkPlug_Set((UInt8) request->spark_on);
    return SAM_CSP_DOMAIN_OK;
}

sam_csp_domain_result_t sam_csp_domain_request_mode(uint8_t mode)
{
    if (mode >= (uint8_t) STATE_MACHINE_MODE_COUNT) {
        return SAM_CSP_DOMAIN_INVALID_STATE;
    }

    if (StateMachine_RequestMode((eStateMachineMode) mode) == 0U) {
        return SAM_CSP_DOMAIN_APPLY_FAILED;
    }

    return SAM_CSP_DOMAIN_OK;
}

sam_csp_domain_result_t sam_csp_domain_get_snapshot(
    sam_csp_snapshot_t *snapshot)
{
    sStateMachineSnapshot state_snapshot;
    sSensorScan sensor_scan;
    UInt32 pt_scan_count;
    UInt32 tc_scan_count;
    TickType_t sample_ticks;
    uint8_t index;

    if (snapshot == NULL) {
        return SAM_CSP_DOMAIN_SNAPSHOT_FAILED;
    }

    memset(snapshot, 0, sizeof(*snapshot));
    memset(&state_snapshot, 0, sizeof(state_snapshot));
    memset(&sensor_scan, 0, sizeof(sensor_scan));

    pt_scan_count = Sensor_GetPtScanCount();
    tc_scan_count = Sensor_GetTcScanCount();
    Sensor_GetScan(&sensor_scan);
    StateMachine_GetSnapshot(&state_snapshot);
    sample_ticks = xTaskGetTickCount();

    snapshot->sample_time_ms = (uint32_t)
        (((uint64_t) sample_ticks * UINT64_C(1000))
            / (uint64_t) configTICK_RATE_HZ);
    snapshot->current_mode = (uint8_t) state_snapshot.currentMode;
    snapshot->requested_mode = (uint8_t) state_snapshot.requestedMode;

    if (pt_scan_count != 0U) {
        snapshot->validity_mask |= SAM_CSP_DOMAIN_PT_VALIDITY;
        for (index = 0U; index < SAM_CSP_SNAPSHOT_PT_COUNT; ++index) {
            snapshot->pt_millivolt[index] =
                (int32_t) sensor_scan.pt.adcMilliVolt[index];
        }
    }

    if (tc_scan_count != 0U) {
        snapshot->validity_mask |= SAM_CSP_DOMAIN_TC_VALIDITY;
        for (index = 0U; index < SAM_CSP_SNAPSHOT_TC_COUNT; ++index) {
            snapshot->tc_microvolt[index] =
                (int32_t) sensor_scan.tc.microVolt[index];
        }
    }

    return SAM_CSP_DOMAIN_OK;
}
