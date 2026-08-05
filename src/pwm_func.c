/**
 * @file pwm_func.c
 * @author Heesung Shin (shs777@danam.co.kr)
 * @brief
 * @version 1.0.0
 * @date 2025-12-18
 *
 * @copyright Danam Systems Copyright (c) 2025
 */

/*==============================================================================
 * Include Files
 *============================================================================*/
/* --- includes --- */
 #include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes
#include <math.h>

/* --- FreeRTOS includes --- */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"

/* --- User includes --- */
#include "sam_ctl.h"

/*==============================================================================
 * Gloabal Variables
 *============================================================================*/

/*==============================================================================
 * Gloabal Function
 *============================================================================*/
void PWM_Init( void );
void PWM_SetPowerOn( void );
void PWM_SetPowerOff( void );
/*==============================================================================
 * Local Variables
 *============================================================================*/

/*==============================================================================
 * Local Function
 *============================================================================*/

/*==============================================================================
 * Functions
 *============================================================================*/


void PWM_SetPowerOn( void )
{
    /* PWM Start */
    PWM0_ChannelsStart(PWM_CHANNEL_0_MASK);
}

void PWM_SetPowerOff( void )
{
    /* PWM Start */
    PWM0_ChannelsStop(PWM_CHANNEL_0_MASK);
}

/* LP valve PWM: 150 MHz MCK / 7500 = 20 kHz. */
#define LP_PWM_CPRD    7500U

/* LP valves 9~12 use TC0 with TIMER_CLOCK2: 18.75 MHz / 938 ~= 20 kHz. */
#define LP_TC0_PERIOD  938U
#define LPV_PA15_MASK  (1UL << 15)
#define LPV_PA16_MASK  (1UL << 16)
#define LPV_PA26_MASK  (1UL << 26)
#define LPV_PA27_MASK  (1UL << 27)
#define LPV_TC0_PINS   (LPV_PA15_MASK | LPV_PA16_MASK | LPV_PA26_MASK | LPV_PA27_MASK)

void PWM_Init( void )
{
    UInt8 ucCh;
    UInt32 tc0cmr;

    /* LP밸브 PWM (lpv n 1=start, 0=stop). 채널 통일 + 데드타임 제거. duty 50%. */
    for( ucCh = 0; ucCh <= 3; ucCh++ )       /* PWM0 ch0~3 = lpv1~4 */
    {
        PWM0_REGS->PWM_CH_NUM[ucCh].PWM_CMR  = PWM_CMR_CPRE_MCK
                                             | PWM_CMR_CPOL_LOW_POLARITY | PWM_CMR_UPDS_UPDATE_AT_PERIOD
                                             | PWM_CMR_CES_SINGLE_EVENT;
        PWM0_REGS->PWM_CH_NUM[ucCh].PWM_CPRD = LP_PWM_CPRD;
        PWM0_REGS->PWM_CH_NUM[ucCh].PWM_CDTY = (LP_PWM_CPRD / 2U);
    }
    for( ucCh = 0; ucCh <= 3; ucCh++ )       /* [확장] PWM1 ch0~3 = lpv5~8 (기존 0~1 -> 0~3) */
    {
        PWM1_REGS->PWM_CH_NUM[ucCh].PWM_CMR  = PWM_CMR_CPRE_MCK
                                             | PWM_CMR_CPOL_LOW_POLARITY | PWM_CMR_UPDS_UPDATE_AT_PERIOD
                                             | PWM_CMR_CES_SINGLE_EVENT;
        PWM1_REGS->PWM_CH_NUM[ucCh].PWM_CPRD = LP_PWM_CPRD;
        PWM1_REGS->PWM_CH_NUM[ucCh].PWM_CDTY = (LP_PWM_CPRD / 2U);
    }

    /* [수정] lpv7=PA31(PWM1 PWMH2, periph D), lpv8=PA5(PWM1 PWML3, periph A) 핀을 PWM에 연결.
     * MCC가 ch2/3 핀을 PDR 안 해서 GPIO로 떠있어 stuck 28V였음. (A=00,B=01,C=10,D=11) */
    PIOA_REGS->PIO_ABCDSR[0] |=  (1UL << 31);   /* PA31 -> D: [0]=1 */
    PIOA_REGS->PIO_ABCDSR[1] |=  (1UL << 31);   /* PA31 -> D: [1]=1 */
    PIOA_REGS->PIO_ABCDSR[0] &= ~(1UL << 5);    /* PA5  -> A: [0]=0 */
    PIOA_REGS->PIO_ABCDSR[1] &= ~(1UL << 5);    /* PA5  -> A: [1]=0 */
    PIOA_REGS->PIO_PDR = (1UL << 31) | (1UL << 5);   /* peripheral 연결 */

    /* [수정] LPV8(PA5=PWM1 PWML3 = L출력=보수)만 채널 정지중 idle=HIGH(28V ON)!
     * 다른 LP채널(H출력)은 정지중 idle=LOW라 부팅 OFF지만, LPV8은 보수출력이라 부팅시 28V 고착됨.
     * -> ch3을 CDTY=0(L출력=0V)로 두고 즉시 start해서 부팅 OFF 보장 (LpValve_Set(8,1)과 동일효과). */
    PWM1_REGS->PWM_CH_NUM[3].PWM_CDTY = 0U;          /* L출력: cdty=0 -> H풀가동 -> L=0V(OFF) */
    PWM1_ChannelsStart( PWM_CHANNEL_3_MASK );        /* ch3만 start (ch0~2는 H출력이라 정지=OFF 유지) */

    /* [확장] lpv9~12 = TC0 ch1/ch2. PMC 클럭(ID24,25 -> PCER0) + PA핀 periph B + 부팅 OFF */
    PMC_REGS->PMC_PCER0 = (1UL << ID_TC0_CHANNEL1) | (1UL << ID_TC0_CHANNEL2);
    PIOA_REGS->PIO_ABCDSR[0] |=  LPV_TC0_PINS;     /* B = ABCDSR[0]=1, [1]=0 */
    PIOA_REGS->PIO_ABCDSR[1] &= ~LPV_TC0_PINS;
    tc0cmr = TC_CMR_TCCLKS_TIMER_CLOCK2 | TC_CMR_WAVE_Msk | TC_CMR_WAVEFORM_WAVSEL_UP_RC
           | TC_CMR_WAVEFORM_EEVT_XC0
           | TC_CMR_WAVEFORM_ACPA_CLEAR | TC_CMR_WAVEFORM_ACPC_SET
           | TC_CMR_WAVEFORM_BCPB_CLEAR | TC_CMR_WAVEFORM_BCPC_SET;
    TC0_REGS->TC_CHANNEL[1].TC_CMR = tc0cmr;  TC0_REGS->TC_CHANNEL[1].TC_RC = LP_TC0_PERIOD;
    TC0_REGS->TC_CHANNEL[1].TC_RA = 0U;        TC0_REGS->TC_CHANNEL[1].TC_RB = 0U;
    TC0_REGS->TC_CHANNEL[1].TC_CCR = TC_CCR_CLKEN_Msk | TC_CCR_SWTRG_Msk;
    TC0_REGS->TC_CHANNEL[2].TC_CMR = tc0cmr;  TC0_REGS->TC_CHANNEL[2].TC_RC = LP_TC0_PERIOD;
    TC0_REGS->TC_CHANNEL[2].TC_RA = 0U;        TC0_REGS->TC_CHANNEL[2].TC_RB = 0U;
    TC0_REGS->TC_CHANNEL[2].TC_CCR = TC_CCR_CLKEN_Msk | TC_CCR_SWTRG_Msk;
    PIOA_REGS->PIO_PER  = LPV_TC0_PINS;        /* 부팅 OFF: GPIO low */
    PIOA_REGS->PIO_OER  = LPV_TC0_PINS;
    PIOA_REGS->PIO_CODR = LPV_TC0_PINS;
}

/*==============================================================================
 * 히터 (TC3 ch9 Waveform/PWM, TIOA9=PE0=Heater1, TIOB9=PE1=Heater2)
 *  - MCC에 TC3 미생성 -> 레지스터로 직접 설정 (PMC 클럭 ID 50)
 *  - TIMER_CLOCK1 = MCK/2(75MHz), RC=7500 -> 10kHz, duty: RA(H1)/RB(H2)
 *============================================================================*/
#define HTR_TC_PERIOD   7500U       /* RC: PWM period count (MCK/8=18.75MHz -> ~2.5kHz) */
#define HTR_PE0_MASK    (1UL << 0)  /* PE0 = TIOA9  = Heater1 (TC3 ch0, peripheral B) */
#define HTR_PE1_MASK    (1UL << 1)  /* PE1 = TIOB9  = Heater2 (TC3 ch0, peripheral B) */
#define HTR_PE3_MASK    (1UL << 3)  /* PE3 = TIOA10 = Heater3      (TC3 ch1, peripheral B) */
#define HTR_PE4_MASK    (1UL << 4)  /* PE4 = TIOB10 = Heater4 (TC3 ch1, peripheral B) */
#define HTR_PC5_MASK    (1UL << 5)  /* PC5 = SP (GPIO ON/OFF, no PWM) */

/* duty 0%일 때 핀을 GPIO로 회수해 LOW 고정 -> 히터 OFF 보장.
 * (TC waveform에서 RA=0이면 TIOA가 H로 고착되는 엣지버그 회피) */
static void htr_pin_off( UInt32 mask )
{
    PIOE_REGS->PIO_PER  = mask;     /* PIO가 핀 제어 (peripheral 분리) */
    PIOE_REGS->PIO_OER  = mask;     /* 출력 */
    PIOE_REGS->PIO_CODR = mask;     /* LOW -> 게이트 L -> FET off -> 히터 off */
}
/* duty>0%: 핀을 다시 peripheral(TC3 TIOA/B)로 복귀 */
static void htr_pin_pwm( UInt32 mask )
{
    PIOE_REGS->PIO_PDR  = mask;     /* peripheral B(TIOA9/TIOB9) 복귀 */
}

void Heater_Init( void )
{
    UInt32 cmr =
          TC_CMR_TCCLKS_TIMER_CLOCK2                          /* MCK/8 (CLOCK1=PCK6는 미설정이라 카운터 안 돎!) */
        | TC_CMR_WAVE_Msk
        | TC_CMR_WAVEFORM_WAVSEL_UP_RC                        /* 0->RC up, auto reset */
        | TC_CMR_WAVEFORM_EEVT_XC0                            /* TIOB를 출력으로 사용 */
        | TC_CMR_WAVEFORM_ACPA_CLEAR | TC_CMR_WAVEFORM_ACPC_SET   /* TIOA: RA에서 L, RC에서 H */
        | TC_CMR_WAVEFORM_BCPB_CLEAR | TC_CMR_WAVEFORM_BCPC_SET;  /* TIOB: RB에서 L, RC에서 H */

    /* 1) PMC 클럭 enable [모두 PCER1]: TC3 ch0(ID50,bit18)+ch1(ID51,bit19), TC2 ch0(ID47,bit15)
     *  ※ID_TC2_CHANNEL0=47 (32 초과라 PCER1, bit=47-32=15). 1<<47은 오버플로 버그였음! */
    PMC_REGS->PMC_PCER1 = (1UL << (ID_TC3_CHANNEL0 - 32U))
                        | (1UL << (ID_TC3_CHANNEL0 + 1U - 32U))
                        | (1UL << (ID_TC2_CHANNEL0 - 32U));   /* TC2 ch0 kept idle while PC5 is SP GPIO */

    /* 1.5) PE3/PE4 = peripheral B (TC3 TIOA10/TIOB10).
     *  PE0/PE1은 MCC가 이미 B로 설정. (B = ABCDSR[0] bit=1, ABCDSR[1] bit=0) */
    PIOE_REGS->PIO_ABCDSR[0] |=  (HTR_PE3_MASK | HTR_PE4_MASK);
    PIOE_REGS->PIO_ABCDSR[1] &= ~(HTR_PE3_MASK | HTR_PE4_MASK);
    PIOC_REGS->PIO_ABCDSR[0] |=  HTR_PC5_MASK;
    PIOC_REGS->PIO_ABCDSR[1] &= ~HTR_PC5_MASK;

    /* 2) TC3 ch0(TIOA9=PE0=HTR1, TIOB9=PE1=HTR2) */
    TC3_REGS->TC_CHANNEL[0].TC_CMR = cmr;
    TC3_REGS->TC_CHANNEL[0].TC_RC = HTR_TC_PERIOD;
    TC3_REGS->TC_CHANNEL[0].TC_RA = 0U;     /* Heater1 duty 0 (off) */
    TC3_REGS->TC_CHANNEL[0].TC_RB = 0U;     /* Heater2 duty 0 (off) */
    TC3_REGS->TC_CHANNEL[0].TC_CCR = TC_CCR_CLKEN_Msk | TC_CCR_SWTRG_Msk;

    /* 2.5) TC3 ch1(TIOA10=PE3=HTR3, TIOB10=PE4=HTR4) — 동일 설정 */
    TC3_REGS->TC_CHANNEL[1].TC_CMR = cmr;
    TC3_REGS->TC_CHANNEL[1].TC_RC = HTR_TC_PERIOD;
    TC3_REGS->TC_CHANNEL[1].TC_RA = 0U;     /* Heater3 duty 0 (off) */
    TC3_REGS->TC_CHANNEL[1].TC_RB = 0U;     /* Heater4 duty 0 (off) */
    TC3_REGS->TC_CHANNEL[1].TC_CCR = TC_CCR_CLKEN_Msk | TC_CCR_SWTRG_Msk;

    /* 2.7) TC2 ch0 remains initialized but PC5 is assigned to SP GPIO control. */
    TC2_REGS->TC_CHANNEL[0].TC_CMR = cmr;
    TC2_REGS->TC_CHANNEL[0].TC_RC = HTR_TC_PERIOD;
    TC2_REGS->TC_CHANNEL[0].TC_RA = 0U;
    TC2_REGS->TC_CHANNEL[0].TC_CCR = TC_CCR_CLKEN_Msk | TC_CCR_SWTRG_Msk;

    /* 4) 부팅 시 Heater 1~4 + SP 모두 OFF 보장 (GPIO LOW 고정) */
    htr_pin_off( HTR_PE0_MASK | HTR_PE1_MASK | HTR_PE3_MASK | HTR_PE4_MASK );  /* PIOE: HTR1~4 */
    PIOC_REGS->PIO_PER  = HTR_PC5_MASK;     /* PIOC: SP(PC5) OFF */
    PIOC_REGS->PIO_OER  = HTR_PC5_MASK;
    PIOC_REGS->PIO_CODR = HTR_PC5_MASK;
}

/**
 * @brief 히터 PWM duty 설정 (검증용)
 * @param ucCh   1=Heater1(TIOA9), 2=Heater2(TIOB9)
 * @param ucPct  0~100 (%)
 */
void Heater_SetDuty( UInt8 ucCh, UInt8 ucPct )
{
    UInt32 uiDuty;
    UInt32 mask;
    UInt8  tcch;     /* TC3 채널: ch1·2 -> 0, ch3·4 -> 1 */
    UInt8  useA;     /* TIOA(RA)면 1, TIOB(RB)면 0 */

    if( ucPct > 100U ) ucPct = 100U;
    if( ucCh < 1U || ucCh > 4U ) { printf( "htr: ch 1~4\r\n" ); return; }

    /* ch1=ch0/A(PE0,TIOA9), ch2=ch0/B(PE1,TIOB9), ch3=ch1/A(PE3,TIOA10), ch4=ch1/B(PE4,TIOB10) */
    switch( ucCh )
    {
        case 1:  mask = HTR_PE0_MASK; tcch = 0U; useA = 1U; break;
        case 2:  mask = HTR_PE1_MASK; tcch = 0U; useA = 0U; break;
        case 3:  mask = HTR_PE3_MASK; tcch = 1U; useA = 1U; break;
        default: mask = HTR_PE4_MASK; tcch = 1U; useA = 0U; break;   /* ch4 */
    }

    if( ucPct == 0U )
    {
        htr_pin_off( mask );                        /* 0%: GPIO LOW 고정 -> 히터 OFF */
        printf( "Heater%d duty = 0%% (OFF, GPIO low)\r\n", ucCh );
        return;
    }

    htr_pin_pwm( mask );                             /* peripheral(TC3) 복귀 */
    if( ucPct >= 100U ) uiDuty = HTR_TC_PERIOD + 1U; /* 100%: RA>RC -> 항상 ON */
    else                uiDuty = ((UInt32)HTR_TC_PERIOD * ucPct) / 100U;

    if( useA ) TC3_REGS->TC_CHANNEL[tcch].TC_RA = uiDuty;   /* TIOA high-time */
    else       TC3_REGS->TC_CHANNEL[tcch].TC_RB = uiDuty;   /* TIOB high-time */
    printf( "Heater%d duty = %d%%\r\n", ucCh, ucPct );
}

void SparkPlug_Set( UInt8 on )
{
    TC2_REGS->TC_CHANNEL[0].TC_RA = 0U;
    PIOC_REGS->PIO_PER = HTR_PC5_MASK;
    PIOC_REGS->PIO_OER = HTR_PC5_MASK;

    if( on != 0U )
    {
        PIOC_REGS->PIO_SODR = HTR_PC5_MASK;
    }
    else
    {
        PIOC_REGS->PIO_CODR = HTR_PC5_MASK;
    }
}

/**
 * @brief LP 밸브 PWM 제어 (검증용)
 * @param ucCh  1~6 (LP_Valve01~06)
 * @param ucOn  1=출력 시작, 0=정지
 *  매핑: ch1~4 = PWM0 ch0~3, ch5~6 = PWM1 ch0~1
 */
/* LP밸브 RTN 저측(U9 TPD2017) 공통 enable = PD12(TPD2017_KILL_ALL).
 * 6채널 RTN을 한꺼번에 GND로 연결(=전류경로 형성). 이게 안 켜지면 부하전류 0(톱니).
 * 회로상 TPD2017 입력은 high에서 RTN enable로 사용한다. */
#define LPRTN_PD12  (1UL << 12)
void LpRtn_SetPin( UInt8 level )
{
    PIOD_REGS->PIO_PER  = LPRTN_PD12;   /* PIO 제어 (peripheral 분리) */
    PIOD_REGS->PIO_OER  = LPRTN_PD12;   /* 출력 */
    if( level ) { PIOD_REGS->PIO_SODR = LPRTN_PD12; }   /* HIGH */
    else        { PIOD_REGS->PIO_CODR = LPRTN_PD12; }   /* LOW  */
    printf( "PD12(TPD2017_KILL_ALL)=%d  [1=RTN enable / 0=RTN off]\r\n", level ? 1 : 0 );
}

void LpValve_Set( UInt8 ucCh, UInt8 ucOn )
{
    switch( ucCh )
    {
        case 1: ucOn ? PWM0_ChannelsStart(PWM_CHANNEL_0_MASK) : PWM0_ChannelsStop(PWM_CHANNEL_0_MASK); break;
        case 2: ucOn ? PWM0_ChannelsStart(PWM_CHANNEL_1_MASK) : PWM0_ChannelsStop(PWM_CHANNEL_1_MASK); break;
        case 3: ucOn ? PWM0_ChannelsStart(PWM_CHANNEL_2_MASK) : PWM0_ChannelsStop(PWM_CHANNEL_2_MASK); break;
        case 4: ucOn ? PWM0_ChannelsStart(PWM_CHANNEL_3_MASK) : PWM0_ChannelsStop(PWM_CHANNEL_3_MASK); break;
        case 5: ucOn ? PWM1_ChannelsStart(PWM_CHANNEL_0_MASK) : PWM1_ChannelsStop(PWM_CHANNEL_0_MASK); break;
        case 6: ucOn ? PWM1_ChannelsStart(PWM_CHANNEL_1_MASK) : PWM1_ChannelsStop(PWM_CHANNEL_1_MASK); break;
        case 7: ucOn ? PWM1_ChannelsStart(PWM_CHANNEL_2_MASK) : PWM1_ChannelsStop(PWM_CHANNEL_2_MASK); break;
        case 8: ucOn ? PWM1_ChannelsStart(PWM_CHANNEL_3_MASK) : PWM1_ChannelsStop(PWM_CHANNEL_3_MASK); break;
        case 9: case 10: case 11: case 12:   /* TC0 ch1/ch2 — RA/RB는 SetDuty가 설정, 여기선 핀 on/off */
        {
            UInt32 m = (ucCh==9U)?LPV_PA15_MASK:(ucCh==10U)?LPV_PA16_MASK:(ucCh==11U)?LPV_PA26_MASK:LPV_PA27_MASK;
            if( ucOn ) { PIOA_REGS->PIO_PDR = m; }                                  /* peripheral(TC0) */
            else       { PIOA_REGS->PIO_PER=m; PIOA_REGS->PIO_OER=m; PIOA_REGS->PIO_CODR=m; }  /* GPIO low */
            break;
        }
        default: printf("lpv: ch 1~12\r\n"); return;
    }
    printf("LP_Valve%02d = %d\r\n", ucCh, ucOn);
}

/*==============================================================================
 * Micro 밸브 전압제어 (사양: Lee Co. micro valve, Open=Peak 28V, Hold 2.5V)
 *  - PWM 듀티로 코일 평균전압 제어: Peak≈100%(28V), Hold≈9%(2.5/28V)
 *  - ch 1~4 -> PWM0 ch0~3, ch 5~6 -> PWM1 ch0~1. CPRD=150000.
 *  - pull-in 시간 경과 후 Hold로 전환 (호출측에서 시간 제어)
 *============================================================================*/
#define MV_CPRD       LP_PWM_CPRD
/* [수정] 출력 극성 반전: 실측상 ON(28V)시간 = (100-duty)% -> CDTY를 반전해서 정의.
 * duty%(28V 비율) -> CDTY = CPRD x (100-duty)/100. */
#define MV_PEAK_CDTY  (0U)                              /* duty100%(28V full) -> cdty 0 */
#define MV_HOLD_CDTY  ((MV_CPRD * (100U-9U)) / 100U)    /* duty9%(2.5V hold) -> cdty 91% */

static void mv_set_cdty( UInt8 ucCh, UInt32 cdty, UInt8 useUpd )
{
    /* useUpd=1: 동작중 글리치없는 갱신(CDTYUPD) / 0: 시작 전 직접(CDTY) */
    switch( ucCh )
    {
        case 1: if(useUpd){PWM0_REGS->PWM_CH_NUM[0].PWM_CDTYUPD=cdty;}else{PWM0_REGS->PWM_CH_NUM[0].PWM_CDTY=cdty;} break;
        case 2: if(useUpd){PWM0_REGS->PWM_CH_NUM[1].PWM_CDTYUPD=cdty;}else{PWM0_REGS->PWM_CH_NUM[1].PWM_CDTY=cdty;} break;
        case 3: if(useUpd){PWM0_REGS->PWM_CH_NUM[2].PWM_CDTYUPD=cdty;}else{PWM0_REGS->PWM_CH_NUM[2].PWM_CDTY=cdty;} break;
        case 4: if(useUpd){PWM0_REGS->PWM_CH_NUM[3].PWM_CDTYUPD=cdty;}else{PWM0_REGS->PWM_CH_NUM[3].PWM_CDTY=cdty;} break;
        case 5: if(useUpd){PWM1_REGS->PWM_CH_NUM[0].PWM_CDTYUPD=cdty;}else{PWM1_REGS->PWM_CH_NUM[0].PWM_CDTY=cdty;} break;
        case 6: if(useUpd){PWM1_REGS->PWM_CH_NUM[1].PWM_CDTYUPD=cdty;}else{PWM1_REGS->PWM_CH_NUM[1].PWM_CDTY=cdty;} break;
        case 7: if(useUpd){PWM1_REGS->PWM_CH_NUM[2].PWM_CDTYUPD=cdty;}else{PWM1_REGS->PWM_CH_NUM[2].PWM_CDTY=cdty;} break;
        case 8: if(useUpd){PWM1_REGS->PWM_CH_NUM[3].PWM_CDTYUPD=cdty;}else{PWM1_REGS->PWM_CH_NUM[3].PWM_CDTY=cdty;} break;
        default: break;
    }
}

void MicroValve_Open( UInt8 ucCh )     /* peak 듀티 세팅 + 채널 start */
{
    if( ucCh < 1U || ucCh > 6U ) { printf("mv: ch 1~6\r\n"); return; }
    mv_set_cdty( ucCh, MV_PEAK_CDTY, 0U );   /* 시작 전 peak 직접 */
    LpValve_Set( ucCh, 1U );
}

void MicroValve_Hold( UInt8 ucCh )     /* hold 듀티로 전환 (동작중) */
{
    if( ucCh < 1U || ucCh > 6U ) return;
    mv_set_cdty( ucCh, MV_HOLD_CDTY, 1U );
}

void MicroValve_Close( UInt8 ucCh )
{
    if( ucCh < 1U || ucCh > 6U ) return;
    LpValve_Set( ucCh, 0U );
    mv_set_cdty( ucCh, 0U, 0U );
}

/* [보정용] LP밸브 PWM 듀티 직접 설정 (10% 단위). 평균전압 = 28V x duty%.
 * 듀티를 바꿔가며 실측 -> 전압 경향 확인 -> 역보정 테이블 작성용. */
void MicroValve_SetDuty( UInt8 ucCh, UInt8 ucPct )
{
    UInt32 cdty;
    if( ucCh < 1U || ucCh > 12U ) { printf("lpv: ch 1~12\r\n"); return; }
    if( ucPct > 100U ) ucPct = 100U;   /* 1% 단위 */

    if( ucCh >= 9U )    /* [확장] lpv9~12 = TC0 ch1/ch2 (RA/RB 반전 = PWM과 동일 극성: 28V=pct%) */
    {
        UInt32 m; UInt8 tcch; UInt8 useA; UInt32 ra;
        switch( ucCh )
        {
            case 9:  m=LPV_PA15_MASK; tcch=1U; useA=1U; break;   /* TIOA1 */
            case 10: m=LPV_PA16_MASK; tcch=1U; useA=0U; break;   /* TIOB1 */
            case 11: m=LPV_PA26_MASK; tcch=2U; useA=1U; break;   /* TIOA2 */
            default: m=LPV_PA27_MASK; tcch=2U; useA=0U; break;   /* ch12 TIOB2 */
        }
        if( ucPct == 0U )
        {
            PIOA_REGS->PIO_PER=m; PIOA_REGS->PIO_OER=m; PIOA_REGS->PIO_CODR=m;   /* GPIO low = OFF */
        }
        else
        {
            ra = (LP_TC0_PERIOD * (100U - (UInt32)ucPct)) / 100U;   /* 반전(PWM CDTY와 동일) */
            if( useA ) TC0_REGS->TC_CHANNEL[tcch].TC_RA = ra;
            else       TC0_REGS->TC_CHANNEL[tcch].TC_RB = ra;
            PIOA_REGS->PIO_PDR = m;                                 /* peripheral B (TC0) */
        }
        printf("LP_Valve%02d duty=%d%% (TC0)\r\n", ucCh, ucPct);
        return;
    }

    if( ucCh == 8U )    /* [수정] lpv8 = PA5 = PWM1 PWML3 (L출력 = H의 보수). 극성 반대라 보정:
                         * 듀티 정방향(반전 안함) + 채널 항상 running(정지하면 L이 28V 고착). */
    {
        cdty = (MV_CPRD * (UInt32)ucPct) / 100U;        /* 정방향: cdty=0 -> H on -> L off(0V), cdty=max -> L 28V */
        mv_set_cdty( 8U, cdty, 0U );
        mv_set_cdty( 8U, cdty, 1U );
        LpValve_Set( 8U, 1U );                          /* 항상 running (0%여도 정지 금지) */
        printf( "LP_Valve08 duty=%d%% (L출력 극성보정)\r\n", ucPct );
        return;
    }

    /* lpv1~7 = PWM0/PWM1 (CDTY 반전) */
    cdty = (MV_CPRD * (100U - (UInt32)ucPct)) / 100U;
    mv_set_cdty( ucCh, cdty, 0U );
    mv_set_cdty( ucCh, cdty, 1U );
    if( ucPct == 0U ) { LpValve_Set( ucCh, 0U ); }
    else              { LpValve_Set( ucCh, 1U ); }
}

/*******************************************************************************
 End of File
*/
