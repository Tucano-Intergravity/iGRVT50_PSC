#include "lpsolvalve.h"

#define LPSV1_PA0_MASK          (1UL << 0)
#define LPSV_RTN_PD12_MASK      (1UL << 12)
#define LPSV1_PWM_PERIOD        3750U

static volatile UInt8 s_lpsvDutyPct[LPSOLVALVE_CHANNEL_COUNT] = { 0U };

static UInt8 LpSolValve_IsValidChannel( UInt8 ch )
{
    return ((ch >= 1U) && (ch <= LPSOLVALVE_CHANNEL_COUNT)) ? 1U : 0U;
}

static void LpSolValve_SetSv1PinLow( void )
{
    PWM0_ChannelsStop( PWM_CHANNEL_0_MASK );
    PWM0_REGS->PWM_CH_NUM[0].PWM_CDTY = 0U;

    PIOA_REGS->PIO_PER  = LPSV1_PA0_MASK;
    PIOA_REGS->PIO_OER  = LPSV1_PA0_MASK;
    PIOA_REGS->PIO_CODR = LPSV1_PA0_MASK;
}

static void LpSolValve_SetSv1PinPwm( void )
{
    PIOA_REGS->PIO_ABCDSR[0] &= ~LPSV1_PA0_MASK;
    PIOA_REGS->PIO_ABCDSR[1] &= ~LPSV1_PA0_MASK;
    PIOA_REGS->PIO_PDR = LPSV1_PA0_MASK;
}

static void LpSolValve_SetReturnEnable( UInt8 enable )
{
    PIOD_REGS->PIO_PER = LPSV_RTN_PD12_MASK;
    PIOD_REGS->PIO_OER = LPSV_RTN_PD12_MASK;

    if( enable != 0U )
    {
        PIOD_REGS->PIO_SODR = LPSV_RTN_PD12_MASK;
    }
    else
    {
        PIOD_REGS->PIO_CODR = LPSV_RTN_PD12_MASK;
    }
}

static void LpSolValve_UpdateReturnEnable( void )
{
    UInt8 i;

    for( i = 0U; i < LPSOLVALVE_CHANNEL_COUNT; i++ )
    {
        if( s_lpsvDutyPct[i] != 0U )
        {
            LpSolValve_SetReturnEnable( 1U );
            return;
        }
    }

    LpSolValve_SetReturnEnable( 0U );
}

void LpSolValve_Init( void )
{
    UInt8 ch;

    for( ch = 1U; ch <= LPSOLVALVE_CHANNEL_COUNT; ch++ )
    {
        LpSolValve_Set( ch, 0U );
    }
}

void LpSolValve_Set( UInt8 ch, UInt8 on )
{
    if( on != 0U )
    {
        LpSolValve_SetDuty( ch, 100U );
    }
    else
    {
        LpSolValve_SetDuty( ch, 0U );
    }
}

void LpSolValve_SetDuty( UInt8 ch, UInt8 dutyPct )
{
    UInt32 cdty;
    UInt8 idx;

    if( LpSolValve_IsValidChannel( ch ) == 0U )
    {
        return;
    }

    idx = (UInt8)(ch - 1U);

    if( dutyPct > 100U )
    {
        dutyPct = 100U;
    }

    if( ch != LPSOLVALVE_SV1 )
    {
        MicroValve_SetDuty( ch, dutyPct );
        s_lpsvDutyPct[idx] = dutyPct;
        LpSolValve_UpdateReturnEnable();
        return;
    }

    if( dutyPct == 0U )
    {
        LpSolValve_SetSv1PinLow();
        s_lpsvDutyPct[idx] = 0U;
        LpSolValve_UpdateReturnEnable();
        return;
    }

    cdty = (LPSV1_PWM_PERIOD * (UInt32)(100U - dutyPct)) / 100U;

    PWM0_REGS->PWM_CH_NUM[0].PWM_CPRD = LPSV1_PWM_PERIOD;
    PWM0_REGS->PWM_CH_NUM[0].PWM_CDTY = cdty;
    PWM0_REGS->PWM_CH_NUM[0].PWM_CDTYUPD = cdty;
    LpSolValve_SetReturnEnable( 1U );
    LpSolValve_SetSv1PinPwm();
    PWM0_ChannelsStart( PWM_CHANNEL_0_MASK );

    s_lpsvDutyPct[idx] = dutyPct;
}

void LpSolValve_Toggle( UInt8 ch )
{
    if( LpSolValve_IsOn( ch ) != 0U )
    {
        LpSolValve_Set( ch, 0U );
    }
    else
    {
        LpSolValve_Set( ch, 1U );
    }
}

UInt8 LpSolValve_IsOn( UInt8 ch )
{
    UInt8 idx;

    if( LpSolValve_IsValidChannel( ch ) == 0U )
    {
        return 0U;
    }

    idx = (UInt8)(ch - 1U);

    return (s_lpsvDutyPct[idx] != 0U) ? 1U : 0U;
}

UInt8 LpSolValve_GetDuty( UInt8 ch )
{
    UInt8 idx;

    if( LpSolValve_IsValidChannel( ch ) == 0U )
    {
        return 0U;
    }

    idx = (UInt8)(ch - 1U);

    return s_lpsvDutyPct[idx];
}
