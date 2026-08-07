#include "fake_opu_tc_scan.h"

#include "sam_ctl.h"

#include <string.h>

static fake_opu_tc_scan_observations_t observations;
static float temperatures[SENSOR_TC_CHANNEL_COUNT];
static int32_t raw_codes[SENSOR_TC_CHANNEL_COUNT];

static void record_event(
    fake_opu_tc_event_type_t type,
    uint8_t device,
    uint8_t channel)
{
    if (observations.event_count < FAKE_OPU_TC_SCAN_MAX_EVENTS) {
        fake_opu_tc_event_t *event =
            &observations.events[observations.event_count];
        event->type = type;
        event->device = device;
        event->channel = channel;
    }
    ++observations.event_count;
}

void fake_opu_tc_scan_reset(void)
{
    memset(&observations, 0, sizeof(observations));
    memset(temperatures, 0, sizeof(temperatures));
    memset(raw_codes, 0, sizeof(raw_codes));
}

void fake_opu_tc_scan_set_temperature(uint8_t channel, float value)
{
    if (channel < SENSOR_TC_CHANNEL_COUNT) {
        temperatures[channel] = value;
    }
}

void fake_opu_tc_scan_set_raw_code(uint8_t channel, int32_t value)
{
    if (channel < SENSOR_TC_CHANNEL_COUNT) {
        raw_codes[channel] = value;
    }
}

void fake_opu_tc_scan_get_observations(
    fake_opu_tc_scan_observations_t *result)
{
    if (result != NULL) {
        *result = observations;
    }
}

float ADS1263_GetTemperatureTask(UInt8 channel)
{
    record_event(FAKE_OPU_TC_EVENT_TEMPERATURE, 1U, channel);
    ++observations.temperature_calls;
    return (channel < SENSOR_TC_CHANNEL_COUNT) ? temperatures[channel] : 0.0f;
}

int32_t ADS1263_GetRawCode(UInt8 device, UInt8 channel)
{
    record_event(FAKE_OPU_TC_EVENT_RAW_CODE, device, channel);
    ++observations.raw_code_calls;
    return (channel < SENSOR_TC_CHANNEL_COUNT) ? raw_codes[channel] : 0;
}

void Sensor_UpdateTcRawScan(const int32_t *raw, UInt8 count)
{
    record_event(FAKE_OPU_TC_EVENT_PUBLISH, 0U, 0U);
    ++observations.publish_calls;
    observations.published_count = count;
    if (raw != NULL) {
        memcpy(observations.published_raw, raw, sizeof(observations.published_raw));
    }
}
