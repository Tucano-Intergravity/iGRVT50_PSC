#include "hpsolvalve.h"

#define HPSV_DRV_NODE_COUNT          4U
#define HPSV_CH_PER_NODE             2U
#define HPSV_CHCTRL_CURRENT_REG      0x2U
#define HPSV_CURRENT_800MA_REG       56U
#define HPSV_CURRENT_HOLD_MIN_REG    0U
#define HPSV_DEFAULT_PEAK_MA         800U
#define HPSV_DEFAULT_HOLD_MA         100U
#define HPSV_CURRENT_SCALE_MA        3000UL
#define HPSV_CURRENT_SCALE_DEN       272UL
#define HPSV_CURRENT_OFFSET_REG      17UL
#define HPSV_CURRENT_REG_MAX         255UL

static volatile UInt8 s_hpsvOn[HPSOLVALVE_CHANNEL_COUNT] = { 0U };
static volatile UInt8 s_hpsvNodeConfigured[HPSV_DRV_NODE_COUNT] = { 0U };
static UInt8 s_hpsvPeakReg[HPSOLVALVE_CHANNEL_COUNT] = {
    HPSV_CURRENT_800MA_REG, HPSV_CURRENT_800MA_REG,
    HPSV_CURRENT_800MA_REG, HPSV_CURRENT_800MA_REG,
    HPSV_CURRENT_800MA_REG, HPSV_CURRENT_800MA_REG,
    HPSV_CURRENT_800MA_REG, HPSV_CURRENT_800MA_REG
};
static UInt8 s_hpsvHoldReg[HPSOLVALVE_CHANNEL_COUNT] = {
    HPSV_CURRENT_HOLD_MIN_REG, HPSV_CURRENT_HOLD_MIN_REG,
    HPSV_CURRENT_HOLD_MIN_REG, HPSV_CURRENT_HOLD_MIN_REG,
    HPSV_CURRENT_HOLD_MIN_REG, HPSV_CURRENT_HOLD_MIN_REG,
    HPSV_CURRENT_HOLD_MIN_REG, HPSV_CURRENT_HOLD_MIN_REG
};

static UInt8 HpSolValve_IsValidChannel( UInt8 ch )
{
    return ((ch >= 1U) && (ch <= HPSOLVALVE_CHANNEL_COUNT)) ? 1U : 0U;
}

static UInt8 HpSolValve_ChannelToIndex( UInt8 ch )
{
    return (UInt8)(ch - 1U);
}

static UInt8 HpSolValve_IndexToNode( UInt8 idx )
{
    return (UInt8)(idx / HPSV_CH_PER_NODE);
}

static UInt8 HpSolValve_IndexToNodeChannel( UInt8 idx )
{
    return (UInt8)(idx % HPSV_CH_PER_NODE);
}

static UInt8 HpSolValve_MilliAmpToReg( UInt16 milliAmp )
{
    UInt32 scaled;

    scaled = ((((UInt32)milliAmp) * HPSV_CURRENT_SCALE_DEN) + (HPSV_CURRENT_SCALE_MA / 2UL)) /
             HPSV_CURRENT_SCALE_MA;
    if( scaled <= HPSV_CURRENT_OFFSET_REG )
    {
        return 0U;
    }

    scaled -= HPSV_CURRENT_OFFSET_REG;
    if( scaled > HPSV_CURRENT_REG_MAX )
    {
        scaled = HPSV_CURRENT_REG_MAX;
    }

    return (UInt8)scaled;
}

static UInt16 HpSolValve_RegToMilliAmp( UInt8 reg )
{
    UInt32 milliAmp;

    milliAmp = ((((UInt32)reg + HPSV_CURRENT_OFFSET_REG) * HPSV_CURRENT_SCALE_MA) +
                (HPSV_CURRENT_SCALE_DEN / 2UL)) / HPSV_CURRENT_SCALE_DEN;

    return (UInt16)milliAmp;
}

static void HpSolValve_UpdateEnablePins( void )
{
    UInt8 i;
    UInt8 ch1On = 0U;
    UInt8 ch2On = 0U;

    for( i = 0U; i < HPSOLVALVE_CHANNEL_COUNT; i++ )
    {
        if( s_hpsvOn[i] != 0U )
        {
            if( HpSolValve_IndexToNodeChannel( i ) == 0U )
            {
                ch1On = 1U;
            }
            else
            {
                ch2On = 1U;
            }
        }
    }

    if( (ch1On != 0U) || (ch2On != 0U) )
    {
        DRV3946Q1_KILL_ALL_Set();
    }

    if( ch1On != 0U ) { DRV3946Q1_EN1_Set(); }
    else              { DRV3946Q1_EN1_Clear(); }

    if( ch2On != 0U ) { DRV3946Q1_EN2_Set(); }
    else              { DRV3946Q1_EN2_Clear(); }
}

static void HpSolValve_ApplyNodeCurrentConfig( UInt8 node )
{
    UInt8 firstIdx;
    UInt16 status0 = 0U;

    if( node >= HPSV_DRV_NODE_COUNT )
    {
        return;
    }

    firstIdx = (UInt8)(node * HPSV_CH_PER_NODE);
    DRV3946_SetNode( node );
    g_drvPC[0] = s_hpsvPeakReg[firstIdx];
    g_drvHC[0] = s_hpsvHoldReg[firstIdx];
    g_drvPC[1] = s_hpsvPeakReg[firstIdx + 1U];
    g_drvHC[1] = s_hpsvHoldReg[firstIdx + 1U];
    (void)DRV3946_Wake( &status0 );
    s_hpsvNodeConfigured[node] = 1U;
}

static void HpSolValve_EnsureNodeConfigured( UInt8 node )
{
    UInt8 echo = 0U;
    UInt16 status0;

    if( node >= HPSV_DRV_NODE_COUNT )
    {
        return;
    }

    DRV3946_SetNode( node );
    status0 = DRV3946_Read24( 0x01U, node, &echo );
    if( (s_hpsvNodeConfigured[node] == 0U) ||
        ((status0 & 0x2000U) != 0U) ||
        (status0 == 0xFFFFU) )
    {
        HpSolValve_ApplyNodeCurrentConfig( node );
    }
}

static void HpSolValve_ApplyNodeOutput( UInt8 node )
{
    UInt8 echo = 0U;
    UInt8 firstIdx;
    UInt8 ch1Ctrl;
    UInt8 ch2Ctrl;

    if( node >= HPSV_DRV_NODE_COUNT )
    {
        return;
    }

    firstIdx = (UInt8)(node * HPSV_CH_PER_NODE);
    ch1Ctrl = (s_hpsvOn[firstIdx] != 0U) ? HPSV_CHCTRL_CURRENT_REG : 0U;
    ch2Ctrl = (s_hpsvOn[firstIdx + 1U] != 0U) ? HPSV_CHCTRL_CURRENT_REG : 0U;

    DRV3946_SetNode( node );
    (void)DRV3946_ChCtrl( ch1Ctrl, ch2Ctrl );
    (void)DRV3946_Read24( 0x01U, node, &echo );
}

void HpSolValve_Init( void )
{
    UInt8 node;

    DRV3946Q1_EN1_OutputEnable();
    DRV3946Q1_EN2_OutputEnable();
    DRV3946Q1_KILL_ALL_OutputEnable();
    DRV3946Q1_EN1_Clear();
    DRV3946Q1_EN2_Clear();
    DRV3946Q1_KILL_ALL_Set();

    for( node = 0U; node < HPSV_DRV_NODE_COUNT; node++ )
    {
        s_hpsvOn[node * HPSV_CH_PER_NODE] = 0U;
        s_hpsvOn[(node * HPSV_CH_PER_NODE) + 1U] = 0U;
        s_hpsvNodeConfigured[node] = 0U;
        HpSolValve_ApplyNodeCurrentConfig( node );
        HpSolValve_ApplyNodeOutput( node );
    }

    HpSolValve_UpdateEnablePins();
}

void HpSolValve_Set( UInt8 ch, UInt8 on )
{
    UInt8 idx;
    UInt8 node;

    if( HpSolValve_IsValidChannel( ch ) == 0U )
    {
        return;
    }

    if( on != 0U )
    {
        HpSolValve_SetPeakHoldMilliAmp( ch, HPSV_DEFAULT_PEAK_MA, HPSV_DEFAULT_HOLD_MA );
        return;
    }

    idx = HpSolValve_ChannelToIndex( ch );
    node = HpSolValve_IndexToNode( idx );
    if( s_hpsvOn[idx] == 0U )
    {
        return;
    }

    s_hpsvOn[idx] = 0U;
    HpSolValve_ApplyNodeOutput( node );
    HpSolValve_UpdateEnablePins();
}

void HpSolValve_SetPeakHoldMilliAmp( UInt8 ch, UInt16 peakMilliAmp, UInt16 holdMilliAmp )
{
    UInt8 idx;
    UInt8 node;
    UInt8 peakReg;
    UInt8 holdReg;

    if( HpSolValve_IsValidChannel( ch ) == 0U )
    {
        return;
    }

    if( peakMilliAmp == 0U )
    {
        HpSolValve_Set( ch, 0U );
        return;
    }

    idx = HpSolValve_ChannelToIndex( ch );
    node = HpSolValve_IndexToNode( idx );
    peakReg = HpSolValve_MilliAmpToReg( peakMilliAmp );
    holdReg = HpSolValve_MilliAmpToReg( holdMilliAmp );

    if( (s_hpsvOn[idx] != 0U) &&
        (s_hpsvPeakReg[idx] == peakReg) &&
        (s_hpsvHoldReg[idx] == holdReg) &&
        (s_hpsvNodeConfigured[node] != 0U) )
    {
        return;
    }

    if( (s_hpsvPeakReg[idx] != peakReg) || (s_hpsvHoldReg[idx] != holdReg) )
    {
        s_hpsvPeakReg[idx] = peakReg;
        s_hpsvHoldReg[idx] = holdReg;
        s_hpsvNodeConfigured[node] = 0U;
    }

    HpSolValve_EnsureNodeConfigured( node );
    s_hpsvOn[idx] = 1U;
    HpSolValve_UpdateEnablePins();
    HpSolValve_ApplyNodeOutput( node );
}

void HpSolValve_Toggle( UInt8 ch )
{
    if( HpSolValve_IsOn( ch ) != 0U )
    {
        HpSolValve_Set( ch, 0U );
    }
    else
    {
        HpSolValve_Set( ch, 1U );
    }
}

UInt8 HpSolValve_IsOn( UInt8 ch )
{
    UInt8 idx;

    if( HpSolValve_IsValidChannel( ch ) == 0U )
    {
        return 0U;
    }

    idx = HpSolValve_ChannelToIndex( ch );

    return s_hpsvOn[idx];
}

UInt16 HpSolValve_GetConfiguredPeakMilliAmp( UInt8 ch )
{
    UInt8 idx;

    if( HpSolValve_IsValidChannel( ch ) == 0U )
    {
        return 0U;
    }

    idx = HpSolValve_ChannelToIndex( ch );

    return HpSolValve_RegToMilliAmp( s_hpsvPeakReg[idx] );
}

UInt16 HpSolValve_GetConfiguredHoldMilliAmp( UInt8 ch )
{
    UInt8 idx;

    if( HpSolValve_IsValidChannel( ch ) == 0U )
    {
        return 0U;
    }

    idx = HpSolValve_ChannelToIndex( ch );

    return HpSolValve_RegToMilliAmp( s_hpsvHoldReg[idx] );
}
