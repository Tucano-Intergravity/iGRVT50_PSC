#include "sensor.h"
#include "psc_io_map.h"

#define SENSOR_TC_DEV_1         1U
#define SENSOR_TC_GAIN_BYPASS   1.0f
#define SENSOR_TC_GAIN_PGA32    32.0f
#define SENSOR_PT_ZERO_MV       500.0f
#define SENSOR_PT_SPAN_MV       4000.0f
#define SENSOR_PT_FULL_SCALE_BAR 100.0f
#define SENSOR_PT_C1_FULL_SCALE_BAR 16.0f
#define SENSOR_PT_MILLIBAR_PER_BAR 1000.0f
#define SENSOR_PT_DIVIDER_TOP_OHM 20000.0f
#define SENSOR_PT_DIVIDER_BOTTOM_OHM (78700.0f / 2.0f)
#define SENSOR_PT_FRONTEND_GAIN \
    ((SENSOR_PT_DIVIDER_TOP_OHM + SENSOR_PT_DIVIDER_BOTTOM_OHM) / \
     SENSOR_PT_DIVIDER_BOTTOM_OHM)
#define SENSOR_KELVIN_OFFSET_C      273.15f
#define SENSOR_TC_VALID_MAX_MK      2500000L

static volatile UInt16 s_ptRawAdc[SENSOR_PT_CHANNEL_COUNT] = { 0U };
static volatile UInt32 s_ptScanCount = 0U;
static volatile int32_t s_tcRawCode[SENSOR_TC_CHANNEL_COUNT] = { 0 };
static volatile UInt32 s_tcScanCount = 0U;
static volatile SInt32 s_tcMilliKelvin[SENSOR_TC_CHANNEL_COUNT] = { 0 };
static volatile UInt16 s_tcTemperatureValidMask = 0U;
static volatile UInt32 s_tcTemperatureScanCount = 0U;
static volatile UInt8 s_sensorOverrideEnabled = 0U;
static volatile SInt32 s_overridePtMilliBar[SENSOR_PT_CHANNEL_COUNT] = { 0 };
static volatile SInt32 s_overrideTcMilliKelvin[SENSOR_TC_CHANNEL_COUNT] = { 0 };
static volatile UInt16 s_overrideTcTemperatureValidMask = 0U;

static SInt32 Sensor_RoundToSInt32( float value )
{
    return (SInt32)( value + ((value >= 0.0f) ? 0.5f : -0.5f) );
}

static float Sensor_GetPtFullScaleBar( UInt8 index )
{
    return (index == (UInt8)PSC_IO_INDEX( PSC_PT_C1 ))
        ? SENSOR_PT_C1_FULL_SCALE_BAR
        : SENSOR_PT_FULL_SCALE_BAR;
}

static SInt32 Sensor_ConvertPtMilliVoltToMilliBar( SInt32 millivolt, UInt8 index )
{
    float pressureBar;

    pressureBar = (((float)millivolt - SENSOR_PT_ZERO_MV) *
        Sensor_GetPtFullScaleBar( index )) / SENSOR_PT_SPAN_MV;
    return Sensor_RoundToSInt32( pressureBar * SENSOR_PT_MILLIBAR_PER_BAR );
}

static SInt32 Sensor_ConvertPtAdcMilliVoltToSensorMilliVolt( SInt32 adcMilliVolt )
{
    return Sensor_RoundToSInt32( (float)adcMilliVolt * SENSOR_PT_FRONTEND_GAIN );
}

static SInt32 Sensor_ConvertPtMilliBarToMilliVolt( SInt32 milliBar, UInt8 index )
{
    float pressureBar;
    float millivolt;

    pressureBar = (float)milliBar / SENSOR_PT_MILLIBAR_PER_BAR;
    millivolt = SENSOR_PT_ZERO_MV +
        ((pressureBar * SENSOR_PT_SPAN_MV) / Sensor_GetPtFullScaleBar( index ));
    return Sensor_RoundToSInt32( millivolt );
}

static SInt32 Sensor_ConvertOverrideKelvinToMicroVolt( SInt32 milliKelvin, UInt8 index )
{
    float tempC;
    float milliVolt;

    if( index == SENSOR_TC_CJC1_INDEX )
    {
        return 0;
    }

    tempC = ((float)milliKelvin / 1000.0f) - SENSOR_KELVIN_OFFSET_C;
    milliVolt = ADS1263_TypeKTempToMilliVolt( tempC );
    if( milliVolt != milliVolt )
    {
        return 0;
    }
    return Sensor_RoundToSInt32( milliVolt * 1000.0f );
}

static UInt8 Sensor_ChannelToIndex( UInt8 ch, UInt8 count )
{
    if( ch == 0U )
    {
        return 0U;
    }
    if( ch > count )
    {
        return (UInt8)( count - 1U );
    }
    return (UInt8)( ch - 1U );
}

static UInt8 Sensor_IsValidKelvin( float value )
{
    if( value != value )
    {
        return 0U;
    }
    if( (value <= 0.0f) || (value > 2500.0f) )
    {
        return 0U;
    }
    return 1U;
}

static float Sensor_GetTcGain( void )
{
    return (ADS1263_GetBypass() != 0U) ? SENSOR_TC_GAIN_BYPASS : SENSOR_TC_GAIN_PGA32;
}

void Sensor_UpdatePtRawAdcScan( const UInt16 *pRawAdc, UInt8 count )
{
    UInt8 i;

    if( s_sensorOverrideEnabled != 0U )
    {
        return;
    }

    if( pRawAdc == (const UInt16 *)0 )
    {
        return;
    }

    if( count > SENSOR_PT_CHANNEL_COUNT )
    {
        count = SENSOR_PT_CHANNEL_COUNT;
    }

    for( i = 0U; i < count; i++ )
    {
        s_ptRawAdc[i] = pRawAdc[i];
    }

    s_ptScanCount++;
}

UInt32 Sensor_GetPtScanCount( void )
{
    return (s_sensorOverrideEnabled != 0U) ? 1U : s_ptScanCount;
}

void Sensor_UpdateTcRawScan( const int32_t *pRawCode, UInt8 count )
{
    UInt8 i;

    if( s_sensorOverrideEnabled != 0U )
    {
        return;
    }

    if( pRawCode == (const int32_t *)0 )
    {
        return;
    }

    if( count > SENSOR_TC_CHANNEL_COUNT )
    {
        count = SENSOR_TC_CHANNEL_COUNT;
    }

    for( i = 0U; i < count; i++ )
    {
        s_tcRawCode[i] = pRawCode[i];
    }

    s_tcScanCount++;
}

void Sensor_UpdateTcTemperatureScan( const float *pKelvin, UInt8 count )
{
    UInt8 i;
    UInt16 validMask = 0U;

    if( s_sensorOverrideEnabled != 0U )
    {
        return;
    }

    if( pKelvin == (const float *)0 )
    {
        return;
    }

    if( count > SENSOR_TC_CHANNEL_COUNT )
    {
        count = SENSOR_TC_CHANNEL_COUNT;
    }

    for( i = 0U; i < count; i++ )
    {
        if( Sensor_IsValidKelvin( pKelvin[i] ) != 0U )
        {
            s_tcMilliKelvin[i] = Sensor_RoundToSInt32( pKelvin[i] * 1000.0f );
            validMask |= (UInt16)(1U << i);
        }
        else
        {
            s_tcMilliKelvin[i] = 0;
        }
    }

    for( ; i < SENSOR_TC_CHANNEL_COUNT; i++ )
    {
        s_tcMilliKelvin[i] = 0;
    }

    s_tcTemperatureValidMask = validMask;
    s_tcTemperatureScanCount++;
}

void SensorOverride_StartFromCurrent( void )
{
    UInt8 i;
    UInt16 raw;
    SInt32 milliVolt;

    for( i = 0U; i < SENSOR_PT_CHANNEL_COUNT; i++ )
    {
        raw = s_ptRawAdc[i];
        milliVolt = Sensor_ConvertPtAdcMilliVoltToSensorMilliVolt(
            Sensor_RoundToSInt32( AFEC_ToVoltage( raw ) * 1000.0f ) );
        s_overridePtMilliBar[i] = Sensor_ConvertPtMilliVoltToMilliBar( milliVolt, i );
    }

    for( i = 0U; i < SENSOR_TC_CHANNEL_COUNT; i++ )
    {
        s_overrideTcMilliKelvin[i] = s_tcMilliKelvin[i];
    }

    s_overrideTcTemperatureValidMask = s_tcTemperatureValidMask;
    s_sensorOverrideEnabled = 1U;
}

void SensorOverride_Stop( void )
{
    s_sensorOverrideEnabled = 0U;
}

UInt8 SensorOverride_IsEnabled( void )
{
    return s_sensorOverrideEnabled;
}

void SensorOverride_SetValues( const SInt32 *pPtMilliBar,
                               const SInt32 *pTcMilliKelvin,
                               UInt8 ptCount,
                               UInt8 tcCount )
{
    UInt8 i;
    UInt16 validMask = 0U;

    if( pPtMilliBar != (const SInt32 *)0 )
    {
        if( ptCount > SENSOR_PT_CHANNEL_COUNT )
        {
            ptCount = SENSOR_PT_CHANNEL_COUNT;
        }
        for( i = 0U; i < ptCount; i++ )
        {
            s_overridePtMilliBar[i] = pPtMilliBar[i];
        }
    }

    if( pTcMilliKelvin != (const SInt32 *)0 )
    {
        if( tcCount > SENSOR_TC_CHANNEL_COUNT )
        {
            tcCount = SENSOR_TC_CHANNEL_COUNT;
        }
        for( i = 0U; i < tcCount; i++ )
        {
            s_overrideTcMilliKelvin[i] = pTcMilliKelvin[i];
            if( (pTcMilliKelvin[i] > 0) &&
                (pTcMilliKelvin[i] <= SENSOR_TC_VALID_MAX_MK) )
            {
                validMask |= (UInt16)(1U << i);
            }
        }
        s_overrideTcTemperatureValidMask = validMask;
    }
}

UInt32 Sensor_GetTcScanCount( void )
{
    return (s_sensorOverrideEnabled != 0U) ? 1U : s_tcScanCount;
}

UInt32 Sensor_GetTcTemperatureScanCount( void )
{
    return (s_sensorOverrideEnabled != 0U) ? 1U : s_tcTemperatureScanCount;
}

UInt16 Sensor_GetPtRawAdc( UInt8 ch )
{
    UInt8 idx = Sensor_ChannelToIndex( ch, SENSOR_PT_CHANNEL_COUNT );

    if( s_sensorOverrideEnabled != 0U )
    {
        return 0U;
    }
    return s_ptRawAdc[idx];
}

float Sensor_GetPtAdcVoltage( UInt8 ch )
{
    UInt8 idx = Sensor_ChannelToIndex( ch, SENSOR_PT_CHANNEL_COUNT );

    if( s_sensorOverrideEnabled != 0U )
    {
        return (float)Sensor_ConvertPtMilliBarToMilliVolt( s_overridePtMilliBar[idx], idx ) / 1000.0f;
    }
    return (float)Sensor_GetPtAdcMilliVolt( ch ) / 1000.0f;
}

SInt32 Sensor_GetPtAdcMilliVolt( UInt8 ch )
{
    UInt8 idx = Sensor_ChannelToIndex( ch, SENSOR_PT_CHANNEL_COUNT );

    if( s_sensorOverrideEnabled != 0U )
    {
        return Sensor_ConvertPtMilliBarToMilliVolt( s_overridePtMilliBar[idx], idx );
    }
    return Sensor_ConvertPtAdcMilliVoltToSensorMilliVolt(
        Sensor_RoundToSInt32( AFEC_ToVoltage( Sensor_GetPtRawAdc( ch ) ) *
                              1000.0f ) );
}

SInt32 Sensor_GetPtPressureMilliBar( UInt8 ch )
{
    UInt8 idx = Sensor_ChannelToIndex( ch, SENSOR_PT_CHANNEL_COUNT );

    if( s_sensorOverrideEnabled != 0U )
    {
        return s_overridePtMilliBar[idx];
    }
    return Sensor_ConvertPtMilliVoltToMilliBar( Sensor_GetPtAdcMilliVolt( ch ), idx );
}

void Sensor_GetPtScan( sSensorPtScan *pScan )
{
    UInt8 i;
    UInt16 raw;

    if( pScan == (sSensorPtScan *)0 )
    {
        return;
    }

    for( i = 0U; i < SENSOR_PT_CHANNEL_COUNT; i++ )
    {
        if( s_sensorOverrideEnabled != 0U )
        {
            pScan->rawAdc[i] = 0U;
            pScan->adcMilliVolt[i] =
                Sensor_ConvertPtMilliBarToMilliVolt( s_overridePtMilliBar[i], i );
            pScan->adcVoltage[i] = (float)pScan->adcMilliVolt[i] / 1000.0f;
            pScan->pressureMilliBar[i] = s_overridePtMilliBar[i];
            continue;
        }
        raw = s_ptRawAdc[i];
        pScan->rawAdc[i] = raw;
        pScan->adcMilliVolt[i] = Sensor_ConvertPtAdcMilliVoltToSensorMilliVolt(
            Sensor_RoundToSInt32( AFEC_ToVoltage( raw ) * 1000.0f ) );
        pScan->adcVoltage[i] = (float)pScan->adcMilliVolt[i] / 1000.0f;
        pScan->pressureMilliBar[i] =
            Sensor_ConvertPtMilliVoltToMilliBar( pScan->adcMilliVolt[i], i );
    }
}

float Sensor_GetTcMilliVolt( UInt8 ch )
{
    UInt8 idx = Sensor_ChannelToIndex( ch, SENSOR_TC_CHANNEL_COUNT );
    int32_t code = s_tcRawCode[idx];

    if( s_sensorOverrideEnabled != 0U )
    {
        return (float)Sensor_ConvertOverrideKelvinToMicroVolt( s_overrideTcMilliKelvin[idx], idx ) / 1000.0f;
    }
    return (((float)code * VREF) / (Sensor_GetTcGain() * ADC_FS)) * 1000.0f;
}

SInt32 Sensor_GetTcMicroVolt( UInt8 ch )
{
    return Sensor_RoundToSInt32( Sensor_GetTcMilliVolt( ch ) * 1000.0f );
}

void Sensor_GetTcScan( sSensorTcScan *pScan )
{
    UInt8 i;
    float gain = Sensor_GetTcGain();

    if( pScan == (sSensorTcScan *)0 )
    {
        return;
    }

    for( i = 0U; i < SENSOR_TC_CHANNEL_COUNT; i++ )
    {
        if( s_sensorOverrideEnabled != 0U )
        {
            pScan->rawCode[i] = 0;
            pScan->microVolt[i] =
                Sensor_ConvertOverrideKelvinToMicroVolt( s_overrideTcMilliKelvin[i], i );
            pScan->milliVolt[i] = (float)pScan->microVolt[i] / 1000.0f;
            continue;
        }
        pScan->rawCode[i] = s_tcRawCode[i];
        pScan->milliVolt[i] = (((float)pScan->rawCode[i] * VREF) / (gain * ADC_FS)) * 1000.0f;
        pScan->microVolt[i] = Sensor_RoundToSInt32( pScan->milliVolt[i] * 1000.0f );
    }
}

void Sensor_GetTcTemperatureScan( sSensorTcTemperatureScan *pScan )
{
    UInt8 i;

    if( pScan == (sSensorTcTemperatureScan *)0 )
    {
        return;
    }

    for( i = 0U; i < SENSOR_TC_CHANNEL_COUNT; i++ )
    {
        if( s_sensorOverrideEnabled != 0U )
        {
            pScan->milliKelvin[i] = s_overrideTcMilliKelvin[i];
            continue;
        }
        pScan->milliKelvin[i] = s_tcMilliKelvin[i];
    }
    pScan->validMask = (s_sensorOverrideEnabled != 0U)
        ? s_overrideTcTemperatureValidMask
        : s_tcTemperatureValidMask;
}

void Sensor_GetScan( sSensorScan *pScan )
{
    if( pScan == (sSensorScan *)0 )
    {
        return;
    }

    Sensor_GetPtScan( &pScan->pt );
    Sensor_GetTcScan( &pScan->tc );
}
