#ifndef TEST_FAKE_SAM_CTL_H
#define TEST_FAKE_SAM_CTL_H

#include <stdint.h>

typedef uint8_t UInt8;
typedef int8_t SInt8;
typedef uint16_t UInt16;
typedef int16_t SInt16;
typedef uint32_t UInt32;
typedef int32_t SInt32;

#define VREF 2.5f
#define ADC_FS 2147483648.0f

typedef struct {
    float fTempCh1;
    float fTempCh2;
    float fTempCh3;
    float fTempCh4;
    float fTempCJ;
} sTcTemp;

float AFEC_ToVoltage(UInt16 adc_value);
float ADS1263_GetTemperatureTask(UInt8 ch);
int32_t ADS1263_GetRawCode(UInt8 dev, UInt8 ch);
UInt8 ADS1263_GetBypass(void);
void Heater_SetDuty(UInt8 ch, UInt8 dutyPct);
void SparkPlug_Set(UInt8 on);

#endif
