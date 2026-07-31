#include "sensor.h"

#define SENSOR_TC_DEV_1         1U
#define SENSOR_TC_GAIN_BYPASS   1.0f
#define SENSOR_TC_GAIN_PGA32    32.0f

static volatile UInt16 s_ptRawAdc[SENSOR_PT_CHANNEL_COUNT] = { 0U };
static volatile UInt32 s_ptScanCount = 0U;

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

void Sensor_UpdatePtRawAdcScan( const UInt16 *pRawAdc, UInt8 count )
{
    UInt8 i;

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
    return s_ptScanCount;
}

UInt16 Sensor_GetPtRawAdc( UInt8 ch )
{
    UInt8 idx = Sensor_ChannelToIndex( ch, SENSOR_PT_CHANNEL_COUNT );

    return s_ptRawAdc[idx];
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
    UInt16 raw;

    if( pScan == (sSensorPtScan *)0 )
    {
        return;
    }

    for( i = 0U; i < SENSOR_PT_CHANNEL_COUNT; i++ )
    {
        raw = s_ptRawAdc[i];
        pScan->rawAdc[i] = raw;
        pScan->adcVoltage[i] = AFEC_ToVoltage( raw );
        pScan->adcMilliVolt[i] = Sensor_RoundToSInt32( pScan->adcVoltage[i] * 1000.0f );
    }
}

float Sensor_GetTcMilliVolt( UInt8 ch )
{
    UInt8 idx = Sensor_ChannelToIndex( ch, SENSOR_TC_CHANNEL_COUNT );
    int32_t code = ADS1263_GetRawCode( SENSOR_TC_DEV_1, idx );

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
        pScan->rawCode[i] = ADS1263_GetRawCode( SENSOR_TC_DEV_1, i );
        pScan->milliVolt[i] = (((float)pScan->rawCode[i] * VREF) / (gain * ADC_FS)) * 1000.0f;
        pScan->microVolt[i] = Sensor_RoundToSInt32( pScan->milliVolt[i] * 1000.0f );
    }
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
