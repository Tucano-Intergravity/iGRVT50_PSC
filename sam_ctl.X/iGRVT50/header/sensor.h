#ifndef IGRVT50_SENSOR_H
#define IGRVT50_SENSOR_H

#include "sam_ctl.h"

#define SENSOR_PT_CHANNEL_COUNT     9U
#define SENSOR_TC_CHANNEL_COUNT     5U
#define SENSOR_PT1_INDEX            0U
#define SENSOR_TC1_INDEX            0U
#define SENSOR_TC_CJC1_INDEX        4U

typedef struct SensorPtScan {
    UInt16 rawAdc[SENSOR_PT_CHANNEL_COUNT];
    float adcVoltage[SENSOR_PT_CHANNEL_COUNT];
    SInt32 adcMilliVolt[SENSOR_PT_CHANNEL_COUNT];
    SInt32 pressureMilliBar[SENSOR_PT_CHANNEL_COUNT];
} sSensorPtScan;

typedef struct SensorTcScan {
    int32_t rawCode[SENSOR_TC_CHANNEL_COUNT];
    float milliVolt[SENSOR_TC_CHANNEL_COUNT];
    SInt32 microVolt[SENSOR_TC_CHANNEL_COUNT];
} sSensorTcScan;

typedef struct SensorTcTemperatureScan {
    SInt32 milliKelvin[SENSOR_TC_CHANNEL_COUNT];
    UInt16 validMask;
} sSensorTcTemperatureScan;

typedef struct SensorScan {
    sSensorPtScan pt;
    sSensorTcScan tc;
} sSensorScan;

void Sensor_UpdatePtRawAdcScan( const UInt16 *pRawAdc, UInt8 count );
void Sensor_UpdateTcRawScan( const int32_t *pRawCode, UInt8 count );
void Sensor_UpdateTcTemperatureScan( const float *pKelvin, UInt8 count );
void SensorOverride_StartFromCurrent( void );
void SensorOverride_Stop( void );
UInt8 SensorOverride_IsEnabled( void );
void SensorOverride_SetValues( const SInt32 *pPtMilliBar,
                               const SInt32 *pTcMilliKelvin,
                               UInt8 ptCount,
                               UInt8 tcCount );
UInt32 Sensor_GetPtScanCount( void );
UInt32 Sensor_GetTcScanCount( void );
UInt32 Sensor_GetTcTemperatureScanCount( void );
void Sensor_GetPtScan( sSensorPtScan *pScan );
void Sensor_GetTcScan( sSensorTcScan *pScan );
void Sensor_GetTcTemperatureScan( sSensorTcTemperatureScan *pScan );
void Sensor_GetScan( sSensorScan *pScan );

UInt16 Sensor_GetPtRawAdc( UInt8 ch );
float Sensor_GetPtAdcVoltage( UInt8 ch );
SInt32 Sensor_GetPtAdcMilliVolt( UInt8 ch );
SInt32 Sensor_GetPtPressureMilliBar( UInt8 ch );

float Sensor_GetTcMilliVolt( UInt8 ch );
SInt32 Sensor_GetTcMicroVolt( UInt8 ch );

#endif /* IGRVT50_SENSOR_H */
