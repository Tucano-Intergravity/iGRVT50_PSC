#ifndef TEST_FAKE_OPU_TC_SCAN_H
#define TEST_FAKE_OPU_TC_SCAN_H

#include "sensor.h"

#include <stddef.h>
#include <stdint.h>

#define FAKE_OPU_TC_SCAN_MAX_EVENTS 9U

typedef enum {
    FAKE_OPU_TC_EVENT_TEMPERATURE = 0,
    FAKE_OPU_TC_EVENT_RAW_CODE,
    FAKE_OPU_TC_EVENT_PUBLISH,
} fake_opu_tc_event_type_t;

typedef struct {
    fake_opu_tc_event_type_t type;
    uint8_t device;
    uint8_t channel;
} fake_opu_tc_event_t;

typedef struct {
    fake_opu_tc_event_t events[FAKE_OPU_TC_SCAN_MAX_EVENTS];
    size_t event_count;
    size_t temperature_calls;
    size_t raw_code_calls;
    size_t publish_calls;
    int32_t published_raw[SENSOR_TC_CHANNEL_COUNT];
    uint8_t published_count;
} fake_opu_tc_scan_observations_t;

void fake_opu_tc_scan_reset(void);
void fake_opu_tc_scan_set_temperature(uint8_t channel, float value);
void fake_opu_tc_scan_set_raw_code(uint8_t channel, int32_t value);
void fake_opu_tc_scan_get_observations(
    fake_opu_tc_scan_observations_t *observations);

#endif
