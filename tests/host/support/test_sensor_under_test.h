#ifndef TEST_SENSOR_UNDER_TEST_H
#define TEST_SENSOR_UNDER_TEST_H

#include "sensor.h"

#include <stddef.h>
#include <stdint.h>

typedef struct {
    size_t afec_conversion_calls;
    size_t tc_gain_calls;
    size_t conversion_calls_in_critical;
} test_sensor_dependency_observations_t;

void test_sensor_reset(void);
void test_sensor_reset_dependency_observations(void);
void test_sensor_get_dependency_observations(
    test_sensor_dependency_observations_t *observations);
void test_sensor_set_tc_bypass(UInt8 bypass);

void test_sensor_update_pt_raw_scan(const UInt16 *raw, UInt8 count);
UInt32 test_sensor_get_pt_scan_count(void);
void test_sensor_update_tc_raw_scan(const int32_t *raw, UInt8 count);
UInt32 test_sensor_get_tc_scan_count(void);
void test_sensor_get_scan(sSensorScan *scan);

UInt32 test_sensor_peek_pt_scan_count(void);
UInt32 test_sensor_peek_tc_scan_count(void);
UInt16 test_sensor_peek_pt_raw(UInt8 index);
int32_t test_sensor_peek_tc_raw(UInt8 index);

#endif
