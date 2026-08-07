#include "sensor.h"

#include "FreeRTOS.h"
#include "task.h"

#define SENSOR_TC_GAIN_BYPASS   1.0f
#define SENSOR_TC_GAIN_PGA32    32.0f

static volatile UInt16 s_ptRawAdc[SENSOR_PT_CHANNEL_COUNT] = { 0U };
static volatile UInt32 s_ptScanCount = 0U;
static volatile int32_t s_tcRawCode[SENSOR_TC_CHANNEL_COUNT] = { 0 };
static volatile UInt32 s_tcScanCount = 0U;

static SInt32 Sensor_RoundToSInt32( float value )
{
    return (SInt32)( value + ((value >= 0.0f) ? 0.5f : -0.5f) );
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

static float Sensor_GetTcGain( void )
{
    return (ADS1263_GetBypass() != 0U) ? SENSOR_TC_GAIN_BYPASS : SENSOR_TC_GAIN_PGA32;
}

static float Sensor_TcCodeToMilliVolt( int32_t code, float gain )
{
    return (((float)code * VREF) / (gain * ADC_FS)) * 1000.0f;
}

static void Sensor_FillPtScan( const UInt16 *pRawAdc, sSensorPtScan *pScan )
{
    UInt8 i;

    for( i = 0U; i < SENSOR_PT_CHANNEL_COUNT; i++ )
    {
        pScan->rawAdc[i] = pRawAdc[i];
        pScan->adcVoltage[i] = AFEC_ToVoltage( pRawAdc[i] );
        pScan->adcMilliVolt[i] = Sensor_RoundToSInt32( pScan->adcVoltage[i] * 1000.0f );
    }
}

static void Sensor_FillTcScan( const int32_t *pRawCode, sSensorTcScan *pScan )
{
    UInt8 i;
    float gain = Sensor_GetTcGain();

    for( i = 0U; i < SENSOR_TC_CHANNEL_COUNT; i++ )
    {
        pScan->rawCode[i] = pRawCode[i];
        pScan->milliVolt[i] = Sensor_TcCodeToMilliVolt( pRawCode[i], gain );
        pScan->microVolt[i] = Sensor_RoundToSInt32( pScan->milliVolt[i] * 1000.0f );
    }
}

void Sensor_UpdatePtRawAdcScan( const UInt16 *pRawAdc, UInt8 count )
{
    UInt8 i;

    if( (pRawAdc == (const UInt16 *)0) || (count < SENSOR_PT_CHANNEL_COUNT) )
    {
        return;
    }

    taskENTER_CRITICAL();
    for( i = 0U; i < SENSOR_PT_CHANNEL_COUNT; i++ )
    {
        s_ptRawAdc[i] = pRawAdc[i];
    }
    s_ptScanCount++;
    taskEXIT_CRITICAL();
}

UInt32 Sensor_GetPtScanCount( void )
{
    UInt32 count;

    taskENTER_CRITICAL();
    count = s_ptScanCount;
    taskEXIT_CRITICAL();
    return count;
}

void Sensor_UpdateTcRawScan( const int32_t *pRawCode, UInt8 count )
{
    UInt8 i;

    if( (pRawCode == (const int32_t *)0) || (count < SENSOR_TC_CHANNEL_COUNT) )
    {
        return;
    }

    taskENTER_CRITICAL();
    for( i = 0U; i < SENSOR_TC_CHANNEL_COUNT; i++ )
    {
        s_tcRawCode[i] = pRawCode[i];
    }
    s_tcScanCount++;
    taskEXIT_CRITICAL();
}

UInt32 Sensor_GetTcScanCount( void )
{
    UInt32 count;

    taskENTER_CRITICAL();
    count = s_tcScanCount;
    taskEXIT_CRITICAL();
    return count;
}

UInt16 Sensor_GetPtRawAdc( UInt8 ch )
{
    UInt8 idx = Sensor_ChannelToIndex( ch, SENSOR_PT_CHANNEL_COUNT );
    UInt16 raw;

    taskENTER_CRITICAL();
    raw = s_ptRawAdc[idx];
    taskEXIT_CRITICAL();
    return raw;
}

float Sensor_GetPtAdcVoltage( UInt8 ch )
{
    return AFEC_ToVoltage( Sensor_GetPtRawAdc( ch ) );
}

SInt32 Sensor_GetPtAdcMilliVolt( UInt8 ch )
{
    return Sensor_RoundToSInt32( Sensor_GetPtAdcVoltage( ch ) * 1000.0f );
}

void Sensor_GetPtScan( sSensorPtScan *pScan )
{
    UInt8 i;
    UInt16 raw[SENSOR_PT_CHANNEL_COUNT];

    if( pScan == (sSensorPtScan *)0 )
    {
        return;
    }

    taskENTER_CRITICAL();
    for( i = 0U; i < SENSOR_PT_CHANNEL_COUNT; i++ )
    {
        raw[i] = s_ptRawAdc[i];
    }
    taskEXIT_CRITICAL();

    Sensor_FillPtScan( raw, pScan );
}

float Sensor_GetTcMilliVolt( UInt8 ch )
{
    UInt8 idx = Sensor_ChannelToIndex( ch, SENSOR_TC_CHANNEL_COUNT );
    int32_t code;

    taskENTER_CRITICAL();
    code = s_tcRawCode[idx];
    taskEXIT_CRITICAL();

    return Sensor_TcCodeToMilliVolt( code, Sensor_GetTcGain() );
}

SInt32 Sensor_GetTcMicroVolt( UInt8 ch )
{
    return Sensor_RoundToSInt32( Sensor_GetTcMilliVolt( ch ) * 1000.0f );
}

void Sensor_GetTcScan( sSensorTcScan *pScan )
{
    UInt8 i;
    int32_t raw[SENSOR_TC_CHANNEL_COUNT];

    if( pScan == (sSensorTcScan *)0 )
    {
        return;
    }

    taskENTER_CRITICAL();
    for( i = 0U; i < SENSOR_TC_CHANNEL_COUNT; i++ )
    {
        raw[i] = s_tcRawCode[i];
    }
    taskEXIT_CRITICAL();

    Sensor_FillTcScan( raw, pScan );
}

void Sensor_GetScan( sSensorScan *pScan )
{
    UInt8 i;
    UInt16 ptRaw[SENSOR_PT_CHANNEL_COUNT];
    int32_t tcRaw[SENSOR_TC_CHANNEL_COUNT];

    if( pScan == (sSensorScan *)0 )
    {
        return;
    }

    taskENTER_CRITICAL();
    for( i = 0U; i < SENSOR_PT_CHANNEL_COUNT; i++ )
    {
        ptRaw[i] = s_ptRawAdc[i];
    }
    for( i = 0U; i < SENSOR_TC_CHANNEL_COUNT; i++ )
    {
        tcRaw[i] = s_tcRawCode[i];
    }
    taskEXIT_CRITICAL();

    Sensor_FillPtScan( ptRaw, &pScan->pt );
    Sensor_FillTcScan( tcRaw, &pScan->tc );
}
