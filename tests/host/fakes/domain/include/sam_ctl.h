#ifndef TEST_FAKE_SAM_CTL_H
#define TEST_FAKE_SAM_CTL_H

#include <stdint.h>

typedef uint8_t UInt8;
typedef int8_t SInt8;
typedef uint16_t UInt16;
typedef int16_t SInt16;
typedef uint32_t UInt32;
typedef int32_t SInt32;

void Heater_SetDuty(UInt8 ch, UInt8 dutyPct);
void SparkPlug_Set(UInt8 on);

#endif
