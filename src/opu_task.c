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
#include "uartcomm.h"
#include "statemachine.h"

/*==============================================================================
 * Gloabal Variables
 *============================================================================*/
/* --- handler  --- */
static TaskHandle_t xTcTask;		// TC task handler
static TaskHandle_t xRsTask;		// RS task handler
static TaskHandle_t xAdcTask;		// ADC task handler
static TaskHandle_t xOpuTaskSelf;   // OPU task handler for timer notification

/*==============================================================================
 * Gloabal Function
 *============================================================================*/
void OpuTask( void *pvParameters );
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

#define LPV01_GPIO_PA0_MASK         (1UL << 0)    /* LP_Valve01 = PA0 / PWM0_PWMH0 */
#define LPV01_RTN_PD12_MASK         (1UL << 12)   /* LP_Valve_CTRL_ALL = PD12 / TPD2017_KILL_ALL */
#define TCMD_PACKET_HEADER          "$iGRVT50"
#define TCMD_COMMAND_SVCON          "SVCON"
#define TCMD_COMMAND_TMREQ          "TMREQ"
#define TCMD_COMMAND_DIAG           "DIAG"
#define TCMD_COMMAND_MODE           "MODE"
#define TCMD_ACK_DATA               "Ack"
#define TCMD_RX_LINE_SIZE           512U
#define TCMD_TMREQ_FIELD_COUNT      2U
#define TCMD_DIAG_FIELD_COUNT       2U
#define TCMD_MODE_FIELD_COUNT       3U
#define TCMD_SVCON_FIELD_COUNT      (2U + LPSOLVALVE_CHANNEL_COUNT + HPSOLVALVE_CHANNEL_COUNT)
#define TCMD_MAX_FIELD_COUNT        TCMD_SVCON_FIELD_COUNT
#define TC_TASK_PERIOD_MS           1000UL
#define ADC_TASK_PERIOD_MS          50UL
#define OPU_TIMER_TICK_MS           10UL
#define OPU_TIMER_MCK_HZ            (CPU_CLOCK_FREQUENCY / 2UL)
#define OPU_TIMER_CLOCK_HZ          (OPU_TIMER_MCK_HZ / 128UL)
#define OPU_TIMER_RC_COUNT          (((OPU_TIMER_CLOCK_HZ * OPU_TIMER_TICK_MS) + 500UL) / 1000UL)
#define OPU_TIMER_CALLBACK_MAX      8U
#define RSTASK_NOTIFY_RX_READY      (1UL << 0)
#define RSTASK_NOTIFY_TM_EVENT      (1UL << 1)
#define TC_TASK_PRIORITY            (tskIDLE_PRIORITY)
#define ADC_TASK_PRIORITY           (tskIDLE_PRIORITY)
#define RS_TASK_PRIORITY            (tskIDLE_PRIORITY + 3U)

/*==============================================================================
 * Local Function
 *============================================================================*/
/*-------- TC Processing --------*/
static void TcTask(void *p);

/*-------- ADC Processing --------*/
static void AdcTask(void *p);

/*-------- RS422 Processing --------*/
static void RsTask(void *p);
static void RsTask_SendSensorPacket( void );
static void RsTask_SendAckPacket( void );
static void RsTask_SendDiagPacket( void );
static void RsTask_ProcessRx( void );
static void RsTask_ProcessTelecommandLine( char *line );
static UInt8 RsTask_ParseModeField( const char *text, eStateMachineMode *mode );
static void RsTask_ApplyTelecommand( const UInt8 *lpvState, const UInt8 *hpvState );
static void Opu_10msCallback( void *context );
static void Opu_100msCallback( void *context );
static void Opu_1000msCallback( void *context );
static void OpuTimer_Init( void );
static void OpuTimer_ClearCallbacks( void );
static void OpuTimer_RegisterDefaultCallbacks( void );
static UInt32 OpuTimer_MsToTicks( UInt32 periodMs );
static void OpuTimer_ServiceCallbacks( UInt32 elapsedTicks );
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
    const TickType_t xPeriodTicks = pdMS_TO_TICKS( TC_TASK_PERIOD_MS );
    TickType_t xLastWakeTime;

    ADS1263_Init();
    xLastWakeTime = xTaskGetTickCount();

	while(1)
	{
        /* [수정] ADS1263은 U3 1개만 실장. 이전 2칩(ADS#2/CS=PD28)은 펌웨어 가공 -> 제거. */
        ADS1263_SetDevice( 1 );
        stTcTemp[0].fTempCh1 = ADS1263_GetTemperatureTask( 0 );   /* AIN0/1 = TC_SEN1 */
        stTcTemp[0].fTempCh2 = ADS1263_GetTemperatureTask( 1 );   /* AIN2/3 = TC_SEN2 */
        stTcTemp[0].fTempCh3 = ADS1263_GetTemperatureTask( 2 );   /* AIN4/5 = TC_SEN3 */
        stTcTemp[0].fTempCh4 = ADS1263_GetTemperatureTask( 3 );   /* AIN6/7 = TC_SEN4 */
        stTcTemp[0].fTempCJ  = ADS1263_GetTemperatureTask( 4 );   /* 내부 die온도 = CJ */
        vTaskDelayUntil( &xLastWakeTime, xPeriodTicks );
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
    const TickType_t xPeriodTicks = pdMS_TO_TICKS( ADC_TASK_PERIOD_MS );
    TickType_t xLastWakeTime = xTaskGetTickCount();

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

        vTaskDelayUntil( &xLastWakeTime, xPeriodTicks );
	}
}

static void RsTask_SendSensorPacket( void )
{
    sSensorScan stScan;
    UInt32 uiPacketTick;
    const char *pcModeName;
    char cTxMsg[240];
    int iTxLen;

    Sensor_GetScan( &stScan );
    uiPacketTick = (UInt32)xTaskGetTickCount();
    pcModeName = StateMachine_GetModeName( StateMachine_GetMode() );

    iTxLen = snprintf( cTxMsg, sizeof(cTxMsg),
                       "$iGRVT50,%lu,%s,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld\r\n",
                       (unsigned long)uiPacketTick,
                       pcModeName,
                       (long)stScan.pt.adcMilliVolt[SENSOR_PT1_INDEX],
                       (long)stScan.pt.adcMilliVolt[1U],
                       (long)stScan.pt.adcMilliVolt[2U],
                       (long)stScan.pt.adcMilliVolt[3U],
                       (long)stScan.pt.adcMilliVolt[4U],
                       (long)stScan.pt.adcMilliVolt[5U],
                       (long)stScan.pt.adcMilliVolt[6U],
                       (long)stScan.pt.adcMilliVolt[7U],
                       (long)stScan.pt.adcMilliVolt[8U],
                       (long)stScan.tc.microVolt[SENSOR_TC1_INDEX],
                       (long)stScan.tc.microVolt[1U],
                       (long)stScan.tc.microVolt[2U],
                       (long)stScan.tc.microVolt[3U] );
    if( iTxLen < 0 )
    {
        return;
    }
    if( iTxLen >= (int)sizeof(cTxMsg) )
    {
        iTxLen = (int)sizeof(cTxMsg) - 1;
    }

    UartComm_SendBlocking( cTxMsg, (UInt32)iTxLen );
}

static void RsTask_SendAckPacket( void )
{
    UartComm_SendStringBlocking( TCMD_PACKET_HEADER "," TCMD_ACK_DATA "\r\n" );
}

static UInt8 RsTask_ParseBinaryField( const char *text, UInt8 *value )
{
    if( (text == NULL) || (value == NULL) )
    {
        return 0U;
    }

    if( ((text[0] == '0') || (text[0] == '1')) && (text[1] == '\0') )
    {
        *value = (UInt8)(text[0] - '0');
        return 1U;
    }

    return 0U;
}

static UInt8 RsTask_ParseModeField( const char *text, eStateMachineMode *mode )
{
    if( (text == NULL) || (mode == NULL) )
    {
        return 0U;
    }

    if( (strcmp( text, "0" ) == 0) ||
        (strcmp( text, "INIT" ) == 0) ||
        (strcmp( text, "init" ) == 0) ||
        (strcmp( text, "init_mode" ) == 0) )
    {
        *mode = STATE_MACHINE_INIT_MODE;
        return 1U;
    }

    if( (strcmp( text, "1" ) == 0) ||
        (strcmp( text, "NORMAL" ) == 0) ||
        (strcmp( text, "normal" ) == 0) ||
        (strcmp( text, "normal_mode" ) == 0) )
    {
        *mode = STATE_MACHINE_NORMAL_MODE;
        return 1U;
    }

    if( (strcmp( text, "2" ) == 0) ||
        (strcmp( text, "RUN" ) == 0) ||
        (strcmp( text, "run" ) == 0) ||
        (strcmp( text, "run_mode" ) == 0) )
    {
        *mode = STATE_MACHINE_RUN_MODE;
        return 1U;
    }

    if( (strcmp( text, "3" ) == 0) ||
        (strcmp( text, "DIAG" ) == 0) ||
        (strcmp( text, "diag" ) == 0) ||
        (strcmp( text, "DIAGNOSTIC" ) == 0) ||
        (strcmp( text, "diagnostic" ) == 0) ||
        (strcmp( text, "diagnostic_mode" ) == 0) )
    {
        *mode = STATE_MACHINE_DIAGNOSTIC_MODE;
        return 1U;
    }

    return 0U;
}

static void RsTask_ApplyTelecommand( const UInt8 *lpvState, const UInt8 *hpvState )
{
    UInt8 i;

    if( (lpvState == NULL) || (hpvState == NULL) )
    {
        return;
    }

    for( i = 0U; i < LPSOLVALVE_CHANNEL_COUNT; i++ )
    {
        LpSolValve_Set( (UInt8)(i + 1U), lpvState[i] );
    }

    for( i = 0U; i < HPSOLVALVE_CHANNEL_COUNT; i++ )
    {
        HpSolValve_Set( (UInt8)(i + 1U), hpvState[i] );
    }
}

static char s_tcmdRxLine[TCMD_RX_LINE_SIZE];
static UInt16 s_tcmdRxLen = 0U;
static volatile UInt32 s_tcmdRxLineCount = 0U;
static volatile UInt32 s_tcmdRxNoHeaderCount = 0U;
static volatile UInt32 s_tcmdRxHeaderCount = 0U;
static volatile UInt32 s_tcmdRxBadFieldCount = 0U;
static volatile UInt32 s_tcmdRxBadBinaryCount = 0U;
static volatile UInt32 s_tcmdRxUnknownCommandCount = 0U;
static volatile UInt32 s_tcmdTmreqCount = 0U;
static volatile UInt32 s_tcmdSvconCount = 0U;
static volatile UInt32 s_tcmdDiagCount = 0U;
static volatile UInt32 s_tcmdModeCount = 0U;
static volatile UInt32 s_tcmdAckSentCount = 0U;
static volatile UInt32 s_tcmdRxOverflowCount = 0U;
static volatile UInt16 s_tcmdLastLineLen = 0U;
static UInt16 s_tcmdRxSkipLen = 0U;
static volatile UInt32 s_opuTimerTickCount = 0U;
static volatile UInt32 s_opu10msCallbackCount = 0U;
static volatile UInt32 s_opu100msCallbackCount = 0U;
static volatile UInt32 s_opu1000msCallbackCount = 0U;

typedef struct
{
    UInt8 used;
    UInt32 periodTicks;
    UInt32 elapsedTicks;
    OpuTimerCallback callback;
    void *context;
} sOpuTimerCallbackEntry;

static sOpuTimerCallbackEntry s_opuTimerCallbacks[OPU_TIMER_CALLBACK_MAX];

static void RsTask_SendDiagPacket( void )
{
    UInt32 uiPacketTick;
    char cTxMsg[320];
    int iTxLen;

    uiPacketTick = (UInt32)xTaskGetTickCount();
    iTxLen = snprintf( cTxMsg, sizeof(cTxMsg),
                       "$iGRVT50,DIAG,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%u\r\n",
                       (unsigned long)uiPacketTick,
                       (unsigned long)UartComm_GetRxByteCount(),
                       (unsigned long)UartComm_GetRxDropCount(),
                       (unsigned long)UartComm_GetRxErrorCount(),
                       (unsigned long)s_tcmdRxLineCount,
                       (unsigned long)s_tcmdRxNoHeaderCount,
                       (unsigned long)s_tcmdRxHeaderCount,
                       (unsigned long)s_tcmdRxBadFieldCount,
                       (unsigned long)s_tcmdRxBadBinaryCount,
                       (unsigned long)s_tcmdRxUnknownCommandCount,
                       (unsigned long)s_tcmdTmreqCount,
                       (unsigned long)s_tcmdSvconCount,
                       (unsigned long)s_tcmdAckSentCount,
                       (unsigned long)s_tcmdRxOverflowCount,
                       (unsigned)s_tcmdLastLineLen );
    if( iTxLen < 0 )
    {
        return;
    }
    if( iTxLen >= (int)sizeof(cTxMsg) )
    {
        iTxLen = (int)sizeof(cTxMsg) - 1;
    }

    UartComm_SendBlocking( cTxMsg, (UInt32)iTxLen );
}

static void RsTask_ProcessTelecommandLine( char *line )
{
    char *start;
    char *next;
    char *token;
    char *fields[TCMD_MAX_FIELD_COUNT + 1U];
    UInt8 fieldCount = 0U;
    UInt8 lpvState[LPSOLVALVE_CHANNEL_COUNT];
    UInt8 hpvState[HPSOLVALVE_CHANNEL_COUNT];
    eStateMachineMode requestedMode;
    UInt8 i;

    if( line == NULL )
    {
        return;
    }

    s_tcmdRxLineCount++;
    s_tcmdLastLineLen = (UInt16)strlen( line );

    start = strstr( line, TCMD_PACKET_HEADER );
    if( start == NULL )
    {
        s_tcmdRxNoHeaderCount++;
        return;
    }
    s_tcmdRxHeaderCount++;

    while( (next = strstr( start + 1, TCMD_PACKET_HEADER )) != NULL )
    {
        start = next;
    }

    token = strtok( start, "," );
    while( (token != NULL) && (fieldCount < (TCMD_MAX_FIELD_COUNT + 1U)) )
    {
        fields[fieldCount] = token;
        fieldCount++;
        token = strtok( NULL, "," );
    }

    if( fieldCount < TCMD_TMREQ_FIELD_COUNT )
    {
        s_tcmdRxBadFieldCount++;
        return;
    }

    if( strcmp( fields[0], TCMD_PACKET_HEADER ) != 0 )
    {
        s_tcmdRxNoHeaderCount++;
        return;
    }

    if( strcmp( fields[1], TCMD_COMMAND_TMREQ ) == 0 )
    {
        if( fieldCount == TCMD_TMREQ_FIELD_COUNT )
        {
            s_tcmdTmreqCount++;
            RsTask_SendSensorPacket();
        }
        else
        {
            s_tcmdRxBadFieldCount++;
        }
        return;
    }

    if( strcmp( fields[1], TCMD_COMMAND_DIAG ) == 0 )
    {
        if( fieldCount == TCMD_DIAG_FIELD_COUNT )
        {
            s_tcmdDiagCount++;
            RsTask_SendDiagPacket();
        }
        else
        {
            s_tcmdRxBadFieldCount++;
        }
        return;
    }

    if( strcmp( fields[1], TCMD_COMMAND_MODE ) == 0 )
    {
        if( (fieldCount == TCMD_MODE_FIELD_COUNT) &&
            (RsTask_ParseModeField( fields[2], &requestedMode ) != 0U) &&
            (StateMachine_RequestMode( requestedMode ) != 0U) )
        {
            s_tcmdModeCount++;
            s_tcmdAckSentCount++;
            RsTask_SendAckPacket();
        }
        else
        {
            s_tcmdRxBadFieldCount++;
        }
        return;
    }

    if( strcmp( fields[1], TCMD_COMMAND_SVCON ) != 0 )
    {
        s_tcmdRxUnknownCommandCount++;
        return;
    }

    if( fieldCount != TCMD_SVCON_FIELD_COUNT )
    {
        s_tcmdRxBadFieldCount++;
        return;
    }

    for( i = 0U; i < LPSOLVALVE_CHANNEL_COUNT; i++ )
    {
        if( RsTask_ParseBinaryField( fields[2U + i], &lpvState[i] ) == 0U )
        {
            s_tcmdRxBadBinaryCount++;
            return;
        }
    }

    for( i = 0U; i < HPSOLVALVE_CHANNEL_COUNT; i++ )
    {
        if( RsTask_ParseBinaryField( fields[2U + LPSOLVALVE_CHANNEL_COUNT + i], &hpvState[i] ) == 0U )
        {
            s_tcmdRxBadBinaryCount++;
            return;
        }
    }

    s_tcmdSvconCount++;
    s_tcmdAckSentCount++;
    /* Ack means the SVCON packet was accepted, not that valve actuation is complete. */
    RsTask_SendAckPacket();
    RsTask_ApplyTelecommand( lpvState, hpvState );
}

static void RsTask_ProcessRx( void )
{
    sRbData rxData;
    UInt32 i;

    while( UartComm_Read( &rxData ) >= 0 )
    {
        for( i = 0U; i < rxData.usSize; i++ )
        {
            char ch = (char)rxData.ucData[i];

            if( s_tcmdRxLen == 0U )
            {
                if( ch == '$' )
                {
                    s_tcmdRxLine[0] = ch;
                    s_tcmdRxLen = 1U;
                    s_tcmdRxSkipLen = 0U;
                }
                else if( ch == '\n' )
                {
                    if( s_tcmdRxSkipLen != 0U )
                    {
                        s_tcmdRxLineCount++;
                        s_tcmdRxNoHeaderCount++;
                        s_tcmdLastLineLen = s_tcmdRxSkipLen;
                        s_tcmdRxSkipLen = 0U;
                    }
                }
                else if( ch != '\r' )
                {
                    if( s_tcmdRxSkipLen < 0xFFFFU )
                    {
                        s_tcmdRxSkipLen++;
                    }
                }
            }
            else if( ch == '\n' )
            {
                s_tcmdRxLine[s_tcmdRxLen] = '\0';
                RsTask_ProcessTelecommandLine( s_tcmdRxLine );
                s_tcmdRxLen = 0U;
                s_tcmdRxSkipLen = 0U;
            }
            else if( ch == '\r' )
            {
                /* Wait for LF. */
            }
            else if( ch == '$' )
            {
                s_tcmdRxLine[0] = ch;
                s_tcmdRxLen = 1U;
                s_tcmdRxSkipLen = 0U;
            }
            else if( s_tcmdRxLen < (TCMD_RX_LINE_SIZE - 1U) )
            {
                s_tcmdRxLine[s_tcmdRxLen] = ch;
                s_tcmdRxLen++;
            }
            else
            {
                s_tcmdRxOverflowCount++;
                s_tcmdRxLen = 0U;
            }
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
    UInt32 notifyValue;

    /* RS485-style telemetry/telecommand on USART1 (PA21 RXD1 / PB4 TXD1, PA22 DE, PA24 /RE) */
    UartComm_Init( UARTCOMM_DEFAULT_BAUDRATE );
    UartComm_SetRxNotifyTask( xTaskGetCurrentTaskHandle(), RSTASK_NOTIFY_RX_READY );

	while(1)
	{
        RsTask_ProcessRx();
        if( xTaskNotifyWait( 0U, 0xFFFFFFFFUL, &notifyValue, portMAX_DELAY ) == pdTRUE )
        {
            if( (notifyValue & RSTASK_NOTIFY_RX_READY) != 0U )
            {
                RsTask_ProcessRx();
            }
            if( (notifyValue & RSTASK_NOTIFY_TM_EVENT) != 0U )
            {
                RsTask_SendSensorPacket();
            }
        }
	}
}

void __attribute__((used)) TC1_CH0_Handler( void )
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    UInt32 status = TC1_REGS->TC_CHANNEL[0].TC_SR;

    if( (status & TC_SR_CPCS_Msk) != 0U )
    {
        s_opuTimerTickCount++;

        if( xOpuTaskSelf != NULL )
        {
            (void)xTaskNotifyFromISR( xOpuTaskSelf, 0U, eIncrement, &xHigherPriorityTaskWoken );
        }
    }

    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

static void OpuTimer_ClearCallbacks( void )
{
    UInt8 i;

    for( i = 0U; i < OPU_TIMER_CALLBACK_MAX; i++ )
    {
        s_opuTimerCallbacks[i].used = 0U;
        s_opuTimerCallbacks[i].periodTicks = 0U;
        s_opuTimerCallbacks[i].elapsedTicks = 0U;
        s_opuTimerCallbacks[i].callback = (OpuTimerCallback)0;
        s_opuTimerCallbacks[i].context = (void *)0;
    }
}

static UInt32 OpuTimer_MsToTicks( UInt32 periodMs )
{
    UInt32 ticks;

    if( periodMs == 0U )
    {
        return 0U;
    }

    ticks = (periodMs + (OPU_TIMER_TICK_MS - 1U)) / OPU_TIMER_TICK_MS;
    if( ticks == 0U )
    {
        ticks = 1U;
    }

    return ticks;
}

UInt8 OpuTimer_RegisterCallback( UInt32 periodMs, OpuTimerCallback callback, void *context )
{
    UInt8 i;
    UInt32 periodTicks = OpuTimer_MsToTicks( periodMs );

    if( (periodTicks == 0U) || (callback == (OpuTimerCallback)0) )
    {
        return 0U;
    }

    taskENTER_CRITICAL();
    for( i = 0U; i < OPU_TIMER_CALLBACK_MAX; i++ )
    {
        if( s_opuTimerCallbacks[i].used == 0U )
        {
            s_opuTimerCallbacks[i].periodTicks = periodTicks;
            s_opuTimerCallbacks[i].elapsedTicks = 0U;
            s_opuTimerCallbacks[i].callback = callback;
            s_opuTimerCallbacks[i].context = context;
            s_opuTimerCallbacks[i].used = 1U;
            taskEXIT_CRITICAL();
            return 1U;
        }
    }
    taskEXIT_CRITICAL();

    return 0U;
}

static void OpuTimer_RegisterDefaultCallbacks( void )
{
    OpuTimer_ClearCallbacks();
    (void)OpuTimer_RegisterCallback( 10U, Opu_10msCallback, (void *)0 );
    (void)OpuTimer_RegisterCallback( 100U, Opu_100msCallback, (void *)0 );
    (void)OpuTimer_RegisterCallback( 1000U, Opu_1000msCallback, (void *)0 );
}

static void OpuTimer_ServiceCallbacks( UInt32 elapsedTicks )
{
    UInt32 tick;
    UInt8 i;

    for( tick = 0U; tick < elapsedTicks; tick++ )
    {
        for( i = 0U; i < OPU_TIMER_CALLBACK_MAX; i++ )
        {
            if( s_opuTimerCallbacks[i].used == 0U )
            {
                continue;
            }

            s_opuTimerCallbacks[i].elapsedTicks++;
            if( s_opuTimerCallbacks[i].elapsedTicks >= s_opuTimerCallbacks[i].periodTicks )
            {
                s_opuTimerCallbacks[i].elapsedTicks = 0U;
                s_opuTimerCallbacks[i].callback( s_opuTimerCallbacks[i].context );
            }
        }
    }
}

static void OpuTimer_Init( void )
{
    NVIC_DisableIRQ( TC1_CH0_IRQn );

    PMC_REGS->PMC_PCER0 = (1UL << ID_TC1_CHANNEL0);
    TC1_REGS->TC_CHANNEL[0].TC_CCR = TC_CCR_CLKDIS_Msk;
    TC1_REGS->TC_CHANNEL[0].TC_IDR = TC_IDR_Msk;
    (void)TC1_REGS->TC_CHANNEL[0].TC_SR;

    TC1_REGS->TC_CHANNEL[0].TC_CMR = TC_CMR_TCCLKS_TIMER_CLOCK4 |
                                     TC_CMR_WAVE_Msk |
                                     TC_CMR_WAVEFORM_WAVSEL_UP_RC;
    TC1_REGS->TC_CHANNEL[0].TC_RC = OPU_TIMER_RC_COUNT;

    NVIC_ClearPendingIRQ( TC1_CH0_IRQn );
    NVIC_SetPriority( TC1_CH0_IRQn, 7 );
    NVIC_EnableIRQ( TC1_CH0_IRQn );

    TC1_REGS->TC_CHANNEL[0].TC_IER = TC_IER_CPCS_Msk;
    TC1_REGS->TC_CHANNEL[0].TC_CCR = TC_CCR_CLKEN_Msk | TC_CCR_SWTRG_Msk;
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
	xTaskCreate( TcTask, "TcTask", SCDAU_STACK_SIZE, NULL, TC_TASK_PRIORITY, &xTcTask );

	/* --- RS422 Task --- */
	xTaskCreate( RsTask, "RsTask", SCDAU_STACK_SIZE, NULL, RS_TASK_PRIORITY, &xRsTask );

    /* --- RS422 Task --- */
	xTaskCreate( AdcTask, "AdcTask", SCDAU_STACK_SIZE, NULL, ADC_TASK_PRIORITY, &xAdcTask );
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
    /* Micro 밸브: PWM0 ch0~3, PWM1 ch0~1 정지 */
    PWM0_ChannelsStop( PWM_CHANNEL_0_MASK | PWM_CHANNEL_1_MASK | PWM_CHANNEL_2_MASK | PWM_CHANNEL_3_MASK );
    PWM1_ChannelsStop( PWM_CHANNEL_0_MASK | PWM_CHANNEL_1_MASK );
    PWM0_REGS->PWM_CH_NUM[0].PWM_CDTY = 0U;
    PIOA_REGS->PIO_PER  = LPV01_GPIO_PA0_MASK;
    PIOA_REGS->PIO_OER  = LPV01_GPIO_PA0_MASK;
    PIOA_REGS->PIO_CODR = LPV01_GPIO_PA0_MASK;
    PIOD_REGS->PIO_PER  = LPV01_RTN_PD12_MASK;
    PIOD_REGS->PIO_OER  = LPV01_RTN_PD12_MASK;
    PIOD_REGS->PIO_CODR = LPV01_RTN_PD12_MASK;
    /* 히터(TC3): duty 0 */
    TC3_REGS->TC_CHANNEL[0].TC_RA = 0U;
    TC3_REGS->TC_CHANNEL[0].TC_RB = 0U;
}

void OpuTask( void *pvParameters )
{
    UInt32 elapsedTicks;

    xOpuTaskSelf = xTaskGetCurrentTaskHandle();
	
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

    /* Start RS485 RX before the slow HPV driver wake/config sequence. */
    UartComm_Init( UARTCOMM_DEFAULT_BAUDRATE );

    HpSolValve_Init();

    /* Task 생성 */
	TaskCreate();
    UartComm_SetRxNotifyTask( xRsTask, RSTASK_NOTIFY_RX_READY );
    StateMachine_Init();
    OpuTimer_RegisterDefaultCallbacks();
    OpuTimer_Init();

    while(1)
    {
        elapsedTicks = ulTaskNotifyTake( pdTRUE, portMAX_DELAY );
        if( elapsedTicks != 0U )
        {
            OpuTimer_ServiceCallbacks( elapsedTicks );
        }
    }
    vTaskDelete( NULL );
}


static void Opu_10msCallback( void *context )
{
    (void)context;

    s_opu10msCallbackCount++;
    WDT_REGS->WDT_CR = WDT_CR_KEY_PASSWD | WDT_CR_WDRSTT_Msk;
    LpSolValve_Service10ms();
    HpSolValve_Service10ms();
    StateMachine_100HzEvent();
}

static void Opu_100msCallback( void *context )
{
    (void)context;

    s_opu100msCallbackCount++;
}

static void Opu_1000msCallback( void *context )
{
    (void)context;

    s_opu1000msCallbackCount++;
}
