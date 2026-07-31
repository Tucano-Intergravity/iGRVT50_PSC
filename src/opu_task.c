/**
 * @file opu_task.c
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
#include <stdio.h>
#include <string.h>
#include "definitions.h"                // SYS function prototypes

/* --- FreeRTOS includes --- */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"

/* --- User includes --- */
#include "sam_ctl.h"
#include "sensor.h"
#include "lpsolvalve.h"
#include "hpsolvalve.h"

/*==============================================================================
 * Gloabal Variables
 *============================================================================*/
/* --- handler  --- */
static TaskHandle_t xTcTask;		// TC task handler
static TaskHandle_t xRsTask;		// RS task handler
static TaskHandle_t xAdcTask;		// ADC task handler

/*==============================================================================
 * Gloabal Function
 *============================================================================*/
void OpuTask( void *pvParameters );
void USART1_ReadCallback(uintptr_t context);
/*==============================================================================
 * Local Variables
 *============================================================================*/
sTcTemp stTcTemp[2];    // [0]=ADS1263 #1, [1]=ADS1263 #2
sAdcTemp stAdcTemp;

/* [28V 전류센스 보정값] acal 명령으로 런타임 설정.
 *  s_isenseOffV = 0A 일 때 P28V_I 핀전압(V), s_isenseApv = A/V 스케일.
 *  I = (Iv - offV) * apv. (IO보드 ACS724+TLC2272 체인이 핀에 주는 전압 기준) */
static float s_isenseOffV = 1.71f;
static float s_isenseApv  = 10.0f;
void OpuSetIsenseCal( float offV, float apv ) { s_isenseOffV = offV; s_isenseApv = apv; }
void OpuGetIsenseCal( float *offV, float *apv ) { if(offV){*offV=s_isenseOffV;} if(apv){*apv=s_isenseApv;} }

/* [압력 게인 보정] 표시값 = 핀전압 × s_presGain (앞단 amp 보상). pcal 명령으로 런타임 보정.
 * 기본값 = 직접주입 1~5V 스윕 실측보정(2026-06-18, 신IO보드). 선형구간(1~4.5V) 표시/주입=0.840 -> gain=1.2044/0.840.
 * 1~4.5V 1:1 검증(4.5V->핀3.14V, 3.3Vref 내). 5V는 핀포화(센서4.5Vmax라 무관). (분배저항 교체 시 재보정 필요) */
static float s_presGain = 1.4338f;
void  OpuSetPresGain( float g ) { if( (g>0.1f) && (g<3.0f) ) { s_presGain = g; } }
float OpuGetPresGain( void )    { return s_presGain; }

#define LPV01_AUTO_TEST_ENABLE      0U
#define LPV01_AUTO_TEST_DELAY_MS    3000UL
#define SV01_AUTO_TEST_POLL_MS      50UL
#define LPV01_GPIO_PA0_MASK         (1UL << 0)    /* LP_Valve01 = PA0 / PWM0_PWMH0 */
#define LPV01_RTN_PD12_MASK         (1UL << 12)   /* LP_Valve_CTRL_ALL = PD12 / TPD2017_KILL_ALL */

#define MAX_RB_IDX 100
#define RX_BUF_SIZE  MAX_RB_DATA
/* [수정] RS422 수신: 16B 한 프레임 다 채워야 콜백 뜨던 문제 -> 1B 단위 즉시 수신.
 * (plib 콜백은 요청크기 다 받아야 발생 + 타임아웃 없음. 1로 두면 바이트마다 처리) */
#define RX_READ_SIZE 1U
static uint8_t rxBuf[RX_BUF_SIZE];

/* 링버퍼 */
sRingBufInfo stUartRbRx;                // RS422 RX Ring Buffer 정보
UInt8 ucUartRbRx[MAX_RB_IDX][RX_BUF_SIZE];		// network RX 링버퍼

/*==============================================================================
 * Local Function
 *============================================================================*/
/*-------- TC Processing --------*/
static void TcTask(void *p);
static void TcPrint( UInt16 usCnt );

/*-------- ADC Processing --------*/
static void AdcTask(void *p);
static void AdcPrint( UInt16 usCnt );

/*-------- RS422 Processing --------*/
static void RsTask(void *p);
static SInt32 UartEnqueue( UInt32 *pBuf, sRingBufInfo *pRingBufInfo, UInt32 uiLen );
static SInt32 UartDequeue( sRbData *pRbData, sRingBufInfo *pRingBufInfo );
static void DdrRingBufferInit( sRingBufInfo *pRingBufInfo );
static void RingBufferInit( void );
static void TaskCreate( void );

/*==============================================================================
 * Functions
 *============================================================================*/



/*-------- TC Processing --------*/
/**
 * @fn TcTask
 * @brief TC 처리 Thread
 * @param void *p
 * @return void
 * @date 2025-12-18
 */
static void TcTask(void *p)
{
    const TickType_t x10ms = pdMS_TO_TICKS( DELAY_10_MSECOND );
    ADS1263_Init();

	while(1)
	{
        /* [수정] ADS1263은 U3 1개만 실장. 이전 2칩(ADS#2/CS=PD28)은 펌웨어 가공 -> 제거. */
        ADS1263_SetDevice( 1 );
        stTcTemp[0].fTempCh1 = ADS1263_GetTemperature( 0 );   /* AIN0/1 = TC_SEN1 */
        stTcTemp[0].fTempCh2 = ADS1263_GetTemperature( 1 );   /* AIN2/3 = TC_SEN2 */
        stTcTemp[0].fTempCh3 = ADS1263_GetTemperature( 2 );   /* AIN4/5 = TC_SEN3 */
        stTcTemp[0].fTempCh4 = ADS1263_GetTemperature( 3 );   /* AIN6/7 = TC_SEN4 */
        stTcTemp[0].fTempCJ  = ADS1263_GetTemperature( 4 );   /* 내부 die온도 = CJ */
        vTaskDelay( (x10ms*50) );
	}
}

/**
 * @fn TcPrint
 * @brief TC 출력 함수
 * @param void
 * @return void
 * @date 2025-12-18
 */
static void TcPrint( UInt16 usCnt )
{
    if( (usCnt%10) == 0 )
    {
        if( usTcPrn == 1 )
        {
            printf( "ADS#1 TC1:%0.2f TC2:%0.2f TC3:%0.2f TC4:%0.2f CJ:%0.2f\r\n",
                stTcTemp[0].fTempCh1, stTcTemp[0].fTempCh2, stTcTemp[0].fTempCh3, stTcTemp[0].fTempCh4, stTcTemp[0].fTempCJ );
            /* [진단] 채널별 raw ADC code (FFFFFFFF=SPI무응답). ADS1263은 U3 1개만 실장. */
            printf( "  raw: %08lX %08lX %08lX %08lX CJ=%08lX\r\n",
                (unsigned long)(uint32_t)ADS1263_GetRawCode(1,0), (unsigned long)(uint32_t)ADS1263_GetRawCode(1,1),
                (unsigned long)(uint32_t)ADS1263_GetRawCode(1,2), (unsigned long)(uint32_t)ADS1263_GetRawCode(1,3),
                (unsigned long)(uint32_t)ADS1263_GetRawCode(1,4) );
        }
    }

}


/*-------- ADC Processing --------*/
 /**
 * @fn AdcTask
 * @brief RS422 처리 Thread
 * @param void *p
 * @return void
 * @date 2025-12-18
 */
/* 5개 샘플 중앙값(median) - 간헐 글리치 제거 */
static UInt16 med5( UInt16 *a )
{
    UInt16 t; UInt8 i, j;
    for( i = 0U; i < 4U; i++ ) {
        for( j = (UInt8)(i + 1U); j < 5U; j++ ) {
            if( a[j] < a[i] ) { t = a[i]; a[i] = a[j]; a[j] = t; }
        }
    }
    return a[2];
}

static void AdcTask(void *p)
{
    const TickType_t x10ms = pdMS_TO_TICKS( DELAY_10_MSECOND );
    /* AFEC0: CH0,2,3,5,6,7,8,9 / AFEC1: CH2,3,4,5,6
     * Board test: physical PT1 responds on AFEC0 CH0 (PD30), not AFEC1 CH6 (PC31). */
    static const UInt8 ch0[8] = { 0U, 2U, 3U, 5U, 6U, 7U, 8U, 9U };
    static const UInt8 ch1[5] = { 2U, 3U, 4U, 5U, 6U };
    UInt16 s0[8][5], s1[5][5];
    UInt16 r0[8], r1[5];
    UInt16 ptRaw[SENSOR_PT_CHANNEL_COUNT];
    UInt8 pass, i;

	while(1)
	{
        /* 다중샘플: 매 pass마다 1회 START 후 전 채널 읽기(혼선 없음), 5회 모아 채널별 median */
        for( pass = 0U; pass < 5U; pass++ )
        {
            AFEC0_SeqConvert(0U);
            for( i = 0U; i < 8U; i++ ) { s0[i][pass] = ReadAFEC0Channel((AFEC_CHANNEL_NUM)ch0[i]); }
            AFEC1_SeqConvert(0U);
            for( i = 0U; i < 5U; i++ ) { s1[i][pass] = ReadAFEC1Channel((AFEC_CHANNEL_NUM)ch1[i]); }
        }
        for( i = 0U; i < 8U; i++ ) { r0[i] = med5( s0[i] ); }
        for( i = 0U; i < 5U; i++ ) { r1[i] = med5( s1[i] ); }

        /* [압력 보정] 표시값 = 핀전압 × s_presGain (앞단 amp 보상). pcal 명령으로 런타임 보정. */
        ptRaw[0] = r0[0];
        ptRaw[1] = r1[4];
        ptRaw[2] = r0[1];
        ptRaw[3] = r0[2];
        ptRaw[4] = r0[3];
        ptRaw[5] = r0[4];
        ptRaw[6] = r0[5];
        ptRaw[7] = r0[6];
        ptRaw[8] = r0[7];
        Sensor_UpdatePtRawAdcScan( ptRaw, SENSOR_PT_CHANNEL_COUNT );
        stAdcTemp.fPres1      = AFEC_ToVoltage( r0[0] ) * s_presGain; // AFEC0 CH0 PD30 PRES_SENSE1
        stAdcTemp.fPres2      = AFEC_ToVoltage( r1[4] ) * s_presGain; // AFEC1 CH6 PC31 PRES_SENSE2 candidate
        stAdcTemp.fPres3      = AFEC_ToVoltage( r0[1] ) * s_presGain; // CH2  PB3  PRES_SENSE3
        stAdcTemp.fPres4      = AFEC_ToVoltage( r0[2] ) * s_presGain; // CH3  PE5  PRES_SENSE4
        stAdcTemp.fPres5      = AFEC_ToVoltage( r0[3] ) * s_presGain; // CH5  PB2  PRES_SENSE5
        stAdcTemp.fSp6        = AFEC_ToVoltage( r0[4] ) * s_presGain; // CH6  PA17 PRES_SENSE6
        stAdcTemp.fSp7        = AFEC_ToVoltage( r0[5] ) * s_presGain; // CH7  PA18 PRES_SENSE7
        stAdcTemp.fSp8        = AFEC_ToVoltage( r0[6] ) * s_presGain; // CH8  PA19 PRES_SENSE8
        stAdcTemp.fSp9        = AFEC_ToVoltage( r0[7] ) * s_presGain; // CH9  PA20 PRES_SENSE9
        stAdcTemp.fP28vIsense = AFEC_ToVoltage( r1[0] );        // CH2  PC15 P28V_ISENSE
        stAdcTemp.fP28vVsense = AFEC_ToVoltage( r1[1] );        // CH3  PC12 P28V_VSENSE
        stAdcTemp.fSen5v      = AFEC_ToVoltage( r1[2] );        // CH4  PC29 SEN_P5V
        stAdcTemp.fSenVdd     = AFEC_ToVoltage( r1[3] );        // CH5  PC30 SEN_VDD

        vTaskDelay( (x10ms*5) );
	}
}

/**
 * @fn AdcPrint
 * @brief ADC 출력 함수
 * @param void
 * @return void
 * @date 2025-12-18
 */
static void AdcPrint( UInt16 usCnt )
{
    if( (usCnt%10) == 0 )
    {
        if( usAdcPrn == 1 )
        {
            /* [환산] 센스 핀전압 -> 실제 단위. 공칭 회로게인 + DMM 실측보정(2026-06-18). */
            #define P28V_V_GAIN (8.61f)   /* 28V = 핀 x8.61 (공칭8.89=INA2.26x전단분배20.1, DMM보정 28.9->28.0V) */
            #define P5V_GAIN    (1.905f)  /* 5V  = 핀 x1.905 (분배0.5->공칭x2.0, DMM보정 2.62->5.0V) */
            #define RAIL5_TH    (4.0f)    /* 5V 존재 임계 */
            #define VDD_TH      (2.7f)    /* VDD(3.3V) 존재 임계 */
            float v28 = stAdcTemp.fP28vVsense * P28V_V_GAIN;
            float v5  = stAdcTemp.fSen5v * P5V_GAIN;     /* 0.5 분배 복원 + DMM보정 */
            float i28 = (stAdcTemp.fP28vIsense - s_isenseOffV) * s_isenseApv;  /* acal로 보정 */
            if( i28 < 0.0f ) { i28 = 0.0f; }
            printf( "PRES1:%0.2f PRES2:%0.2f PRES3:%0.2f PRES4:%0.2f PRES5:%0.2f PRES6:%0.2f PRES7:%0.2f PRES8:%0.2f PRES9:%0.2f V | 28V=%0.1fV I=%0.2fA(Iv=%0.2f) | 5V:%s(%0.2f) VDD:%s(%0.2f)\r\n",
                stAdcTemp.fPres1, stAdcTemp.fPres2, stAdcTemp.fPres3, stAdcTemp.fPres4, stAdcTemp.fPres5,
                stAdcTemp.fSp6, stAdcTemp.fSp7, stAdcTemp.fSp8, stAdcTemp.fSp9,
                v28, i28, stAdcTemp.fP28vIsense,
                (v5 >= RAIL5_TH)?"UP":"LO", v5,
                (stAdcTemp.fSenVdd >= VDD_TH)?"UP":"LO", stAdcTemp.fSenVdd );
                //stAdcTemp.fSen5v,
                //stAdcTemp.fSenVdd );
        }
    }

}


/*-------- RS422 Processing --------*/
 /**
 * @fn RsTask
 * @brief RS422 처리 Thread
 * @param void *p
 * @return void
 * @date 2025-12-18
 */
static void RsTask(void *p)
{
    const TickType_t xLpsvPeakTicks = pdMS_TO_TICKS( 100UL );
    const TickType_t xLpsvHoldTicks = pdMS_TO_TICKS( 900UL );
    char cTxMsg[160];
    int iTxLen;
    sRbData stRbData;

    /* RS485-style test on USART1 (PA21 RXD1 / PB4 TXD1, PA22 DE, PA24 /RE) */
    RS422_Init( 115200 );
    USART1_Read( rxBuf, RX_READ_SIZE );

	while(1)
	{
        TickType_t xLpsvCycleTick = xTaskGetTickCount();
        sSensorPtScan stPtScan;
        sSensorTcScan stTcScan;
        UInt32 uiScanCount;

        LpSolValve_SetDuty( LPSOLVALVE_SV1, 100U );
        HpSolValve_Toggle( HPSOLVALVE_SV1 );
        vTaskDelayUntil( &xLpsvCycleTick, xLpsvPeakTicks );
        LpSolValve_SetDuty( LPSOLVALVE_SV1, 10U );

        Sensor_GetPtScan( &stPtScan );
        Sensor_GetTcScan( &stTcScan );
        uiScanCount = Sensor_GetPtScanCount();

        iTxLen = snprintf( cTxMsg, sizeof(cTxMsg),
                           "SVTEST CNT=%lu PT1:%ldmV(%u) TC1:%lduV(%ld) LPSV1:100%%/100ms->10%%/900ms HPSV1:%s\r\n",
                           (unsigned long)uiScanCount,
                           (long)stPtScan.adcMilliVolt[0], (unsigned)stPtScan.rawAdc[0],
                           (long)stTcScan.microVolt[0], (long)stTcScan.rawCode[0],
                           (HpSolValve_IsOn( HPSOLVALVE_SV1 ) != 0U) ? "ON" : "OFF" );
        if( iTxLen < 0 )
        {
            iTxLen = 0;
        }
        else if( iTxLen >= (int)sizeof(cTxMsg) )
        {
            iTxLen = (int)sizeof(cTxMsg) - 1;
        }

        RS485_SetTransmit( 1U );
        while( USART1_WriteIsBusy() ) { }
        if( iTxLen > 0 )
        {
            USART1_Write( cTxMsg, (size_t)iTxLen );
        }
        while( USART1_WriteIsBusy() ) { }
        while( !USART1_TransmitComplete() ) { }
        RS485_SetTransmit( 0U );

        /* UART Dequeue: 밀린 데이터 전부 처리(1B 수신/버스트 대응) */
        while( UartDequeue( &stRbData, &stUartRbRx ) >= 0 )
        {
            /* RS422 Loopback (uart 1=ON / uart 0=OFF) */
            if( usRs422Loop == 1 )
            {
                RS485_SetTransmit( 1U );
                while( USART1_WriteIsBusy() ) { }
                USART1_Write( stRbData.ucData, stRbData.usSize );
                while( USART1_WriteIsBusy() ) { }
                while( !USART1_TransmitComplete() ) { }
                RS485_SetTransmit( 0U );
            }
        }

        vTaskDelayUntil( &xLpsvCycleTick, xLpsvHoldTicks );
	}
}

/**
 * @fn		UartEnqueue
 * @brief	DDR3 Ring Buffer write 함수
 * @param	UInt8 *pBuf : write 데이터 포인터
 * @param	sRingBufInfo *pRingBufInfo : Ring Buffer 정보
 * @return	Ring Buffer 상태 (-1: Ring buffer is full, 1 : Normal)
 * @date	2025/12/18
 */
static SInt32 UartEnqueue( UInt32 *pBuf, sRingBufInfo *pRingBufInfo, UInt32 uiLen )
{
	SInt32 ucSts = 0;				// -1: Ring buffer is full, 1 : Normal
	volatile UInt32 *pAddr = (volatile UInt32 *)pRingBufInfo->uiAddr;

	if( pRingBufInfo->siCount == MAX_RB_IDX )
	{
		/* Ring buffer is full */
		/* 가장오래된 데이터 삭제 */
		pRingBufInfo->siFront = (pRingBufInfo->siFront+1)%MAX_RB_IDX;
		pRingBufInfo->siCount--;
		ucSts = -1;
	}
	else
	{
		/* BRAM to DDR3 write */
		memcpy( &pAddr[pRingBufInfo->siRear*(RX_BUF_SIZE/4)+1], pBuf, (uiLen+(4-uiLen%4)) );
		pAddr[pRingBufInfo->siRear*(RX_BUF_SIZE/4)] = uiLen;
		pRingBufInfo->siRear = (pRingBufInfo->siRear+1)%MAX_RB_IDX;
		pRingBufInfo->siCount++;
	}

	return ucSts;
}


/**
 * @fn		UartDequeue
 * @brief	DDR3 Ring Buffer Read 함수
 * @param	UInt8 *pBuf : Read 데이터 포인터
 * @param	sRingBufInfo *pRingBufInfo : Ring Buffer 정보
 * @return	Ring Buffer 상태 (-1: Ring buffer is Empty, 0~ : Message Count)
 * @date	2025/12/18
 */
static SInt32 UartDequeue( sRbData *pRbData, sRingBufInfo *pRingBufInfo )
{
	SInt32 ucSts;																// -1: Ring buffer is Empty, 0~ : Message Count
	volatile UInt8 *pAddr = (volatile UInt8 *)pRingBufInfo->uiAddr;

	UInt32 *pData = (UInt32 *)pAddr+pRingBufInfo->siFront*(MAX_RB_DATA/4);		// 4byte 데이터 포인터

	if( pRingBufInfo->siCount == 0 )
	{
		ucSts = -1;
	}
	else
	{
		/* 메시지 길이 확인 */

		pRbData->usSize = *pData;

		/* BRAM to DDR3 read */
		memcpy( pRbData->ucData, &pAddr[pRingBufInfo->siFront*MAX_RB_DATA+4], pRbData->usSize );
		pRingBufInfo->siFront = (pRingBufInfo->siFront+1)%MAX_RB_IDX;
		pRingBufInfo->siCount--;

		ucSts = pRingBufInfo->siCount;
	}
	return ucSts;
}

/**
 * @fn		USART1_ReadCallback
 * @brief	RS422(USART1) 수신 콜백 함수
 * @return	void
 * @date	2025/12/18
 */
void USART1_ReadCallback(uintptr_t context)
{
    UInt16 scSts;
    size_t rxCount;

    if (USART1_ErrorGet() != USART_ERROR_NONE)
    {
        USART1_Read( rxBuf, RX_READ_SIZE );
        return;
    }

    rxCount = USART1_ReadCountGet();

    if ( rxCount > 0)
    {
        scSts = UartEnqueue( rxBuf, &stUartRbRx, rxCount );
        if( scSts < 0 )
        {
            /* ring buffer is full */
        }
    }

    USART1_Read( rxBuf, RX_READ_SIZE );
}

/**
 * @fn		DdrRingBufferInit
 * @brief	DDR3 Ring Buffer 초기화 함수
 * @param	sRingBufInfo *pRingBufInfo : Ring Buffer 정보
 * @return	void
 * @date	2023/02/03
 */
static void DdrRingBufferInit( sRingBufInfo *pRingBufInfo )
{
	/* Ring Buffer 초기화 */
	pRingBufInfo->siFront = 0;
	pRingBufInfo->siRear = 0;
	pRingBufInfo->siCount = 0;
}


/**
 * @fn RingBufferInit
 * @brief 링버퍼 초기화 함수
 * @param void
 * @return void
 * @date 2025-12-18
 */
static void RingBufferInit( void )
{
	UInt32 i;

	/* UART */
	DdrRingBufferInit( &stUartRbRx );
	stUartRbRx.uiAddr =  ucUartRbRx;
}


/**
 * @fn TaskCreate
 * @brief Task 생성 함수
 * @param void
 * @return void
 * @date 2025-12-18
 */
static void TaskCreate( void )
{
	/* --- TC Task --- */
	xTaskCreate( TcTask, "TcTask", SCDAU_STACK_SIZE, NULL, tskIDLE_PRIORITY, &xTcTask );

	/* --- RS422 Task --- */
	xTaskCreate( RsTask, "RsTask", SCDAU_STACK_SIZE, NULL, tskIDLE_PRIORITY, &xRsTask );

    /* --- RS422 Task --- */
	xTaskCreate( AdcTask, "AdcTask", SCDAU_STACK_SIZE, NULL, tskIDLE_PRIORITY, &xAdcTask );
}

/*==============================================================================
 * HP 밸브 드라이버 DRV3946-Q1 (SPI0 폴링, 검증용)
 *  - SPI0: SCK PD22 / MISO PD20 / MOSI PD21 / CS NPCS1 PD25 (하드웨어 CS)
 *  - 16-bit frame. ※ SPI mode·레지스터 주소맵은 DRV3946-Q1 데이터시트로 확정 필요
 *    (현재 기본값: CPOL=0/CPHA=1(mode1). 링크 read값이 이상하면 mode 조정)
 *  - SPI0 클럭은 SPI0_Initialize()(plib)에서 이미 enable. 여기서 16bit/NPCS1로 재설정.
 *============================================================================*/
/* [DRV SPI 모드] hpv mode로 런타임 변경(이전엔 Read24가 덮어써서 토글 무효였음).
 * 기본 = Mode1(CPOL0/CPHA1 = NCPHA_TRAILING). 0xFFFF면 4모드 다 시험. */
uint32_t g_drvCpolNcpha = (SPI_CSR_CPOL_IDLE_LOW | SPI_CSR_NCPHA_VALID_TRAILING_EDGE);
void DRV3946_SetCsrMode( uint32_t cpolNcpha ) { g_drvCpolNcpha = cpolNcpha; }

/* [추가] DRV3946 활성 노드(0~3) — 4칩 공유 SPI버스에서 wake/config/ChCtrl/read 대상 칩 선택.
 * nFAULT/NAD 저항: 5.6k=node0(HP1-2,U22), 12k=node1(HP3-4,U23), 27k=node2(HP5-6,U25), 56k=node3(HP7-8,U24).
 * Write24/Read24(Wake경유)/ChCtrl 헤더 상위2비트(A6A5)에 반영. CLR_FAULT 등 CMD2는 브로드캐스트 유지. */
UInt8 g_drvNode = 0U;
void  DRV3946_SetNode( UInt8 n ) { g_drvNode = (UInt8)(n & 0x3U); }
UInt8 DRV3946_GetNode( void )    { return g_drvNode; }

void DRV3946_SPI_Init( void )
{
    SPI0_REGS->SPI_CR  = SPI_CR_SPIDIS_Msk | SPI_CR_SWRST_Msk;
    SPI0_REGS->SPI_MR  = SPI_MR_MSTR_Msk | SPI_MR_PCS_NPCS1 | SPI_MR_MODFDIS_Msk;
    SPI0_REGS->SPI_CSR[1] = g_drvCpolNcpha
                          | SPI_CSR_BITS_16_BIT | SPI_CSR_SCBR(150);   /* ~1MHz */
    SPI0_REGS->SPI_CR  = SPI_CR_SPIEN_Msk;
}

/* [디버그] 2프레임 읽기: CS를 내린 채(CSAAT) 2워드 연속 -> 파이프라인 읽기 칩 대응.
 * w1=주소 요청, w2=NOP(0x0000). 반환=2번째 응답(=w1 주소의 데이터), *rx1=1번째 응답 */
UInt16 DRV3946_Xfer2( UInt16 w1, UInt16 w2, UInt16 *rx1 )
{
    UInt16 r1, r2;
    SPI0_REGS->SPI_CSR[1] |= SPI_CSR_CSAAT_Msk;                     /* CS 유지 */
    (void)(SPI0_REGS->SPI_RDR);                                     /* flush */
    while( (SPI0_REGS->SPI_SR & SPI_SR_TDRE_Msk) == 0U ){}
    SPI0_REGS->SPI_TDR = w1;
    while( (SPI0_REGS->SPI_SR & SPI_SR_RDRF_Msk) == 0U ){}
    r1 = (UInt16)(SPI0_REGS->SPI_RDR & SPI_RDR_RD_Msk);
    SPI0_REGS->SPI_CR = SPI_CR_LASTXFER_Msk;                        /* 다음이 마지막 -> 후 CS 해제 */
    while( (SPI0_REGS->SPI_SR & SPI_SR_TDRE_Msk) == 0U ){}
    SPI0_REGS->SPI_TDR = w2;
    while( (SPI0_REGS->SPI_SR & SPI_SR_RDRF_Msk) == 0U ){}
    r2 = (UInt16)(SPI0_REGS->SPI_RDR & SPI_RDR_RD_Msk);
    SPI0_REGS->SPI_CSR[1] &= ~SPI_CSR_CSAAT_Msk;                    /* 복원 */
    if( rx1 != NULL ) { *rx1 = r1; }
    return r2;
}

/* [DRV3946 정식 프로토콜] 24-bit read = 8bit헤더 + 16bit데이터, mode1, CRC불필요.
 * 헤더 = [A6 A5 node][A4..A0 reg][1=read]. CS를 24클럭 내내 Low(CSAAT) 8bit×3.
 * SDO: byte0=echo/err, byte1=DATA[15:8], byte2=DATA[7:0]. 반환=레지스터값. */
/* [내부] DRV 24bit(3바이트) 한 프레임 송수신 (CS low 유지 후 해제). rx[0..2] 반환. */
static void drv_frame3( UInt8 b0, UInt8 b1, UInt8 b2, UInt8 *rx )
{
    UInt8 r0, r1, r2;
    (void)(SPI0_REGS->SPI_RDR);
    while( (SPI0_REGS->SPI_SR & SPI_SR_TDRE_Msk) == 0U ){}
    SPI0_REGS->SPI_TDR = b0;
    while( (SPI0_REGS->SPI_SR & SPI_SR_RDRF_Msk) == 0U ){}
    r0 = (UInt8)(SPI0_REGS->SPI_RDR & 0xFFU);
    while( (SPI0_REGS->SPI_SR & SPI_SR_TDRE_Msk) == 0U ){}
    SPI0_REGS->SPI_TDR = b1;
    while( (SPI0_REGS->SPI_SR & SPI_SR_RDRF_Msk) == 0U ){}
    r1 = (UInt8)(SPI0_REGS->SPI_RDR & 0xFFU);
    SPI0_REGS->SPI_CR = SPI_CR_LASTXFER_Msk;
    while( (SPI0_REGS->SPI_SR & SPI_SR_TDRE_Msk) == 0U ){}
    SPI0_REGS->SPI_TDR = b2;
    while( (SPI0_REGS->SPI_SR & SPI_SR_RDRF_Msk) == 0U ){}
    r2 = (UInt8)(SPI0_REGS->SPI_RDR & 0xFFU);
    if( rx != NULL ) { rx[0]=r0; rx[1]=r1; rx[2]=r2; }
}

/* [v0.7.25 확정] 단일 프레임 읽기. rdn 덤프 실측: F1=항상 데이터(정렬정상, b1b2=DATA,
 * b0=상태바이트), F2+=에코(CS재토글 후 정렬붕괴). DRV SDO는 in-frame 응답(Table6-13).
 * -> 한 프레임만 읽고 rx[1:2] 반환하면 데이터 상위바이트가 (reg<<1)|1과 같아도 정확.
 * (이전 2프레임+에코휴리스틱은 STATUS0=0x03xx 같은 값을 에코로 오판하는 버그가 있었음) */
UInt16 DRV3946_Read24( UInt8 reg5, UInt8 node2, UInt8 *echo )
{
    UInt8  hdr = (UInt8)(((node2 & 0x3U) << 6) | ((reg5 & 0x1FU) << 1) | 0x1U);
    UInt8  rx[3] = {0,0,0};
    UInt32 csr_save = SPI0_REGS->SPI_CSR[1];

    /* [필수] LLB(내부 루프백) 강제 해제 + DLYBCS(프레임간 CS-high 시간) — 연속리드 정렬 보장 */
    SPI0_REGS->SPI_CR  = SPI_CR_SPIDIS_Msk;
    SPI0_REGS->SPI_MR  = (SPI0_REGS->SPI_MR & ~SPI_MR_LLB_Msk & ~SPI_MR_DLYBCS_Msk)
                       | SPI_MR_MSTR_Msk | SPI_MR_PCS_NPCS1 | SPI_MR_MODFDIS_Msk | SPI_MR_DLYBCS(255);
    SPI0_REGS->SPI_CR  = SPI_CR_SPIEN_Msk;

    SPI0_REGS->SPI_CSR[1] = g_drvCpolNcpha
                          | SPI_CSR_BITS_8_BIT | SPI_CSR_SCBR(255)
                          | SPI_CSR_DLYBS(64) | SPI_CSR_CSAAT_Msk;
    SYSTICK_DelayMs( 2 );                     /* [실험] 리드 전 회복시간 — 연속리드 정렬 검증 */
    drv_frame3( hdr, 0x00U, 0x00U, rx );     /* 단일 프레임 = 항상 정렬된 실데이터 */

    SPI0_REGS->SPI_CSR[1] = csr_save;
    if( echo != NULL ) { *echo = rx[0]; }
    return (UInt16)((rx[1] << 8) | rx[2]);
}

/* [진단] 같은 reg를 nframes번 CS토글 연속 읽기 -> 각 프레임 3바이트씩 out[]에 덤프.
 * 응답이 몇 번째 프레임에 나오는지(파이프라인 깊이) 실측용. out 크기 = nframes*3. */
void DRV3946_ReadFrames( UInt8 reg5, UInt8 node2, UInt8 *out, int nframes )
{
    UInt8 hdr = (UInt8)(((node2 & 0x3U) << 6) | ((reg5 & 0x1FU) << 1) | 0x1U);
    int   f;
    UInt32 csr_save = SPI0_REGS->SPI_CSR[1];
    /* LLB off 보장 + DLYBCS(프레임간 CS-high) */
    SPI0_REGS->SPI_CR = SPI_CR_SPIDIS_Msk;
    SPI0_REGS->SPI_MR = (SPI0_REGS->SPI_MR & ~SPI_MR_LLB_Msk & ~SPI_MR_DLYBCS_Msk)
                      | SPI_MR_MSTR_Msk | SPI_MR_PCS_NPCS1 | SPI_MR_MODFDIS_Msk | SPI_MR_DLYBCS(255);
    SPI0_REGS->SPI_CR = SPI_CR_SPIEN_Msk;
    SPI0_REGS->SPI_CSR[1] = g_drvCpolNcpha
                          | SPI_CSR_BITS_8_BIT | SPI_CSR_SCBR(255)
                          | SPI_CSR_DLYBS(64) | SPI_CSR_CSAAT_Msk;
    for( f = 0; f < nframes; f++ )
    {
        drv_frame3( hdr, 0x00U, 0x00U, &out[f*3] );   /* 각 프레임 CS토글, 3바이트 캡처 */
    }
    SPI0_REGS->SPI_CSR[1] = csr_save;
}

/* [진단] CS를 내린 채 hdr + (n-1)개 0x00 연속 클럭 -> SDO 스트림 n바이트 덤프.
 * reg 실제값(reset 0xC040 / 우리가쓴 0xED01)이 스트림 어디에 박히는지 = read offset 확정. */
void DRV3946_ReadRaw( UInt8 reg5, UInt8 *out, int n )
{
    UInt8  hdr = (UInt8)(((reg5 & 0x1FU) << 1) | 0x1U);
    int    i;
    UInt32 csr_save = SPI0_REGS->SPI_CSR[1];
    SPI0_REGS->SPI_CSR[1] = g_drvCpolNcpha
                          | SPI_CSR_BITS_8_BIT | SPI_CSR_SCBR(255) | SPI_CSR_DLYBS(64) | SPI_CSR_CSAAT_Msk;
    (void)(SPI0_REGS->SPI_RDR);
    for( i = 0; i < n; i++ )
    {
        if( i == n-1 ) { SPI0_REGS->SPI_CR = SPI_CR_LASTXFER_Msk; }
        while( (SPI0_REGS->SPI_SR & SPI_SR_TDRE_Msk) == 0U ){}
        SPI0_REGS->SPI_TDR = (i == 0) ? hdr : 0x00U;
        while( (SPI0_REGS->SPI_SR & SPI_SR_RDRF_Msk) == 0U ){}
        out[i] = (UInt8)(SPI0_REGS->SPI_RDR & 0xFFU);
    }
    SPI0_REGS->SPI_CSR[1] = csr_save;
}

/* [진단] 임의 TX 버퍼를 CS유지로 보내고 RX 그대로 캡처 (루프백 판별용).
 * MISO에 우리 MOSI 패턴(0xAA,0x55..)이 그대로 되돌아오면 = SDO 경로 단선/루프백.
 * DRV의 실데이터가 오면 = SDO 정상. */
void DRV3946_XferRaw( const UInt8 *tx, UInt8 *rx, int n )
{
    int    i;
    UInt32 csr_save = SPI0_REGS->SPI_CSR[1];
    SPI0_REGS->SPI_CSR[1] = g_drvCpolNcpha
                          | SPI_CSR_BITS_8_BIT | SPI_CSR_SCBR(255) | SPI_CSR_DLYBS(64) | SPI_CSR_CSAAT_Msk;
    (void)(SPI0_REGS->SPI_RDR);
    for( i = 0; i < n; i++ )
    {
        if( i == n-1 ) { SPI0_REGS->SPI_CR = SPI_CR_LASTXFER_Msk; }
        while( (SPI0_REGS->SPI_SR & SPI_SR_TDRE_Msk) == 0U ){}
        SPI0_REGS->SPI_TDR = tx[i];
        while( (SPI0_REGS->SPI_SR & SPI_SR_RDRF_Msk) == 0U ){}
        rx[i] = (UInt8)(SPI0_REGS->SPI_RDR & 0xFFU);
    }
    SPI0_REGS->SPI_CSR[1] = csr_save;
}

/* [진단] DRV 읽기 2프레임 바이트덤프 (파이프라인 정렬 확인용).
 * DRV SDO는 "이전 프레임" 데이터를 반환(데이터시트) -> 프레임2에 reg의 실데이터가 옴.
 * out6 = [F1: echo,hi,lo][F2: echo,hi,lo]. F2의 hi/lo가 reg 실값일 가능성. */
void DRV3946_ReadDump( UInt8 reg, UInt8 *out6 )
{
    UInt8  e = 0;
    UInt16 d;
    d = DRV3946_Read24( reg, 0U, &e ); out6[0]=e; out6[1]=(UInt8)(d>>8); out6[2]=(UInt8)d;
    d = DRV3946_Read24( reg, 0U, &e ); out6[3]=e; out6[4]=(UInt8)(d>>8); out6[5]=(UInt8)d;
}

/* [디버그] TcTask 일시정지/재개 (tcid 단독 SPI1 접근 -> 경합 제거) */
void TcTask_Hold( void )    { if( xTcTask != NULL ) { vTaskSuspend( xTcTask ); } }
void TcTask_Release( void ) { if( xTcTask != NULL ) { vTaskResume( xTcTask ); } }

/* [DRV3946] CRC8 (poly=0x97, init=0xFF, MSB-first) over header+command */
static UInt8 drv_crc8( UInt8 b0, UInt8 b1 )
{
    /* [실측 확정] 커맨드 CRC = poly 0x2F, init 0xFF (CONFIG영역 CRC의 0x97과 다름!).
     * 4개 커맨드 CRC를 디바이스에서 역산해 전수탐색으로 확정 (CLR_FAULT[3C,80]=0x64 등). */
    UInt8 d[2]; UInt8 crc = 0xFFU; int i, k;
    d[0] = b0; d[1] = b1;
    for( i = 0; i < 2; i++ )
    {
        crc ^= d[i];
        for( k = 0; k < 8; k++ )
        {
            if( crc & 0x80U ) crc = (UInt8)((crc << 1) ^ 0x2FU);
            else              crc = (UInt8)(crc << 1);
        }
    }
    return crc;
}

/* [DRV3946] Command(브로드캐스트) 프레임 = header + cmd + CRC8, 24bit CSAAT.
 * 예: REINIT_NAD = DRV3946_Cmd24(0x3C, 0x40)  (CMD2=0x1E write, RE_INIT bit14) */
UInt16 DRV3946_Cmd24( UInt8 hdr, UInt8 cmd, UInt8 *echo )
{
    UInt8  crc = drv_crc8( hdr, cmd );
    UInt16 r0, r1, r2;
    UInt32 csr_save = SPI0_REGS->SPI_CSR[1];

    SPI0_REGS->SPI_CR  = SPI_CR_SPIDIS_Msk;   /* [필수] SPI 리셋 — 리드와 동일, 연속 프레임 정렬 보장 */
    SPI0_REGS->SPI_MR  = (SPI0_REGS->SPI_MR & ~SPI_MR_LLB_Msk & ~SPI_MR_DLYBCS_Msk)
                       | SPI_MR_MSTR_Msk | SPI_MR_PCS_NPCS1 | SPI_MR_MODFDIS_Msk | SPI_MR_DLYBCS(255);
    SPI0_REGS->SPI_CR  = SPI_CR_SPIEN_Msk;
    SPI0_REGS->SPI_CSR[1] = g_drvCpolNcpha
                          | SPI_CSR_BITS_8_BIT | SPI_CSR_SCBR(255) | SPI_CSR_DLYBS(64) | SPI_CSR_CSAAT_Msk;
    SYSTICK_DelayMs( 2 );                     /* 연속 프레임 회복시간 */
    (void)(SPI0_REGS->SPI_RDR);
    while( (SPI0_REGS->SPI_SR & SPI_SR_TDRE_Msk) == 0U ){}
    SPI0_REGS->SPI_TDR = hdr;
    while( (SPI0_REGS->SPI_SR & SPI_SR_RDRF_Msk) == 0U ){}
    r0 = (UInt16)(SPI0_REGS->SPI_RDR & 0xFFU);
    while( (SPI0_REGS->SPI_SR & SPI_SR_TDRE_Msk) == 0U ){}
    SPI0_REGS->SPI_TDR = cmd;
    while( (SPI0_REGS->SPI_SR & SPI_SR_RDRF_Msk) == 0U ){}
    r1 = (UInt16)(SPI0_REGS->SPI_RDR & 0xFFU);
    SPI0_REGS->SPI_CR = SPI_CR_LASTXFER_Msk;
    while( (SPI0_REGS->SPI_SR & SPI_SR_TDRE_Msk) == 0U ){}
    SPI0_REGS->SPI_TDR = crc;
    while( (SPI0_REGS->SPI_SR & SPI_SR_RDRF_Msk) == 0U ){}
    r2 = (UInt16)(SPI0_REGS->SPI_RDR & 0xFFU);

    SPI0_REGS->SPI_CSR[1] = csr_save;
    if( echo != NULL ) { *echo = (UInt8)r0; }
    return (UInt16)((r1 << 8) | r2);
}

/* [DRV3946 진단] raw 3바이트 커맨드 송신 (CRC를 명시적으로 지정) — 커맨드 CRC 무차별 대입용.
 * SPI 리셋+지연 포함 (연속 프레임 정렬 보장). 반환 = SDO b1:b2. */
UInt16 DRV3946_CmdRaw( UInt8 b0, UInt8 b1, UInt8 b2 )
{
    UInt16 r1, r2;
    UInt32 csr_save = SPI0_REGS->SPI_CSR[1];
    SPI0_REGS->SPI_CR  = SPI_CR_SPIDIS_Msk;
    SPI0_REGS->SPI_MR  = (SPI0_REGS->SPI_MR & ~SPI_MR_LLB_Msk & ~SPI_MR_DLYBCS_Msk)
                       | SPI_MR_MSTR_Msk | SPI_MR_PCS_NPCS1 | SPI_MR_MODFDIS_Msk | SPI_MR_DLYBCS(255);
    SPI0_REGS->SPI_CR  = SPI_CR_SPIEN_Msk;
    SPI0_REGS->SPI_CSR[1] = g_drvCpolNcpha
                          | SPI_CSR_BITS_8_BIT | SPI_CSR_SCBR(255) | SPI_CSR_DLYBS(64) | SPI_CSR_CSAAT_Msk;
    SYSTICK_DelayMs( 2 );
    (void)(SPI0_REGS->SPI_RDR);
    while( (SPI0_REGS->SPI_SR & SPI_SR_TDRE_Msk) == 0U ){} SPI0_REGS->SPI_TDR = b0;
    while( (SPI0_REGS->SPI_SR & SPI_SR_RDRF_Msk) == 0U ){} (void)(SPI0_REGS->SPI_RDR);
    while( (SPI0_REGS->SPI_SR & SPI_SR_TDRE_Msk) == 0U ){} SPI0_REGS->SPI_TDR = b1;
    while( (SPI0_REGS->SPI_SR & SPI_SR_RDRF_Msk) == 0U ){} r1 = (UInt16)(SPI0_REGS->SPI_RDR & 0xFFU);
    SPI0_REGS->SPI_CR = SPI_CR_LASTXFER_Msk;
    while( (SPI0_REGS->SPI_SR & SPI_SR_TDRE_Msk) == 0U ){} SPI0_REGS->SPI_TDR = b2;
    while( (SPI0_REGS->SPI_SR & SPI_SR_RDRF_Msk) == 0U ){} r2 = (UInt16)(SPI0_REGS->SPI_RDR & 0xFFU);
    SPI0_REGS->SPI_CSR[1] = csr_save;
    return (UInt16)((r1 << 8) | r2);
}

/* [DRV3946] Write 프레임 = 헤더(W=0) + DATA[15:0], CRC 없음 (Table 6-14).
 * SDO 응답 = 해당 주소의 "이전(prior)" 데이터 -> 쓰기 전 값 확인용으로 반환. */
UInt16 DRV3946_Write24( UInt8 reg5, UInt16 data, UInt8 *echo )
{
    UInt8  hdr = (UInt8)(((g_drvNode & 0x3U) << 6) | ((reg5 & 0x1FU) << 1));   /* node=g_drvNode, W=0 */
    UInt16 r0, r1, r2;
    UInt32 csr_save = SPI0_REGS->SPI_CSR[1];

    SPI0_REGS->SPI_CR  = SPI_CR_SPIDIS_Msk;   /* [필수] SPI 리셋 — 리드와 동일, 연속 프레임 정렬 보장 */
    SPI0_REGS->SPI_MR  = (SPI0_REGS->SPI_MR & ~SPI_MR_LLB_Msk & ~SPI_MR_DLYBCS_Msk)
                       | SPI_MR_MSTR_Msk | SPI_MR_PCS_NPCS1 | SPI_MR_MODFDIS_Msk | SPI_MR_DLYBCS(255);
    SPI0_REGS->SPI_CR  = SPI_CR_SPIEN_Msk;
    SPI0_REGS->SPI_CSR[1] = g_drvCpolNcpha
                          | SPI_CSR_BITS_8_BIT | SPI_CSR_SCBR(255) | SPI_CSR_DLYBS(64) | SPI_CSR_CSAAT_Msk;
    SYSTICK_DelayMs( 2 );                     /* 연속 프레임 회복시간 */
    (void)(SPI0_REGS->SPI_RDR);
    while( (SPI0_REGS->SPI_SR & SPI_SR_TDRE_Msk) == 0U ){}
    SPI0_REGS->SPI_TDR = hdr;
    while( (SPI0_REGS->SPI_SR & SPI_SR_RDRF_Msk) == 0U ){}
    r0 = (UInt16)(SPI0_REGS->SPI_RDR & 0xFFU);
    while( (SPI0_REGS->SPI_SR & SPI_SR_TDRE_Msk) == 0U ){}
    SPI0_REGS->SPI_TDR = (UInt8)(data >> 8);
    while( (SPI0_REGS->SPI_SR & SPI_SR_RDRF_Msk) == 0U ){}
    r1 = (UInt16)(SPI0_REGS->SPI_RDR & 0xFFU);
    SPI0_REGS->SPI_CR = SPI_CR_LASTXFER_Msk;
    while( (SPI0_REGS->SPI_SR & SPI_SR_TDRE_Msk) == 0U ){}
    SPI0_REGS->SPI_TDR = (UInt8)(data & 0xFFU);
    while( (SPI0_REGS->SPI_SR & SPI_SR_RDRF_Msk) == 0U ){}
    r2 = (UInt16)(SPI0_REGS->SPI_RDR & 0xFFU);

    SPI0_REGS->SPI_CSR[1] = csr_save;
    if( echo != NULL ) { *echo = (UInt8)r0; }
    return (UInt16)((r1 << 8) | r2);
}

/* [DRV3946] CRC8 (poly 0x97, init 0xFF) — 임의 길이 버퍼 (CONFIG 영역 CRC용) */
static UInt8 drv_crc8_buf( const UInt8 *d, int n )
{
    UInt8 crc = 0xFFU; int i, k;
    for( i = 0; i < n; i++ )
    {
        crc ^= d[i];
        for( k = 0; k < 8; k++ )
        {
            if( crc & 0x80U ) crc = (UInt8)((crc << 1) ^ 0x97U);
            else              crc = (UInt8)(crc << 1);
        }
    }
    return crc;
}

/* HP밸브 peak/hold 전류 레지스터값 (CONFIG_A0/A1 = (PC<<8)|HC)
 * I = (N+17)/272 x 20000 x 3V / R_IPROPI  [데이터시트 7.3.1]
 * ※실장 R_IPROPI=20k (리워크) 기준: PC=74(0x4A)->1.00A, HC=1(0x01)->0.198A.
 *   (56k는 데이터시트 유효범위 4.4~22.7k 초과라 실장 불가 -> 20k 채택. 0.07A는 20k에선 불가, 하한 ~0.19A)
 * ※주의: 0xED(237)는 56k기준 1A지만 20k에선 2.8A! 기본 PC=0x4A로 1.0A@20k 맞춤. */
UInt8 g_drvPC[2] = { 0x4A, 0x4A };
UInt8 g_drvHC[2] = { 0x01, 0x01 };

/* [DRV3946 wake — 데이터시트 8.3.2 Device Initialization]
 * CONFIG_A0..A6(0x10~0x16, 13바이트 CRC8 -> A6 하위바이트)
 * CONFIG_B0..B4(0x17~0x1B,  9바이트 CRC8 -> B4 하위바이트)
 * -> CLR_FAULT(CMD2 broadcast) -> STANDBY 진입.
 * 반환: STATUS1 (DEVICE_ID 확인용). pS0에 STATUS0. */
UInt16 DRV3946_Wake( UInt16 *pS0 )
{
    extern UInt16 DRV3946_Cmd24( UInt8 hdr, UInt8 cmd, UInt8 *echo );
    UInt16 cfgA[7], cfgB[5];
    UInt8  e;
    int    i, t;

    /* [수정] REINIT_NAD 제거 — 파워업 후 디바이스는 이미 INIT2(CONFIG 대기, POR=1, DEV_ERR=0).
     * REINIT_NAD는 ABIST를 재실행해 DEV_ERR을 유발했음. 데이터시트 §8.3.2대로 CONFIG→CLR_FAULT만 수행. */
    (void)e;

    /* CONFIG_A: A0/A1=peak/hold, 나머지 리셋 디폴트 (A2=2424,A3=0088,A4=130C,A5=8000) */
    cfgA[0] = (UInt16)(((UInt16)g_drvPC[0] << 8) | g_drvHC[0]);   /* CH1 PC/HC */
    cfgA[1] = (UInt16)(((UInt16)g_drvPC[1] << 8) | g_drvHC[1]);   /* CH2 PC/HC */
    /* cfgA[4]: 0x130C -> 0x530C, PIN_CONFIG(bit14)=1 설정.
     * pin5=EN1(CH1), pin24=EN2(CH2) 분리 enable로 -> 펌웨어 EN1/EN2 모델과 일치(CH2 동작).
     * (기본 PIN_CONFIG=0이면 pin24=DIS라 hpv on 2가 출력을 꺼버림) */
    cfgA[2] = 0x2424U;  cfgA[3] = 0x0088U;  cfgA[4] = 0x530CU;  cfgA[5] = 0x8000U;
    cfgA[6] = 0x0000U;                                            /* A6: slope=0, CRC는 아래서 */
    /* A0~A5 데이터 기록 (verify-retry로 landing 보장) */
    for( i = 0; i < 6; i++ )
    {
        for( t = 0; t < 3; t++ ) { (void)DRV3946_Write24( (UInt8)(0x10U + i), cfgA[i], &e );
                                   if( DRV3946_Read24( (UInt8)(0x10U + i), 0U, &e ) == cfgA[i] ) break; }
    }
    /* A6 CRC: peak/hold 값이 바뀌면 계산식 CRC가 디바이스와 안 맞는 경우가 있어 런타임 무차별 대입(B와 동일).
     * CONFIG_A_CRC_W(STATUS1 bit1)=0 되는 값을 찾아 cfgA[6]에 저장 -> 어떤 PC/HC값이든 통과. */
    for( i = 0; i < 256; i++ )
    {
        (void)DRV3946_Write24( 0x16U, (UInt16)i, &e );
        if( (DRV3946_Read24( 0x02U, g_drvNode, &e ) & 0x0002U) == 0U ) { cfgA[6] = (UInt16)i; break; }
    }

    /* CONFIG_B3(0x1A): 리셋0x8000 + bit13 STARTUP_ABIST_BYPASS + bit9 RIPROPI_W_BYPASS = 0xA200
     * -> IPROPI 미실장(오픈)이어도 INIT2->STANDBY 통과. */
    /* cfgB[1]: bit13 SPI_WD_DIS=1 추가(0x0040->0x2040) — SPI 유휴 시 워치독 폴트/경고로
     * 출력이 꺼지거나 STANDBY 이탈하는 것 방지(주기적 SPI 없이도 출력 유지). */
    cfgB[0] = 0x2623U;  cfgB[1] = 0x2040U;  cfgB[2] = 0x0B0BU;  cfgB[3] = 0xA200U;  cfgB[4] = 0x0000U;
    for( i = 0; i < 4; i++ ) { (void)DRV3946_Write24( (UInt8)(0x17U + i), cfgB[i], &e ); }
    /* B의 CRC는 A(hi-first 13B)와 바이트구성이 달라 계산이 안 맞음 -> B4(0x1B) 런타임 무차별 대입.
     * CONFIG_B_CRC_W(STATUS1 bit0)=0 되는 값을 찾아 cfgB[4]에 저장. */
    for( i = 0; i < 256; i++ )
    {
        (void)DRV3946_Write24( 0x1BU, (UInt16)i, &e );
        if( (DRV3946_Read24( 0x02U, g_drvNode, &e ) & 0x0001U) == 0U ) { cfgB[4] = (UInt16)i; break; }
    }

    /* A/B CRC 검증-재적용 루프 (한쪽 기록이 다른쪽 경고를 재설정할 수 있어 둘 다 0될 때까지) */
    for( i = 0; i < 5; i++ )
    {
        UInt16 s = DRV3946_Read24( 0x02U, g_drvNode, &e );
        if( (s & 0x0003U) == 0U ) { break; }
        if( s & 0x0002U ) { (void)DRV3946_Write24( 0x16U, cfgA[6], &e ); }   /* A_CRC_W -> A6 재적용 */
        if( s & 0x0001U ) { (void)DRV3946_Write24( 0x1BU, cfgB[4], &e ); }   /* B_CRC_W -> B4 재적용 */
    }

    /* CLR_FAULT (CMD2 broadcast: hdr=0x3C, cmd=0x80) -> 무경고면 STANDBY 진입 */
    (void)DRV3946_Cmd24( 0x3CU, 0x80U, &e );

    /* STATUS0(0x01)/STATUS1(0x02) 리드백 */
    if( pS0 != NULL ) { *pS0 = DRV3946_Read24( 0x01U, g_drvNode, &e ); }
    return DRV3946_Read24( 0x02U, g_drvNode, &e );
}

/* [DRV3946] 채널 ON/OFF — CMD1(0x1D) 커맨드 프레임(CRC).
 * 커맨드 바이트 = CMD1[15:8]: bit7=CLR_FAULT, bit6=LOCK, [5:3]=CH1_CTRL, [2:0]=CH2_CTRL.
 * CHx_CTRL: 0x0=Shutoff, 0x2=Turn ON(내부 전류 레귤레이션). EN핀과 AND 조건. */
UInt16 DRV3946_ChCtrl( UInt8 ch1, UInt8 ch2 )
{
    extern UInt16 DRV3946_Cmd24( UInt8 hdr, UInt8 cmd, UInt8 *echo );
    UInt8 e;
    UInt8 cmd = (UInt8)(((ch1 & 7U) << 3) | (ch2 & 7U));
    return DRV3946_Cmd24( (UInt8)(((g_drvNode & 0x3U) << 6) | (0x1DU << 1)), cmd, &e );   /* CMD1 write, node=g_drvNode */
}

/* [HP/LP SV1 반복 구동] HP=node0/ch1 정전류 0.8A 목표, LP=Valve01 DMM 확인용 100% high 3s / low 3s.
 * I = (N+17)/272 * 3A @ R_IPROPI=20k -> N=56이면 약 0.805A.
 * PC/HC를 같은 값으로 설정해 peak 이후 hold도 동일 전류로 유지한다. */
#define HPV_SV1_CYCLE_NODE              0U
#define HPV_SV1_CYCLE_CHCTRL_CURRENT    0x2U
#define HPV_SV1_CYCLE_REG_800MA         56U
#define HPV_SV1_CYCLE_ON_MS             1000UL
#define HPV_SV1_CYCLE_OFF_MS            1000UL
#define LPV_SV1_PWM_PERIOD              750U
#define LPV_SV1_HIGH_MS                 3000UL
#define LPV_SV1_LOW_MS                  3000UL
#define LPV_RTN_PD12_MASK               (1UL << 12)

static volatile UInt8 s_hpvSv1CycleEnable = 0U;
static volatile UInt8 s_hpvSv1CycleRestart = 0U;
static UInt8 s_hpvSv1CycleOn = 0U;
static UInt8 s_lpvSv1CycleOn = 0U;
static TickType_t s_hpvSv1CycleTick = 0U;
static TickType_t s_lpvSv1CycleTick = 0U;

static void LpvSv1CyclePinPwm( void )
{
    PIOA_REGS->PIO_ABCDSR[0] &= ~LPV01_GPIO_PA0_MASK;  /* PA0 peripheral A = PWM0_PWMH0 */
    PIOA_REGS->PIO_ABCDSR[1] &= ~LPV01_GPIO_PA0_MASK;
    PIOA_REGS->PIO_PDR = LPV01_GPIO_PA0_MASK;
}

static void LpvSv1CyclePinOff( void )
{
    PIOA_REGS->PIO_PER  = LPV01_GPIO_PA0_MASK;
    PIOA_REGS->PIO_OER  = LPV01_GPIO_PA0_MASK;
    PIOA_REGS->PIO_CODR = LPV01_GPIO_PA0_MASK;
}

static void LpvSv1CycleSetRtnEnable( UInt8 ucEnable )
{
    PIOD_REGS->PIO_PER = LPV_RTN_PD12_MASK;
    PIOD_REGS->PIO_OER = LPV_RTN_PD12_MASK;
    if( ucEnable != 0U ) { PIOD_REGS->PIO_SODR = LPV_RTN_PD12_MASK; }  /* PD12 high = RTN enable */
    else                 { PIOD_REGS->PIO_CODR = LPV_RTN_PD12_MASK; }  /* PD12 low = RTN off */
}

static void LpvSv1CycleSetOff( void )
{
    PWM0_ChannelsStop( PWM_CHANNEL_0_MASK );
    PWM0_REGS->PWM_CH_NUM[0].PWM_CDTY = 0U;
    LpvSv1CyclePinOff();
    LpvSv1CycleSetRtnEnable( 0U );
    s_lpvSv1CycleOn = 0U;
}

static void LpvSv1CycleSetOn( void )
{
    PWM0_REGS->PWM_CH_NUM[0].PWM_CPRD = LPV_SV1_PWM_PERIOD;
    PWM0_REGS->PWM_CH_NUM[0].PWM_CDTY = 0U;      /* LPV1 H output: 100% high for DMM */
    LpvSv1CycleSetRtnEnable( 1U );
    LpvSv1CyclePinPwm();
    PWM0_ChannelsStart( PWM_CHANNEL_0_MASK );
    s_lpvSv1CycleOn = 1U;
}

static void HpvSv1CycleApplyConfig( void )
{
    UInt16 s0 = 0U;

    g_drvNode = HPV_SV1_CYCLE_NODE;
    g_drvPC[0] = HPV_SV1_CYCLE_REG_800MA;
    g_drvHC[0] = HPV_SV1_CYCLE_REG_800MA;
    (void)DRV3946_Wake( &s0 );
}

static void HpvSv1CycleSetOff( void )
{
    UInt8 e = 0U;

    g_drvNode = HPV_SV1_CYCLE_NODE;
    DRV3946Q1_EN1_Clear();
    (void)DRV3946_ChCtrl( 0U, 0U );
    (void)DRV3946_Read24( 0x01U, g_drvNode, &e );
    s_hpvSv1CycleOn = 0U;
}

static void HpvSv1CycleSetOn( void )
{
    UInt8 e = 0U;
    UInt16 s0;

    g_drvNode = HPV_SV1_CYCLE_NODE;
    s0 = DRV3946_Read24( 0x01U, g_drvNode, &e );
    if( (s0 & 0x2000U) != 0U || s0 == 0xFFFFU )
    {
        HpvSv1CycleApplyConfig();
    }

    DRV3946Q1_EN1_OutputEnable();
    DRV3946Q1_EN2_OutputEnable();
    DRV3946Q1_KILL_ALL_OutputEnable();
    DRV3946Q1_KILL_ALL_Set();
    DRV3946Q1_EN2_Clear();
    DRV3946Q1_EN1_Set();
    (void)DRV3946_ChCtrl( HPV_SV1_CYCLE_CHCTRL_CURRENT, 0U );
    (void)DRV3946_Read24( 0x01U, g_drvNode, &e );
    s_hpvSv1CycleOn = 1U;
}

void HpvSv1CycleStart( void )
{
    s_hpvSv1CycleRestart = 1U;
    s_hpvSv1CycleEnable = 1U;
}

void HpvSv1CycleStop( void )
{
    s_hpvSv1CycleEnable = 0U;
    s_hpvSv1CycleRestart = 0U;
    HpvSv1CycleSetOff();
    LpvSv1CycleSetOff();
}

UInt8 HpvSv1CycleIsEnabled( void )
{
    return s_hpvSv1CycleEnable;
}

static void HpvSv1CycleService( void )
{
    const TickType_t xOnTicks = pdMS_TO_TICKS( HPV_SV1_CYCLE_ON_MS );
    const TickType_t xOffTicks = pdMS_TO_TICKS( HPV_SV1_CYCLE_OFF_MS );
    const TickType_t xLpvHighTicks = pdMS_TO_TICKS( LPV_SV1_HIGH_MS );
    const TickType_t xLpvLowTicks = pdMS_TO_TICKS( LPV_SV1_LOW_MS );
    TickType_t xNow = xTaskGetTickCount();

    if( s_hpvSv1CycleEnable == 0U )
    {
        if( s_hpvSv1CycleOn != 0U )
        {
            HpvSv1CycleSetOff();
        }
        if( s_lpvSv1CycleOn != 0U )
        {
            LpvSv1CycleSetOff();
        }
        return;
    }

    if( s_hpvSv1CycleRestart != 0U )
    {
        s_hpvSv1CycleRestart = 0U;
        HpvSv1CycleApplyConfig();
        HpvSv1CycleSetOn();
        LpvSv1CycleSetOn();
        s_hpvSv1CycleTick = xNow;
        s_lpvSv1CycleTick = xNow;
        return;
    }

    if( s_hpvSv1CycleOn != 0U )
    {
        if( (xNow - s_hpvSv1CycleTick) >= xOnTicks )
        {
            HpvSv1CycleSetOff();
            s_hpvSv1CycleTick = xNow;
        }
    }
    else
    {
        if( (xNow - s_hpvSv1CycleTick) >= xOffTicks )
        {
            HpvSv1CycleSetOn();
            s_hpvSv1CycleTick = xNow;
        }
    }

    if( s_lpvSv1CycleOn != 0U )
    {
        if( (xNow - s_lpvSv1CycleTick) >= xLpvHighTicks )
        {
            LpvSv1CycleSetOff();
            s_lpvSv1CycleTick = xNow;
        }
    }
    else
    {
        if( (xNow - s_lpvSv1CycleTick) >= xLpvLowTicks )
        {
            LpvSv1CycleSetOn();
            s_lpvSv1CycleTick = xNow;
        }
    }
}

/* 16-bit SPI 프레임 교환 (HW NPCS1 CS, 프레임마다 자동 해제) */
UInt16 DRV3946_Xfer16( UInt16 uiOut )
{
    (void)(SPI0_REGS->SPI_RDR);                                     /* flush */
    while( (SPI0_REGS->SPI_SR & SPI_SR_TDRE_Msk) == 0U ){}
    SPI0_REGS->SPI_TDR = uiOut;
    while( (SPI0_REGS->SPI_SR & SPI_SR_RDRF_Msk) == 0U ){}
    return (UInt16)(SPI0_REGS->SPI_RDR & SPI_RDR_RD_Msk);
}

/**
 * @fn OpuTask
 * @brief OPU Task 함수
 * @param pvParameters Task 매개변수
 * @return void
 * @date 2025-12-18
 */
/* [안전] 모든 액추에이터 강제 OFF. 폴트/리셋/명령 시 호출.
 * 폴트 컨텍스트 안전(레지스터 직접 접근, RTOS/printf 호출 없음). */
void EnterSafeState( void )
{
    /* HP 밸브(DRV3946): EN 해제 + KILL_ALL low(외부 AND게이트로 출력 강제차단) */
    DRV3946Q1_EN1_Clear();
    DRV3946Q1_EN2_Clear();
    DRV3946Q1_KILL_ALL_Clear();
    LpvSv1CycleSetOff();
    /* Micro 밸브: PWM0 ch0~3, PWM1 ch0~1 정지 */
    PWM0_ChannelsStop( PWM_CHANNEL_0_MASK | PWM_CHANNEL_1_MASK | PWM_CHANNEL_2_MASK | PWM_CHANNEL_3_MASK );
    PWM1_ChannelsStop( PWM_CHANNEL_0_MASK | PWM_CHANNEL_1_MASK );
    /* 히터(TC3): duty 0 */
    TC3_REGS->TC_CHANNEL[0].TC_RA = 0U;
    TC3_REGS->TC_CHANNEL[0].TC_RB = 0U;
}

void OpuTask( void *pvParameters )
{
#if LPV01_AUTO_TEST_ENABLE
    const TickType_t xLpv01TestDelay = pdMS_TO_TICKS( LPV01_AUTO_TEST_DELAY_MS );
    const TickType_t xHpv01OnDelay = pdMS_TO_TICKS( HPV_SV1_CYCLE_ON_MS );
    const TickType_t xHpv01OffDelay = pdMS_TO_TICKS( HPV_SV1_CYCLE_OFF_MS );
    const TickType_t xAutoTestPollDelay = pdMS_TO_TICKS( SV01_AUTO_TEST_POLL_MS );
    TickType_t xNow;
    TickType_t xLpv01Tick;
    TickType_t xHpv01Tick;
    UInt8 ucLpv01On = 0U;
    UInt8 ucHpv01On = 0U;
#else
    const TickType_t x10ms = pdMS_TO_TICKS( DELAY_10_MSECOND );

    /* 메인 주기 카운트 */
	UInt16 usMainCnt = 0;
#endif
	
    /* 링버퍼 초기화 */
	RingBufferInit();

    /* AFEC 추가 채널(SEN_P5V/SEN_VDD/AD6) 및 PC29/PC30/PC31 아날로그 전환
     * !! AdcTask 생성(TaskCreate) 전에 호출해야 CH6가 enable됨 (레이스 방지) */
    AFEC_Init();

    /* PWM 초기화 */
    PWM_Init();

    /* 히터(TC3) 초기화 */
    Heater_Init();

    /* HP 밸브 드라이버 SPI0 초기화 */
    DRV3946_SPI_Init();

    /* DRV3946-Q1 제어 핀 초기 안전 상태
     * EN_ALL = EN AND KILL_ALL (회로도 U3 AND 게이트) -> KILL_ALL High 시 출력 허용.
     * EN1/EN2/KILL_ALL 토글은 hpv 명령(en1/en2/kill)으로 검증 */
    DRV3946Q1_EN1_Clear();              // 초기 OFF (안전)
    DRV3946Q1_EN2_Clear();              // 초기 OFF (안전)
    DRV3946Q1_KILL_ALL_Set();           // 출력 허용 (검증용)

    LpSolValve_Init();
    HpSolValve_Init();

    /* Task 생성 */
	TaskCreate();

#if LPV01_AUTO_TEST_ENABLE
    PWM0_ChannelsStop( PWM_CHANNEL_0_MASK );
    PIOA_REGS->PIO_PER  = LPV01_GPIO_PA0_MASK;    /* PA0을 PWM peripheral에서 GPIO로 회수 */
    PIOA_REGS->PIO_OER  = LPV01_GPIO_PA0_MASK;    /* GPIO output */
    PIOA_REGS->PIO_CODR = LPV01_GPIO_PA0_MASK;    /* OFF 시작 */
    PIOD_REGS->PIO_PER  = LPV01_RTN_PD12_MASK;    /* PD12 = LP_Valve_CTRL_ALL */
    PIOD_REGS->PIO_OER  = LPV01_RTN_PD12_MASK;
    PIOD_REGS->PIO_CODR = LPV01_RTN_PD12_MASK;    /* OFF 시작 */

    HpvSv1CycleApplyConfig();                      /* HP_Valve01 = node0/ch1, 0.8A current-reg */
    HpvSv1CycleSetOff();

    xNow = xTaskGetTickCount();
    xLpv01Tick = xNow;
    xHpv01Tick = xNow;

    PIOD_REGS->PIO_SODR = LPV01_RTN_PD12_MASK;    /* LSV0 RTN enable */
    PIOA_REGS->PIO_SODR = LPV01_GPIO_PA0_MASK;    /* LSV0 High */
    ucLpv01On = 1U;

    HpvSv1CycleSetOn();                            /* HPV0 0.8A current-reg ON */
    ucHpv01On = 1U;

    while(1)
    {
        WDT_REGS->WDT_CR = WDT_CR_KEY_PASSWD | WDT_CR_WDRSTT_Msk;
        xNow = xTaskGetTickCount();

        if( ucHpv01On != 0U )
        {
            if( (xNow - xHpv01Tick) >= xHpv01OnDelay )
            {
                HpvSv1CycleSetOff();
                ucHpv01On = 0U;
                xHpv01Tick = xNow;
            }
        }
        else
        {
            if( (xNow - xHpv01Tick) >= xHpv01OffDelay )
            {
                HpvSv1CycleSetOn();
                ucHpv01On = 1U;
                xHpv01Tick = xNow;
            }
        }

        if( ucLpv01On != 0U )
        {
            if( (xNow - xLpv01Tick) >= xLpv01TestDelay )
            {
                PIOA_REGS->PIO_CODR = LPV01_GPIO_PA0_MASK;    /* LSV0 Low */
                PIOD_REGS->PIO_CODR = LPV01_RTN_PD12_MASK;    /* LSV0 RTN off */
                ucLpv01On = 0U;
                xLpv01Tick = xNow;
            }
        }
        else
        {
            if( (xNow - xLpv01Tick) >= xLpv01TestDelay )
            {
                PIOD_REGS->PIO_SODR = LPV01_RTN_PD12_MASK;    /* LSV0 RTN enable */
                PIOA_REGS->PIO_SODR = LPV01_GPIO_PA0_MASK;    /* LSV0 High */
                ucLpv01On = 1U;
                xLpv01Tick = xNow;
            }
        }

        vTaskDelay( xAutoTestPollDelay );
    }
#else
    while(1)
    {
        /* [안전] 워치독 refresh (16s 타임아웃, 본 루프 50ms 주기 -> 320x 여유) */
        WDT_REGS->WDT_CR = WDT_CR_KEY_PASSWD | WDT_CR_WDRSTT_Msk;

        HpvSv1CycleService();

        /* TC 출력 */
        TcPrint( usMainCnt );

        /* ADC 출력 */
        AdcPrint( usMainCnt );

        /* 메인 주기 관리 */
        usMainCnt++;

        if( usMainCnt == 50 )
        {
            usMainCnt = 0;
        }
    	vTaskDelay( (x10ms*5) );
    }
#endif
    vTaskDelete( NULL );
}
