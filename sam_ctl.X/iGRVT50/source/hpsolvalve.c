#include "hpsolvalve.h"

#define HPSV1_DRV_NODE              0U
#define HPSV1_CHCTRL_CURRENT_REG    0x2U
#define HPSV1_CURRENT_800MA_REG     56U

static volatile UInt8 s_hpsv1On = 0U;
static volatile UInt8 s_hpsv1Configured = 0U;

static UInt8 HpSolValve_IsValidChannel( UInt8 ch )
{
    return (ch == HPSOLVALVE_SV1) ? 1U : 0U;
}

static void HpSolValve_SelectSv1Node( void )
{
    DRV3946_SetNode( HPSV1_DRV_NODE );
}

static void HpSolValve_ApplySv1CurrentConfig( void )
{
    UInt16 status0 = 0U;

    HpSolValve_SelectSv1Node();
    g_drvPC[0] = HPSV1_CURRENT_800MA_REG;
    g_drvHC[0] = HPSV1_CURRENT_800MA_REG;
    (void)DRV3946_Wake( &status0 );
    s_hpsv1Configured = 1U;
}

void HpSolValve_Init( void )
{
    DRV3946Q1_EN1_OutputEnable();
    DRV3946Q1_EN2_OutputEnable();
    DRV3946Q1_KILL_ALL_OutputEnable();

    HpSolValve_ApplySv1CurrentConfig();
    HpSolValve_Set( HPSOLVALVE_SV1, 0U );
}

void HpSolValve_Set( UInt8 ch, UInt8 on )
{
    UInt8 echo = 0U;
    UInt16 status0;

    if( HpSolValve_IsValidChannel( ch ) == 0U )
    {
        return;
    }

    HpSolValve_SelectSv1Node();

    if( on != 0U )
    {
        status0 = DRV3946_Read24( 0x01U, HPSV1_DRV_NODE, &echo );
        if( (s_hpsv1Configured == 0U) || ((status0 & 0x2000U) != 0U) || (status0 == 0xFFFFU) )
        {
            HpSolValve_ApplySv1CurrentConfig();
        }

        DRV3946Q1_KILL_ALL_Set();
        DRV3946Q1_EN2_Clear();
        DRV3946Q1_EN1_Set();
        (void)DRV3946_ChCtrl( HPSV1_CHCTRL_CURRENT_REG, 0U );
        (void)DRV3946_Read24( 0x01U, HPSV1_DRV_NODE, &echo );
        s_hpsv1On = 1U;
    }
    else
    {
        DRV3946Q1_EN1_Clear();
        (void)DRV3946_ChCtrl( 0U, 0U );
        (void)DRV3946_Read24( 0x01U, HPSV1_DRV_NODE, &echo );
        s_hpsv1On = 0U;
    }
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
    if( HpSolValve_IsValidChannel( ch ) == 0U )
    {
        return 0U;
    }

    return s_hpsv1On;
}
