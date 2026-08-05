/**
 * @file sam_ctl.h
 * @author Heesung Shin (shs777@danam.co.kr)
 * @brief
 * @version 1.0
 * @date 2025-12-18
 *
 * @copyright Danam Systems Copyright (c) 2025
 */

#ifndef __COMMON_H__
#define __COMMON_H__

#include "definitions.h"                // SYS function prototypes

#pragma pack(1)

/*==============================================================================
 * Common Define
 *============================================================================*/
/* --- Data Type --- */
typedef unsigned char    	UInt8;
typedef signed char    		SInt8;
typedef unsigned short   	UInt16;
typedef signed short   		SInt16;
typedef unsigned int    	UInt;
typedef signed int    		SInt;
typedef unsigned long    	UInt32;         // 64비트 OS에서 64bits
typedef signed long    		SInt32;         // 64비트 OS에서 64bits
typedef unsigned long long  UInt64;
typedef signed long long    SInt64;

typedef void (*OpuTimerCallback)( void *context );

/* --- Task Stack Size --- */
#define SCDAU_STACK_SIZE 1024
#define DYN_TEST_MAIN	0

/* --- Delay Define --- */
#define DELAY_10_SECONDS	10000UL
#define DELAY_1_SECOND		1000UL
#define DELAY_10_MSECOND	10UL
#define DELAY_5_MSECOND		5UL
#define DELAY_1_MSECOND		1UL

/*==============================================================================
 * Global Function Define
 *============================================================================*/
/* --- Main --- */
extern void DbgTask( void *pvParameters );			// DbgTask 함수 선언
extern void OpuTask( void *pvParameters );			// DbgTask 함수 선언
extern UInt8 OpuTimer_RegisterCallback( UInt32 periodMs, OpuTimerCallback callback, void *context );

/* --- Dbg_task --- */
extern UInt16 usTimerLog;
extern UInt16 usTimerLog2;

/* --- ADS1263 --- */
extern void ADS1263_Init(void);
extern void ADS1263_SetDevice( UInt8 dev );                // Select device 1 or 2.
extern float ADS1263_GetTemperature( UInt8 ucCh );
extern float ADS1263_GetTemperatureTask( UInt8 ucCh );      // Task-context read; conversion waits use vTaskDelay.
extern int32_t ADS1263_GetRawCode( UInt8 dev, UInt8 ch );  // Last raw ADC code by channel.
extern UInt8 ADS1263_GetBypass( void );                    // 1=PGA bypass gain1, 0=PGA gain32.
extern void  ADS1263_SetSpiMode( UInt8 m );                // 0=Mode0, 1=Mode1.
extern UInt8 ADS1263_GetSpiMode( void );

/* --- AFEC --- */
extern float AFEC_Init( void );

float AFEC_ToVoltage( UInt16 adc_value );
extern UInt16 ReadAFEC0Channel( AFEC_CHANNEL_NUM channel );
extern UInt16 ReadAFEC1Channel( AFEC_CHANNEL_NUM channel );
extern void AFEC0_SeqConvert( UInt32 chMask );
extern void AFEC1_SeqConvert( UInt32 chMask );

/* --- RS422 (USART1) --- */
extern void RS422_Init( UInt32 uiBaudRate );
extern void RS485_SetTransmit( UInt8 ucEnable );

/* --- 안전 --- */
extern void EnterSafeState( void );    /* 모든 액추에이터 강제 OFF (폴트/리셋/명령) */

/* --- Micro 밸브 전압제어 (Peak 28V -> Hold 2.5V) --- */
extern void MicroValve_Open( UInt8 ucCh );    /* peak 듀티 + start */
extern void MicroValve_Hold( UInt8 ucCh );    /* hold 듀티 전환 */
extern void MicroValve_Close( UInt8 ucCh );   /* 정지 */
extern void MicroValve_SetDuty( UInt8 ucCh, UInt8 ucPct );  /* 10% 듀티 직접설정(보정용) */

/* --- PWM --- */
extern void PWM_Init( void );
extern void PWM_SetPowerOn( void );
extern void PWM_SetPowerOff( void );

/* --- LP Valve (PWM 6ch) --- */
extern void LpValve_Set( UInt8 ucCh, UInt8 ucOn );

/* --- Heater (TC3 TIOA9/TIOB9) --- */
extern void Heater_Init( void );
extern void Heater_SetDuty( UInt8 ucCh, UInt8 ucPct );
extern void SparkPlug_Set( UInt8 on );

/* --- HP Valve Driver DRV3946-Q1 (SPI1) --- */
extern UInt8  g_drvPC[2];
extern UInt8  g_drvHC[2];
extern void   DRV3946_SPI_Init( void );
extern UInt16 DRV3946_Xfer16( UInt16 uiOut );
extern void   DRV3946_SetNode( UInt8 n );
extern UInt8  DRV3946_GetNode( void );
extern UInt16 DRV3946_Read24( UInt8 reg5, UInt8 node2, UInt8 *echo );
extern UInt16 DRV3946_Write24( UInt8 reg5, UInt16 data, UInt8 *echo );
extern UInt16 DRV3946_Cmd24( UInt8 hdr, UInt8 cmd, UInt8 *echo );
extern UInt16 DRV3946_Wake( UInt16 *pS0 );
extern UInt16 DRV3946_ChCtrl( UInt8 ch1, UInt8 ch2 );
/*==============================================================================
 * Global Variables Define
 *============================================================================*/
extern UInt16 usTcPrn;
extern UInt16 usRs422Loop;
extern UInt16 usAdcPrn;


/*==============================================================================
 * DBG_TASK
 *============================================================================*/
/* --- Define --- */
#define MH                          0
#define pl_ready_cmd                0x00000001
#define pl_stop_cmd                 0x00000020
#define pcm_data_ram_cmddata_0      0x20028002
#define pcm_data_ram_cmddata_1      0x0200004e
#define pcm_data_ram_cmddata_2      0x4e200280
#define pcm_data_ram_cmddata_3      0x00000000
#define SHMAXTOK                    16				// 한 라인당 최대 토큰 수
#define SHARGLEN                    170				// 인수 저장 영역 길이
#define SHBUFLEN                    168				// 일반 버퍼 길이
#define DEL                         0x7f
#define BS                          '\b'
#define BELL                        7
#define HIS_CNT                     64			// 2의 거듭제곱이어야 함
#define HIS_MSK                     (HIS_CNT-1)
#define USRCMDS                     100			// 사용자 명령 수

#define UART_DEVICE_ID              XPAR_XUARTPS_0_DEVICE_ID
#define UART_BASEADDR               XPAR_XUARTPS_0_BASEADDR

#define NULLCH                      '\0'
#define LF                          0x0a		// 줄 바꿈 문자
#define CR                          0x0d		// 리턴 문자
#define CAN                         0x18		// 취소 문자
#define DEL                         0x7f		// 삭제 문자

/* --- 구조체 --- */
typedef struct shvars {
    SInt8 *shtok[SHMAXTOK];						// 입력 토큰을 가리키는 포인터 배열
    SInt8 shargst[SHARGLEN];					// 실제 인수 문자열
} SHVARS;

typedef struct cmdent {							// 명령 테이블의 항목
    SInt32 flag;								// 반복 실행 플래그
    SInt8 *cmdnam;								// 명령 이름
    SInt32 (*cproc)(int argc, char *argv[]);	// 실행 프로시저
    SInt8 *cmdhelp;								// 도움말 명령
} CMDENT;

typedef struct usrcmd {
    SInt8 name[9];								// 명령 이름
    SInt32 (*cproc)(int argc, char *argv[]);	// 실행 프로시저
    SInt8 help[80];								// 도움말
} USRCMD;

/*==============================================================================
 * TC_TASK
 *============================================================================*/
/* --- Register addresses --- */
#define ADS1263_ID              (0x00)
#define ADS1263_POWER           (0x01)
#define ADS1263_INTERFACE       (0x02)
#define ADS1263_MODE0           (0x03)
#define ADS1263_MODE1           (0x04)
#define ADS1263_MODE2           (0x05)
#define ADS1263_INPMUX          (0x06)
#define ADS1263_OFCAL0          (0x07)
#define ADS1263_OFCAL1          (0x08)
#define ADS1263_OFCAL2          (0x09)
#define ADS1263_FSCAL0          (0x0A)
#define ADS1263_FSCAL1          (0x0B)
#define ADS1263_FSCAL2          (0x0C)
#define ADS1263_IDACMUX         (0x0D)
#define ADS1263_IDACMAG         (0x0E)
#define ADS1263_REFMUX          (0x0F)
#define ADS1263_TDACP           (0x10)
#define ADS1263_TDACN           (0x11)
#define ADS1263_GPIOCON         (0x12)
#define ADS1263_GPIODIR         (0x13)
#define ADS1263_GPIODAT         (0x14)
#define ADS1263_ADC2CFG         (0x15)
#define ADS1263_ADC2MUX         (0x16)
#define ADS1263_ADC2OFC0        (0x17)
#define ADS1263_ADC2OFC1        (0x18)
#define ADS1263_ADC2FSC0        (0x19)
#define ADS1263_ADC2FSC1        (0x1A)

/* --- Register settings --- */
#define ADS1263_POWER_SETUP     (0x01)      //Set Reset indicator to 0
#define ADS1263_POWER_DEFAULT   (0x11)      //Set Reset indicator to 1 (Default state)
#define ADS1263_INPMUX_DEFAULT  (0x01)      //Default MUX setup (MUXP - AIN0 and MUXN - AIN1)
#define ADS1263_INPMUX_SETUP    (0x23)      //MUXP - AIN2 and MUXN - AIN3
#define ADS1263_IDACMUX_SETUP   (0xB4)      //IDAC1 and AIN4
#define ADS1263_IDACMUX_DEFAULT (0xBB)      //IDAC disabled (Default state)
#define ADS1263_IDACMAG_SETUP   (0x06)      //IDAC1 and 1 mA
#define ADS1263_IDACMAG_DEFAULT (0x00)      //IDAC disabled (Default state)
#define ADS1263_MODE0_SETUP     (0x40)      //Pulse conversion (one shot)
#define ADS1263_MODE0_DEFAULT   (0x00)      //Continuous conversion (default)
#define ADS1263_MODE1_SINC1     (0x00)      //Sinc1 mode
#define ADS1263_MODE1_SINC2     (0x20)      //Sinc2 mode
#define ADS1263_MODE1_SINC3     (0x40)      //Sinc3 mode
#define ADS1263_MODE1_SINC4     (0x60)      //Sinc4 mode
#define ADS1263_MODE1_DEFAULT   (0x80)      //FIR mode (default)
#define ADS1263_MODE2_SETUP     (0x09)      //PGA bypass enabled, 1V/V, 1200 SPS (Not working with FIR)
#define ADS1263_MODE2_DEFAULT   (0x04)      //PGA bypass enabled, 1V/V, 20 SPS (default)
#define ADS1263_TDACP_SETUP     (0x80)      //Set TDACP output to pin AIN6 (2.5 V)
#define ADS1263_TDACP_DEFAULT   (0x00)      //Default state (off)
#define ADS1263_TDACN_SETUP     (0x98)      //Set TDACP output to pin AIN7 (1.5 V)
#define ADS1263_TDACN_DEFAULT   (0x00)      //Default state (off)

/* --- Commands --- */
#define ADS1263_NOP_CMD         (0x00)
#define ADS1263_RESET_CMD       (0x06)      //Or 0x07
#define ADS1263_START1_CMD      (0x08)      //Or 0x09
#define ADS1263_STOP1_CMD       (0x0A)      //Or 0x0B
#define ADS1263_START2_CMD      (0x0C)      //Or 0x0D
#define ADS1263_STOP2_CMD       (0x0E)      //Or 0x0F
#define ADS1263_RDATA1_CMD      (0x12)      //Or 0x13
#define ADS1263_RDATA2_CMD      (0x14)      //Or 0x15
#define ADS1263_SYOCAL1_CMD     (0x16)
#define ADS1263_SYGCAL1_CMD     (0x17)
#define ADS1263_SFOCAL1_CMD     (0x19)
#define ADS1263_SYOCAL2_CMD     (0x1B)
#define ADS1263_SYGCAL2_CMD     (0x1C)
#define ADS1263_SFOCAL2_CMD     (0x1E)

/* --- Const --- */
#define ADS1263_READ_ADD        (0x20)
#define ADS1263_WRITE_ADD       (0x40)

#define ADS1263_HIGH            (1)
#define ADS1263_LOW             (0)

/* --- ADS1263 --- */
#define VREF                    2.5f
#define PGA                     32.0f
#define ADC_FS                  2147483648.0f   // 2^31
#define R_FIXED                 10000.0f   // 10k

/* --- NTC --- */
#define NTC_R0                  10000.0f   // 10k  25C
#define NTC_T0                  298.15f    // 25C in Kelvin
#define NTC_BETA                3435.0f

#define TYPEK_TABLE_SIZE        1642
#define TYPEK_TABLE_MIN_TEMP   (-270.0f)
#define TYPEK_TABLE_MAX_TEMP   (1370.0f)

typedef struct InputAdc24bit {
	unsigned char ucInput[3];
}InputAdc24bit;

typedef struct InputAdc16bit {
	unsigned char ucInput[2];
}InputAdc16bit;

typedef struct ReadDataStruct {
	InputAdc16bit InputAdcCJT;
	InputAdc24bit InputAdcTC[16];
} ReadData;

typedef struct TCDataStruct {
	short TC[16];
} TCData;

typedef enum TCtypeDef {
	K,
	T
} TC_Type;

/**
 * @brief Defines Thermocouple temperature ranges
 */
typedef struct TCTempRangeDef {
    const float min;
    const float max;
    const int   numCoeffs;
    const float *coeffs;
} TC_TempRanges;

/**
 * @brief Defines Thermocouple coefficient set for table lookup
 */
typedef struct thermoTableCoefficientSetDef
{
	TC_Type 	 type;
	int 		 min;
	int 		 max;
	int 		 size;
	const float* table;
} thermoTableCoefficientSet;

/**
 * @brief Defines Thermocouple coefficient set for polynomial calculation
 */
typedef struct thermoPolyCoefficientSetDef
{
	TC_Type 	 	type;
	const int 	 	numTempRanges;
	TC_TempRanges 	*tempRanges;
} thermoPolyCoefficientSet;

typedef struct {
    float fTempCh1;
    float fTempCh2;
    float fTempCh3;
    float fTempCh4;
    float fTempCJ;
}__attribute__((packed)) sTcTemp;



 /*==============================================================================
 * ADC_TASK
 *============================================================================*/
typedef struct {
    float fPres1;       // AFEC0 CH0 (PD30) = PRES_SENSE1
    float fPres2;       // AFEC1 CH6 (PC31) = PRES_SENSE2 candidate
    float fPres3;       // AFEC0 CH2 (PB3)  = PRES_SENSE3
    float fPres4;       // AFEC0 CH3 (PE5)  = PRES_SENSE4
    float fPres5;       // AFEC0 CH5 (PB2)  = PRES_SENSE5
    float fP28vIsense;  // AFEC1 CH2 (PC15) = P28V_ISENSE
    float fP28vVsense;  // AFEC1 CH3 (PC12) = P28V_VSENSE
    float fSen5v;       // AFEC1 CH4 (PC29) = SEN_P5V_D_CB
    float fSenVdd;      // AFEC1 CH5 (PC30) = SEN_VDD_MCU_P3V3
    float fSp6;         // AFEC0 CH6 (PA17) 여유핀 - 점퍼검증용
    float fSp7;         // AFEC0 CH7 (PA18) 여유핀
    float fSp8;         // AFEC0 CH8 (PA19) 여유핀
    float fSp9;         // AFEC0 CH9 (PA20) 여유핀 - 점퍼검증용
}__attribute__((packed)) sAdcTemp;

 /*==============================================================================
 * RS422_TASK
 *============================================================================*/

/* Baud Rate */
#define BAUDRATE_2400     3906   // 150,000,000 / (16 * 2400)
#define BAUDRATE_4800     1953   // 150,000,000 / (16 * 4800)
#define BAUDRATE_9600      976   // 150,000,000 / (16 * 9600)
#define BAUDRATE_14400     651   // 150,000,000 / (16 * 14400)
#define BAUDRATE_19200     488   // 150,000,000 / (16 * 19200)
#define BAUDRATE_28800     325   // 150,000,000 / (16 * 28800)
#define BAUDRATE_38400     244   // 150,000,000 / (16 * 38400)
#define BAUDRATE_57600     162   // 150,000,000 / (16 * 57600)
#define BAUDRATE_76800     122   // 150,000,000 / (16 * 76800)
#define BAUDRATE_115200     81   // 150,000,000 / (16 * 115200)
#define BAUDRATE_153600     61   // 150,000,000 / (16 * 153600)
#define BAUDRATE_230400     40   // 150,000,000 / (16 * 230400)
#define BAUDRATE_460800     20   // 150,000,000 / (16 * 460800)
#define BAUDRATE_921600     10   // 150,000,000 / (16 * 921600)


 /*==============================================================================
 * OPU_TASK
 *============================================================================*/
#define MAX_RB_DATA 16

/* Ring buffer 정보 */
typedef struct
{
	UInt32 uiAddr;			// DDR3 시작 주소
	SInt32 siFront;			// Ring buffer Front
	SInt32 siRear;			// Ring buffer Rear
	SInt32 siCount;			// Ring buffer Count
} __attribute__((packed)) sRingBufInfo;

/* Ring buffer 저장 데이터 구조체 */
typedef struct
{
	UInt32 usSize;					// 데이터 사이즈
	UInt8 ucData[MAX_RB_DATA];		// 데이터
} __attribute__((packed)) sRbData;

#endif 			//__COMMON_H__
