#include "test_sensor_under_test.h"

#include "fakes/freertos/fake_freertos.h"

#include <FreeRTOS.h>
#include <task.h>

#include <string.h>

static test_sensor_dependency_observations_t dependency_observations;
static UInt8 tc_bypass = 1U;

float AFEC_ToVoltage(UInt16 raw)
{
    ++dependency_observations.afec_conversion_calls;
    if (fake_freertos_task_in_critical()) {
        ++dependency_observations.conversion_calls_in_critical;
    }
    return (float) raw / 1000.0f;
}

UInt8 ADS1263_GetBypass(void)
{
    ++dependency_observations.tc_gain_calls;
    if (fake_freertos_task_in_critical()) {
        ++dependency_observations.conversion_calls_in_critical;
    }
    return tc_bypass;
}

#define Sensor_UpdatePtRawAdcScan test_sensor_update_pt_raw_scan
#define Sensor_GetPtScanCount test_sensor_get_pt_scan_count
#define Sensor_UpdateTcRawScan test_sensor_update_tc_raw_scan
#define Sensor_GetTcScanCount test_sensor_get_tc_scan_count
#define Sensor_GetPtScan test_sensor_get_pt_scan
#define Sensor_GetTcScan test_sensor_get_tc_scan
#define Sensor_GetScan test_sensor_get_scan
#define Sensor_GetPtRawAdc test_sensor_get_pt_raw_adc
#define Sensor_GetPtAdcVoltage test_sensor_get_pt_adc_voltage
#define Sensor_GetPtAdcMilliVolt test_sensor_get_pt_adc_millivolt
#define Sensor_GetTcMilliVolt test_sensor_get_tc_millivolt
#define Sensor_GetTcMicroVolt test_sensor_get_tc_microvolt
#include "../../../sam_ctl.X/iGRVT50/source/sensor.c"
#undef Sensor_UpdatePtRawAdcScan
#undef Sensor_GetPtScanCount
#undef Sensor_UpdateTcRawScan
#undef Sensor_GetTcScanCount
#undef Sensor_GetPtScan
#undef Sensor_GetTcScan
#undef Sensor_GetScan
#undef Sensor_GetPtRawAdc
#undef Sensor_GetPtAdcVoltage
#undef Sensor_GetPtAdcMilliVolt
#undef Sensor_GetTcMilliVolt
#undef Sensor_GetTcMicroVolt

void test_sensor_reset(void)
{
    UInt8 index;

    taskENTER_CRITICAL();
    for (index = 0U; index < SENSOR_PT_CHANNEL_COUNT; ++index) {
        s_ptRawAdc[index] = 0U;
    }
    for (index = 0U; index < SENSOR_TC_CHANNEL_COUNT; ++index) {
        s_tcRawCode[index] = 0;
    }
    s_ptScanCount = 0U;
    s_tcScanCount = 0U;
    taskEXIT_CRITICAL();
    tc_bypass = 1U;
    memset(&dependency_observations, 0, sizeof(dependency_observations));
}

void test_sensor_reset_dependency_observations(void)
{
    memset(&dependency_observations, 0, sizeof(dependency_observations));
}

void test_sensor_get_dependency_observations(
    test_sensor_dependency_observations_t *result)
{
    if (result != NULL) {
        *result = dependency_observations;
    }
}

void test_sensor_set_tc_bypass(UInt8 bypass)
{
    tc_bypass = bypass;
}

UInt32 test_sensor_peek_pt_scan_count(void)
{
    return s_ptScanCount;
}

UInt32 test_sensor_peek_tc_scan_count(void)
{
    return s_tcScanCount;
}

UInt16 test_sensor_peek_pt_raw(UInt8 index)
{
    return (index < SENSOR_PT_CHANNEL_COUNT) ? s_ptRawAdc[index] : 0U;
}

int32_t test_sensor_peek_tc_raw(UInt8 index)
{
    return (index < SENSOR_TC_CHANNEL_COUNT) ? s_tcRawCode[index] : 0;
}
