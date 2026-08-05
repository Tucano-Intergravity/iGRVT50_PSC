#include "lpsolvalve.h"

#define LPSV1_PA0_MASK          (1UL << 0)
#define LPSV_RTN_PD12_MASK      (1UL << 12)
#define LPSV1_PWM_PERIOD        7500U
#define LPSV1_PWM_MODE          (PWM_CMR_CPRE_MCK | PWM_CMR_CPOL_LOW_POLARITY | \
                                 PWM_CMR_UPDS_UPDATE_AT_PERIOD | PWM_CMR_CES_SINGLE_EVENT)
#define LPSV_OPEN_DUTY_PCT      100U
#define LPSV_HOLD_DUTY_PCT      20U
#define LPSV_OPEN_HOLD_TICKS    2U

typedef enum LpSolValveState {
    LPSV_STATE_OFF = 0U,
    LPSV_STATE_OPEN,
    LPSV_STATE_HOLD
} eLpSolValveState;

static volatile UInt8 s_lpsvDutyPct[LPSOLVALVE_CHANNEL_COUNT] = { 0U };
static volatile eLpSolValveState s_lpsvState[LPSOLVALVE_CHANNEL_COUNT] = { LPSV_STATE_OFF };
static volatile UInt8 s_lpsvOpenTicks[LPSOLVALVE_CHANNEL_COUNT] = { 0U };

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

static void LpSolValve_ApplyDuty( UInt8 ch, UInt8 dutyPct )
{
    UInt32 cdty;
    UInt8 idx;

    idx = (UInt8)(ch - 1U);

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

    PWM0_REGS->PWM_CH_NUM[0].PWM_CMR = LPSV1_PWM_MODE;
    PWM0_REGS->PWM_CH_NUM[0].PWM_CPRD = LPSV1_PWM_PERIOD;
    PWM0_REGS->PWM_CH_NUM[0].PWM_CDTY = cdty;
    PWM0_REGS->PWM_CH_NUM[0].PWM_CDTYUPD = cdty;
    LpSolValve_SetReturnEnable( 1U );
    LpSolValve_SetSv1PinPwm();
    PWM0_ChannelsStart( PWM_CHANNEL_0_MASK );

    s_lpsvDutyPct[idx] = dutyPct;
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
    UInt8 idx;

    if( LpSolValve_IsValidChannel( ch ) == 0U )
    {
        return;
    }

    idx = (UInt8)(ch - 1U);

    if( on != 0U )
    {
        if( s_lpsvState[idx] != LPSV_STATE_OFF )
        {
            return;
        }

        s_lpsvState[idx] = LPSV_STATE_OPEN;
        s_lpsvOpenTicks[idx] = LPSV_OPEN_HOLD_TICKS;
        LpSolValve_ApplyDuty( ch, LPSV_OPEN_DUTY_PCT );
    }
    else
    {
        s_lpsvState[idx] = LPSV_STATE_OFF;
        s_lpsvOpenTicks[idx] = 0U;
        LpSolValve_ApplyDuty( ch, 0U );
    }
}

void LpSolValve_SetDuty( UInt8 ch, UInt8 dutyPct )
{
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

    s_lpsvState[idx] = (dutyPct == 0U) ? LPSV_STATE_OFF : LPSV_STATE_HOLD;
    s_lpsvOpenTicks[idx] = 0U;
    LpSolValve_ApplyDuty( ch, dutyPct );
}

void LpSolValve_Service10ms( void )
{
    UInt8 idx;
    UInt8 ch;

    for( idx = 0U; idx < LPSOLVALVE_CHANNEL_COUNT; idx++ )
    {
        if( s_lpsvState[idx] != LPSV_STATE_OPEN )
        {
            continue;
        }

        if( s_lpsvOpenTicks[idx] != 0U )
        {
            s_lpsvOpenTicks[idx]--;
        }

        if( s_lpsvOpenTicks[idx] == 0U )
        {
            s_lpsvState[idx] = LPSV_STATE_HOLD;
            ch = (UInt8)(idx + 1U);
            LpSolValve_ApplyDuty( ch, LPSV_HOLD_DUTY_PCT );
        }
    }
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
