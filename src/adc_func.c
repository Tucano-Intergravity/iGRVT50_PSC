/**
 * @file adc_func.c
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


/* [수정] 회로도 확인: MCU ADC 입력단엔 분배기 없음(직렬R+필터캡만).
 * 분배/게인은 앞단 LMV324 op-amp에서 끝남 -> 핀전압 = 센스네트(주입)전압 1:1.
 * 따라서 게인 1.0 = 표시값이 ADC 핀 실전압(=주입전압)과 동일. (1V 주입 -> 1.00 표시)
 * 실 물리량(28V/압력) 환산은 채널별 op-amp 전달함수 적용이 필요(별도 보정). */
#define ADC_GAIN  (1.0f)

/*==============================================================================
 * Gloabal Variables
 *============================================================================*/

/*==============================================================================
 * Gloabal Function
 *============================================================================*/
float AFEC_Init( void );
float AFEC_ToVoltage( UInt16 adc_value );
UInt16 ReadAFEC0Channel( AFEC_CHANNEL_NUM channel );
UInt16 ReadAFEC1Channel( AFEC_CHANNEL_NUM channel );

/*==============================================================================
 * Local Variables
 *============================================================================*/

/*==============================================================================
 * Local Function
 *============================================================================*/

/*==============================================================================
 * Functions
 *============================================================================*/

float AFEC_ToVoltage( UInt16 adc_value )
{
    const float fVrefAdc = 3.4f;   /* [보정] 실측 VREFP=3.4V (VREFN을 GND로 수정 후) */
    float fret;

    fret = ((adc_value * fVrefAdc) / 4095.0f);
    fret = fret*ADC_GAIN;

    return fret;
}

/* [데이터시트 정석] COCR(AOFF)는 init에서 한 번만 512로 설정(유휴 중 갱신 금지!).
 * 읽기는 그룹당 1회 START 후 채널별 EOC 대기 -> CDR. COCR/SWRST 절대 안 건드림.
 * (유휴 중 COCR 갱신하면 다음 변환이 틀림 = 그동안의 튐/혼선 원인) */
/* FREERUN 연속변환 중이므로 수동 START 불필요 (no-op). 읽기는 CDR만. */
void AFEC0_SeqConvert( UInt32 chMask ) { (void)chMask; }
void AFEC1_SeqConvert( UInt32 chMask ) { (void)chMask; }

UInt16 ReadAFEC0Channel( AFEC_CHANNEL_NUM channel )
{
    UInt32 to = 1000000U;
    while( ((AFEC0_REGS->AFEC_ISR & (1UL << (uint32_t)channel)) == 0U) && (--to > 0U) ) { }
    AFEC0_REGS->AFEC_CSELR = (uint32_t)channel;
    return (UInt16)(AFEC0_REGS->AFEC_CDR & 0xFFFFU);
}

/* AFEC1 CH2/CH3 and CH4/CH5/CH6 need different ONE settings on this board.
 * Use isolated single-channel conversion to avoid mixed-channel alignment issues. */
UInt16 ReadAFEC1Channel( AFEC_CHANNEL_NUM channel )
{
    UInt32 to = 1000000U;
    uint32_t ch = (uint32_t)channel;
    uint32_t mr = AFEC_MR_PRESCAL(7U) | AFEC_MR_STARTUP_SUT64 | AFEC_MR_TRANSFER(2U);

    if( ch >= 4U ) { mr |= AFEC_MR_ONE_Msk; }

    AFEC1_REGS->AFEC_CR    = AFEC_CR_SWRST_Msk;
    AFEC1_REGS->AFEC_MR    = mr;
    AFEC1_REGS->AFEC_EMR   = AFEC_EMR_RES_NO_AVERAGE;
    AFEC1_REGS->AFEC_CGR   = 0U;
    AFEC1_REGS->AFEC_DIFFR = 0U;
    AFEC1_REGS->AFEC_CSELR = ch;
    AFEC1_REGS->AFEC_COCR  = 512U;
    AFEC1_REGS->AFEC_CHER  = (1UL << ch);
    AFEC1_REGS->AFEC_ACR   = AFEC_ACR_PGA0EN_Msk | AFEC_ACR_PGA1EN_Msk | AFEC_ACR_IBCTL(0x3U);
    AFEC1_REGS->AFEC_CR    = AFEC_CR_START_Msk;

    while( ((AFEC1_REGS->AFEC_ISR & (1UL << ch)) == 0U) && (--to > 0U) ) { }
    AFEC1_REGS->AFEC_CSELR = ch;
    return (UInt16)(AFEC1_REGS->AFEC_CDR & 0xFFFFU);
}

float AFEC_Init( void )
{
    /* Operational AFEC setup: TAG off, single-ended, gain x1, PGA input buffer on.
     * AFEC0 runs freerun; AFEC1 is read with isolated single-channel conversions. */
    AFEC0_REGS->AFEC_CR  = AFEC_CR_SWRST_Msk;
    AFEC1_REGS->AFEC_CR  = AFEC_CR_SWRST_Msk;

    /* PGA는 FREERUN과 함께써야 계속 켜진채 정착 유지(off면 변환마다 재기동->0 읽힘). MCC 기본이 그래서 freerun. */
    /* [수정] ONE 비트 추가: 데이터시트상 MR 쓰기 시 항상 1 필수. 누락 시 일부 채널(CH4/5) 변환 이상(4095 고착). */
    AFEC0_REGS->AFEC_MR  = AFEC_MR_PRESCAL(7U) | AFEC_MR_STARTUP_SUT64 | AFEC_MR_TRANSFER(2U) | AFEC_MR_ONE_Msk | AFEC_MR_FREERUN_Msk;
    AFEC1_REGS->AFEC_MR  = AFEC_MR_PRESCAL(7U) | AFEC_MR_STARTUP_SUT64 | AFEC_MR_TRANSFER(2U) | AFEC_MR_ONE_Msk | AFEC_MR_FREERUN_Msk;

    AFEC0_REGS->AFEC_EMR = AFEC_EMR_RES_NO_AVERAGE;   /* TAG off, 단일ended */
    AFEC1_REGS->AFEC_EMR = AFEC_EMR_RES_NO_AVERAGE;

    /* [데이터시트 필수] PGA0EN|PGA1EN 반드시 ON (입력버퍼). off면 핀이 ADC에 연결 안 돼 mid값만 읽힘. */
    AFEC0_REGS->AFEC_ACR = AFEC_ACR_PGA0EN_Msk | AFEC_ACR_PGA1EN_Msk | AFEC_ACR_IBCTL(0x3U);
    AFEC1_REGS->AFEC_ACR = AFEC_ACR_PGA0EN_Msk | AFEC_ACR_PGA1EN_Msk | AFEC_ACR_IBCTL(0x3U);
    AFEC0_REGS->AFEC_CGR = 0U; AFEC1_REGS->AFEC_CGR = 0U;   /* PGA 게인 X1 */
    AFEC0_REGS->AFEC_DIFFR = 0U; AFEC1_REGS->AFEC_DIFFR = 0U;

    /* 사용 채널 오프셋 0 */
    AFEC0_REGS->AFEC_CSELR = (uint32_t)AFEC_CH0; AFEC0_REGS->AFEC_COCR = 512U;
    AFEC0_REGS->AFEC_CSELR = (uint32_t)AFEC_CH2; AFEC0_REGS->AFEC_COCR = 512U;
    AFEC0_REGS->AFEC_CSELR = (uint32_t)AFEC_CH3; AFEC0_REGS->AFEC_COCR = 512U;
    AFEC0_REGS->AFEC_CSELR = (uint32_t)AFEC_CH5; AFEC0_REGS->AFEC_COCR = 512U;   /* PB2 = PRES1 리루트 */
    AFEC0_REGS->AFEC_CSELR = (uint32_t)AFEC_CH6; AFEC0_REGS->AFEC_COCR = 512U;
    AFEC0_REGS->AFEC_CSELR = (uint32_t)AFEC_CH7; AFEC0_REGS->AFEC_COCR = 512U;
    AFEC0_REGS->AFEC_CSELR = (uint32_t)AFEC_CH8; AFEC0_REGS->AFEC_COCR = 512U;
    AFEC0_REGS->AFEC_CSELR = (uint32_t)AFEC_CH9; AFEC0_REGS->AFEC_COCR = 512U;
    AFEC1_REGS->AFEC_CSELR = (uint32_t)AFEC_CH2; AFEC1_REGS->AFEC_COCR = 512U;
    AFEC1_REGS->AFEC_CSELR = (uint32_t)AFEC_CH3; AFEC1_REGS->AFEC_COCR = 512U;
    AFEC1_REGS->AFEC_CSELR = (uint32_t)AFEC_CH4; AFEC1_REGS->AFEC_COCR = 512U;
    AFEC1_REGS->AFEC_CSELR = (uint32_t)AFEC_CH5; AFEC1_REGS->AFEC_COCR = 512U;
    AFEC1_REGS->AFEC_CSELR = (uint32_t)AFEC_CH6; AFEC1_REGS->AFEC_COCR = 512U;   /* PC31 = PRES2 candidate */

    /* 아날로그 입력 핀 전환 (읽는 채널 전부 명시: 입력+풀없음) */
    PIOC_REGS->PIO_ODR  = (1UL<<29)|(1UL<<30)|(1UL<<15)|(1UL<<12)|(1UL<<31);   /* PC29/30 SEN, PC15/12 P28V, PC31 pressure input */
    PIOC_REGS->PIO_PUDR = (1UL<<29)|(1UL<<30)|(1UL<<15)|(1UL<<12)|(1UL<<31);
    PIOD_REGS->PIO_ODR  = (1UL << 30);                                /* PD30 PRES1 */
    PIOD_REGS->PIO_PUDR = (1UL << 30);
    PIOB_REGS->PIO_ODR  = (1UL<<3)|(1UL<<2);                          /* PB3 PRES3, PB2 */
    PIOB_REGS->PIO_PUDR = (1UL<<3)|(1UL<<2);
    PIOE_REGS->PIO_ODR  = (1UL<<5)|(1UL<<4);                          /* PE5 PRES4, PE4 */
    PIOE_REGS->PIO_PUDR = (1UL<<5)|(1UL<<4);
    PIOA_REGS->PIO_ODR  = (1UL<<17)|(1UL<<18)|(1UL<<19)|(1UL<<20);
    PIOA_REGS->PIO_PUDR = (1UL<<17)|(1UL<<18)|(1UL<<19)|(1UL<<20);

    /* 채널 enable: AFEC0 CH0,2,3,6,7,8,9 / AFEC1 CH2,3,4,5 */
    AFEC0_REGS->AFEC_CHER = AFEC_CHER_CH0_Msk | AFEC_CHER_CH2_Msk | AFEC_CHER_CH3_Msk | AFEC_CHER_CH5_Msk
                          | AFEC_CHER_CH6_Msk | AFEC_CHER_CH7_Msk | AFEC_CHER_CH8_Msk | AFEC_CHER_CH9_Msk;
    AFEC1_REGS->AFEC_CHER = AFEC_CHER_CH2_Msk | AFEC_CHER_CH3_Msk | AFEC_CHER_CH4_Msk | AFEC_CHER_CH5_Msk | AFEC_CHER_CH6_Msk;

    /* [핵심 누락 수정] FREERUN 연속변환 킥오프 START. 이게 없어서 변환이 안 돌고 전부 stale(0) 읽힘. */
    AFEC0_REGS->AFEC_CR = AFEC_CR_START_Msk;
    AFEC1_REGS->AFEC_CR = AFEC_CR_START_Msk;

    return 0.0f;
}

/*******************************************************************************
 End of File
*/
