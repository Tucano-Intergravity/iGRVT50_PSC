#include "csp/sam_csp_domain.h"

#include <stddef.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"

#include "hpsolvalve.h"
#include "lpsolvalve.h"
#include "sam_ctl.h"
#include "sensor.h"
#include "statemachine.h"

#define SAM_CSP_SENSOR_VALID_PT_MASK    0x01FFU
#define SAM_CSP_SENSOR_VALID_TC_MASK    0x3E00U
#define SAM_CSP_TEMPERATURE_VALID_MASK  0x001FU

static volatile uint16_t s_lpv_on_mask = 0U;
static volatile uint8_t s_hpv_on_mask = 0U;
static volatile uint8_t s_heater_on_mask = 0U;
static volatile uint8_t s_spark_on = 0U;

static const ePscLpvValve k_lpv_valve_map[LPSOLVALVE_CHANNEL_COUNT] = {
    PSC_LPV_SV_R01,
    PSC_LPV_SV_R02,
    PSC_LPV_SV_R03,
    PSC_LPV_SV_R04,
    PSC_LPV_SV_R05,
    PSC_LPV_SV_R06,
    PSC_LPV_SV_R07,
    PSC_LPV_SV_R08,
    PSC_LPV_SV_R09,
    PSC_LPV_SV_R10,
    PSC_LPV_SV_R11,
    PSC_LPV_SV_R12
};

static const ePscHpvValve k_hpv_valve_map[HPSOLVALVE_CHANNEL_COUNT] = {
    PSC_HPV_SV_O1,
    PSC_HPV_SV_O2,
    PSC_HPV_SV_O3,
    PSC_HPV_SPARE4,
    PSC_HPV_SV_F1,
    PSC_HPV_SV_F2,
    PSC_HPV_SV_F3,
    PSC_HPV_SPARE8
};

static uint8_t domain_is_diagnostic_mode(void)
{
    return (StateMachine_GetMode() == STATE_MACHINE_DIAGNOSTIC_MODE) ? 1U : 0U;
}

static uint8_t domain_is_sim_start_allowed_mode(void)
{
    const eStateMachineMode mode = StateMachine_GetMode();

    return ((mode == STATE_MACHINE_NORMAL_MODE) ||
            (mode == STATE_MACHINE_DIAGNOSTIC_MODE)) ? 1U : 0U;
}

void SamCspDomain_RecordLpvState(uint8_t ch, uint8_t on)
{
    if ((ch < 1U) || (ch > LPSOLVALVE_CHANNEL_COUNT)) {
        return;
    }
    taskENTER_CRITICAL();
    if (on != 0U) {
        s_lpv_on_mask |= (uint16_t)(1U << (ch - 1U));
    } else {
        s_lpv_on_mask &= (uint16_t)~(uint16_t)(1U << (ch - 1U));
    }
    taskEXIT_CRITICAL();
}

void SamCspDomain_RecordHpvState(uint8_t ch, uint8_t on)
{
    if ((ch < 1U) || (ch > HPSOLVALVE_CHANNEL_COUNT)) {
        return;
    }
    taskENTER_CRITICAL();
    if (on != 0U) {
        s_hpv_on_mask |= (uint8_t)(1U << (ch - 1U));
    } else {
        s_hpv_on_mask &= (uint8_t)~(uint8_t)(1U << (ch - 1U));
    }
    taskEXIT_CRITICAL();
}

void SamCspDomain_RecordHeaterState(uint8_t ch, uint8_t on)
{
    if ((ch < 1U) || (ch > 4U)) {
        return;
    }
    taskENTER_CRITICAL();
    if (on != 0U) {
        s_heater_on_mask |= (uint8_t)(1U << (ch - 1U));
    } else {
        s_heater_on_mask &= (uint8_t)~(uint8_t)(1U << (ch - 1U));
    }
    taskEXIT_CRITICAL();
}

void SamCspDomain_RecordSparkPlugState(uint8_t on)
{
    taskENTER_CRITICAL();
    s_spark_on = (on != 0U) ? 1U : 0U;
    taskEXIT_CRITICAL();
}

void SamCspDomain_RecordAllOutputsOff(void)
{
    taskENTER_CRITICAL();
    s_lpv_on_mask = 0U;
    s_hpv_on_mask = 0U;
    s_heater_on_mask = 0U;
    s_spark_on = 0U;
    taskEXIT_CRITICAL();
}

sam_csp_domain_result_t SamCspDomain_ApplyOutputs(
    const sam_csp_set_outputs_request_t *request)
{
    uint8_t i;

    if (request == NULL) {
        return SAM_CSP_DOMAIN_APPLY_FAILED;
    }

    for (i = 0U; i < LPSOLVALVE_CHANNEL_COUNT; i++) {
        const ePscLpvValve valve = k_lpv_valve_map[i];
        const uint8_t on =
            ((request->lpv_on_mask & (uint16_t)(1U << i)) != 0U) ? 1U : 0U;
        LpSolValve_Set((UInt8)valve, (UInt8)on);
        SamCspDomain_RecordLpvState((uint8_t)valve, on);
    }

    if (domain_is_diagnostic_mode() != 0U) {
        for (i = 0U; i < HPSOLVALVE_CHANNEL_COUNT; i++) {
            const ePscHpvValve valve = k_hpv_valve_map[i];
            const uint8_t on =
                ((request->hpv_on_mask & (uint8_t)(1U << i)) != 0U) ? 1U : 0U;
            HpSolValve_Set((UInt8)valve, (UInt8)on);
            SamCspDomain_RecordHpvState((uint8_t)valve, on);
        }
    }

    for (i = 0U; i < 4U; i++) {
        const uint8_t on =
            ((request->heater_on_mask & (uint8_t)(1U << i)) != 0U) ? 1U : 0U;
        Heater_SetDuty((UInt8)(i + 1U), (UInt8)((on != 0U) ? 100U : 0U));
        SamCspDomain_RecordHeaterState((uint8_t)(i + 1U), on);
    }

    SparkPlug_Set((UInt8)request->spark_on);
    SamCspDomain_RecordSparkPlugState(request->spark_on);
    return SAM_CSP_DOMAIN_OK;
}

sam_csp_domain_result_t SamCspDomain_ApplyLpvOutputs(
    const sam_csp_set_lpv_outputs_request_t *request)
{
    uint8_t i;

    if (request == NULL) {
        return SAM_CSP_DOMAIN_APPLY_FAILED;
    }

    for (i = 0U; i < LPSOLVALVE_CHANNEL_COUNT; i++) {
        const ePscLpvValve valve = k_lpv_valve_map[i];
        const uint8_t on =
            ((request->lpv_on_mask & (uint16_t)(1U << i)) != 0U) ? 1U : 0U;
        LpSolValve_Set((UInt8)valve, (UInt8)on);
        SamCspDomain_RecordLpvState((uint8_t)valve, on);
    }

    return SAM_CSP_DOMAIN_OK;
}

sam_csp_domain_result_t SamCspDomain_RequestMode(uint8_t mode)
{
    if (mode >= STATE_MACHINE_MODE_COUNT) {
        return SAM_CSP_DOMAIN_INVALID_STATE;
    }
    return (StateMachine_RequestMode((eStateMachineMode)mode) != 0U)
        ? SAM_CSP_DOMAIN_OK
        : SAM_CSP_DOMAIN_APPLY_FAILED;
}

sam_csp_domain_result_t SamCspDomain_StartThruster(
    const sam_csp_thruster_start_request_t *request)
{
    sThrusterStartParams params;

    if (request == NULL) {
        return SAM_CSP_DOMAIN_APPLY_FAILED;
    }

    params.burnTimeMs = (UInt32)request->burn_time_ms;
    params.svO3OpenDelayMs = (UInt32)request->sv_o3_open_delay_ms;
    params.svF3OpenDelayMs = (UInt32)request->sv_f3_open_delay_ms;
    params.sparkOnDelayMs = (UInt32)request->spark_on_delay_ms;
    params.sparkOnDurationMs = (UInt32)request->spark_on_duration_ms;
    params.svO3CloseDelayMs = (UInt32)request->sv_o3_close_delay_ms;
    params.svF3CloseDelayMs = (UInt32)request->sv_f3_close_delay_ms;

    return (Opu_RequestThrusterStart(&params) != 0U)
        ? SAM_CSP_DOMAIN_OK
        : SAM_CSP_DOMAIN_INVALID_STATE;
}

sam_csp_domain_result_t SamCspDomain_StartPar(
    const sam_csp_par_start_request_t *request)
{
    sParStartParams params;

    if (request == NULL) {
        return SAM_CSP_DOMAIN_APPLY_FAILED;
    }

    params.oxidizerPressureSensor =
        ((request->selector & SAM_CSP_PAR_OXIDIZER_PT_O4_MASK) != 0U)
            ? PSC_PT_O4
            : PSC_PT_O3;
    params.fuelPressureSensor =
        ((request->selector & SAM_CSP_PAR_FUEL_PT_F4_MASK) != 0U)
            ? PSC_PT_F4
            : PSC_PT_F3;

    return (Opu_RequestParStart(&params) != 0U)
        ? SAM_CSP_DOMAIN_OK
        : SAM_CSP_DOMAIN_INVALID_STATE;
}

sam_csp_domain_result_t SamCspDomain_StopPar(void)
{
    return (Opu_RequestParStop() != 0U)
        ? SAM_CSP_DOMAIN_OK
        : SAM_CSP_DOMAIN_APPLY_FAILED;
}

sam_csp_domain_result_t SamCspDomain_StartSim(void)
{
    if (domain_is_sim_start_allowed_mode() == 0U) {
        return SAM_CSP_DOMAIN_INVALID_STATE;
    }
    if (SensorOverride_IsEnabled() != 0U) {
        return SAM_CSP_DOMAIN_OK;
    }

    SensorOverride_StartFromCurrent();
    return SAM_CSP_DOMAIN_OK;
}

sam_csp_domain_result_t SamCspDomain_StopSim(void)
{
    SensorOverride_Stop();
    return SAM_CSP_DOMAIN_OK;
}

sam_csp_domain_result_t SamCspDomain_SetSimSensors(
    const sam_csp_set_sim_sensor_request_t *request)
{
    if (request == NULL) {
        return SAM_CSP_DOMAIN_APPLY_FAILED;
    }
    if (SensorOverride_IsEnabled() == 0U) {
        return SAM_CSP_DOMAIN_INVALID_STATE;
    }

    SensorOverride_SetValues(
        request->pt_millibar,
        request->tc_millikelvin,
        SAM_CSP_PT_CHANNEL_COUNT,
        SAM_CSP_TC_CHANNEL_COUNT);
    return SAM_CSP_DOMAIN_OK;
}

sam_csp_domain_result_t SamCspDomain_ClearFaults(void)
{
    Opu_ClearThrusterFaults();
    return SAM_CSP_DOMAIN_OK;
}

sam_csp_domain_result_t SamCspDomain_GetSensorSnapshot(
    sam_csp_sensor_snapshot_t *snapshot)
{
    sSensorScan scan;
    sSensorTcTemperatureScan temperature;
    sStateMachineSnapshot state;
    uint8_t i;
    uint16_t validity = 0U;
    uint16_t temperature_validity = 0U;

    if (snapshot == NULL) {
        return SAM_CSP_DOMAIN_SNAPSHOT_FAILED;
    }

    Sensor_GetScan(&scan);
    Sensor_GetTcTemperatureScan(&temperature);
    StateMachine_GetSnapshot(&state);

    if (Sensor_GetPtScanCount() != 0U) {
        validity |= SAM_CSP_SENSOR_VALID_PT_MASK;
    }
    if (Sensor_GetTcScanCount() != 0U) {
        validity |= SAM_CSP_SENSOR_VALID_TC_MASK;
    }
    if (Sensor_GetTcTemperatureScanCount() != 0U) {
        temperature_validity =
            (uint16_t)(temperature.validMask & SAM_CSP_TEMPERATURE_VALID_MASK);
    }

    snapshot->sample_time_ms = (uint32_t)xTaskGetTickCount();
    snapshot->current_mode = (uint8_t)state.currentMode;
    snapshot->requested_mode = (uint8_t)state.requestedMode;
    snapshot->validity_mask = validity;
    for (i = 0U; i < SENSOR_PT_CHANNEL_COUNT; i++) {
        snapshot->pt_millivolt[i] =
            ((validity & SAM_CSP_SENSOR_VALID_PT_MASK) != 0U)
                ? (int32_t)scan.pt.adcMilliVolt[i]
                : 0;
        snapshot->pt_millibar[i] =
            ((validity & SAM_CSP_SENSOR_VALID_PT_MASK) != 0U)
                ? (int32_t)scan.pt.pressureMilliBar[i]
                : 0;
    }
    for (i = 0U; i < SENSOR_TC_CHANNEL_COUNT; i++) {
        snapshot->tc_microvolt[i] =
            ((validity & SAM_CSP_SENSOR_VALID_TC_MASK) != 0U)
                ? (int32_t)scan.tc.microVolt[i]
                : 0;
        snapshot->tc_millikelvin[i] =
            ((temperature_validity & (uint16_t)(1U << i)) != 0U)
                ? (int32_t)temperature.milliKelvin[i]
                : 0;
    }
    return SAM_CSP_DOMAIN_OK;
}

sam_csp_domain_result_t SamCspDomain_GetSolvalveSnapshot(
    sam_csp_solvalve_snapshot_t *snapshot)
{
    uint16_t lpv_mask;
    uint8_t hpv_mask;
    uint8_t heater_mask;
    uint8_t spark_on;

    if (snapshot == NULL) {
        return SAM_CSP_DOMAIN_SNAPSHOT_FAILED;
    }

    taskENTER_CRITICAL();
    lpv_mask = s_lpv_on_mask;
    hpv_mask = s_hpv_on_mask;
    heater_mask = s_heater_on_mask;
    spark_on = s_spark_on;
    taskEXIT_CRITICAL();

    snapshot->sample_time_ms = (uint32_t)xTaskGetTickCount();
    snapshot->current_mode = (uint8_t)StateMachine_GetMode();
    snapshot->lpv_on_mask = lpv_mask;
    snapshot->hpv_on_mask = hpv_mask;
    snapshot->heater_on_mask = heater_mask;
    snapshot->spark_on = spark_on;
    return SAM_CSP_DOMAIN_OK;
}
