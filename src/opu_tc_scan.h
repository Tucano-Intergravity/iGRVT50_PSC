#ifndef OPU_TC_SCAN_H
#define OPU_TC_SCAN_H

#include <sam_ctl.h>
#include <sensor.h>

#include <stdint.h>

static inline void OpuTcScan_AcquireAndPublish(sTcTemp *temperature)
{
    int32_t raw[SENSOR_TC_CHANNEL_COUNT];

    temperature->fTempCh1 = ADS1263_GetTemperatureTask(0U);
    temperature->fTempCh2 = ADS1263_GetTemperatureTask(1U);
    temperature->fTempCh3 = ADS1263_GetTemperatureTask(2U);
    temperature->fTempCh4 = ADS1263_GetTemperatureTask(3U);
    raw[0] = ADS1263_GetRawCode(1U, 0U);
    raw[1] = ADS1263_GetRawCode(1U, 1U);
    raw[2] = ADS1263_GetRawCode(1U, 2U);
    raw[3] = ADS1263_GetRawCode(1U, 3U);
    Sensor_UpdateTcRawScan(raw, SENSOR_TC_CHANNEL_COUNT);
}

#endif
