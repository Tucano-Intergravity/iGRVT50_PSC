/**
 * @file dbg_task.c
 * @author Heesung Shin (shs777@danam.co.kr)
 * @brief
 * @version 1.0
 * @date 2025-12-18
 *
 * @copyright Danam Systems Copyright (c) 2025
 */


/*==============================================================================
 * Include Files
 *============================================================================*/

#include <stdio.h>			// 표준 입력/출력 라이브러리
#include <stdlib.h>			// 표준 라이브러리
#include <stddef.h>			// 포인터와 배열 관련 라이브러리
#include <string.h>			// 문자열 처리 관련 라이브러리

#include <unistd.h>			// 유닉스 시스템 관련 라이브러리
#include <time.h>			// 시간과 관련된 라이브러리
#include <sys/time.h>		// 시간 관련 라이브러리

#include "sam_ctl.h"
#include "uartcomm.h"


/*==============================================================================
 * Gloabal Function
 *============================================================================*/
void DbgTask( void *pvParameters );										// DBG Task

/*==============================================================================
 * Gloabal Variables
 *============================================================================*/
UInt16 usTcPrn;
UInt16 usRs422Loop = 0U;   /* Default OFF for telemetry/telecommand monitoring. Use 'uart 1' only for echo tests. */
UInt16 usAdcPrn;

/*==============================================================================
 * Local Variables
 *============================================================================*/
static SInt8 *his_ptrs[HIS_CNT];	// 히스토리 포인터 배열
static SInt8 *for_his;				// 포워드 히스토리 포인터
static SInt8 *promptp;				// 프롬프트 포인터

static SInt32 his_loc;				// 히스토리 위치
static SInt32 HIS_NEW;				// 새로운 히스토리 위치
static SInt32 bufcnt;				// 버퍼 카운트
static SInt32 usr_cmds;				// 사용자 명령 식별
static SInt32 cmdflag;				// 명령 플래그

static CMDENT *cmds;				// 명령 포인터
static USRCMD user_cmd[USRCMDS];	// 사용자 명령 배열


/*==============================================================================
 * Local Function
 *============================================================================*/
static void his_save(void);														// 수신 함수
static void init_cmd(void);														// 명령 초기화 함수
static void tohigh(char *line);													// 대소문자 변환 함수
static void cmdint(char c);														// 명령 초기화 함수
static void cmd_anal(int argc,char *argv[]);									// 명령 확인 함수
static void UsrCmdInit(void);													// 사용자 명령 초기화
static void puts_scc2(char c);													// char 출력 함수
static void UsrCmdList(void);													// 사용자 함수 등록 함수
static void uart_rx_check(void);												// UART 수신 함수
static void lexan(char *line);													// 수신 함수
static int lexanal(char *line);													// 수신 함수
static int NullCmd(int argc, char *argv[]);										// NULL 명령 입력 시 출력 함수
static int UsrCmdSet( const char *name, int (*cproc)(int argc, char *argv[]),
		const char *help, char flag, const char *name2);						// 사용자 명령 설정
static int help_Usrcmd(int argc, char *argv[]);									// HELP 명령 입력 시 출력 함수

/*==============================================================================
 * Functions
 *============================================================================*/

/**
 * @fn uart_rx_check
 * @brief Check and process received UART data
 * @return None
 * @date 2023-01-19
 */
static void uart_rx_check(void)
{
    if (USART0_ReceiverIsReady())
    {
        UInt8 ch = USART0_ReadByte();
        cmdint((char)ch);
    }
}


/**
 * @fn NullCmd
 * @brief Null Command Function
 * @param argc - Argument count
 * @param argv - Argument vector
 * @return Always returns 0
 * @date 2023-01-19
 */
static int NullCmd(int argc, char *argv[])
{
	/* 터미널 출력 */
	printf("%s",promptp);
	return 0;				// '0' 값 반환
}


/**
 * @fn help_Usrcmd
 * @brief Help Command Function
 * @param argc - Argument count
 * @param argv - Argument vector
 * @return Always returns 0
 * @date 2023-01-19
 */
static int help_Usrcmd(int argc, char *argv[])
{
	int	i;

	/* HELP 명령 출력: 사용 가능한 사용자 명령 목록 출력 */
	for(i=0;i<usr_cmds;i++)
	{
		/* null인 경우 */
		if(strcmp(user_cmd[i].name,"NULL"))
		{
			/* 출력 */
			printf("%02d : %s 	%s\r\n",i,user_cmd[i].name,user_cmd[i].help);
		}
	}
	return 0; // 함수 완료 후 0 반환
}


/**
 * @fn UsrCmdSet
 * @brief Set User Command
 * @param name - Command name
 * @param cproc - Command processing function
 * @param help - Command help message
 * @param flag - Flag to indicate whether to use 'name' or 'name2'
 * @param name2 - Alternative command name (used when flag is 'C')
 * @return Index of the added command, or -1 if maximum command limit reached
 * @date 2023-01-19
 */
static int UsrCmdSet(const char *name, int (*cproc)(int argc, char *argv[]), const char *help, char flag, const char *name2)
{
    int i, ii, jj;     // 변수 선언
    char *cp;          // 포인터 변수 선언

    tohigh(name);       // 문자열을 대문자로 변환

    ii = usr_cmds;      // 명령 저장
    usr_cmds++;		// 사용자 명령 증가

    // 사용자 명령 구조체에 정보 설정
    user_cmd[ii].cproc = cproc;

    // 명령 이름 설정 (최대 8자까지 지원)
    memset( user_cmd[ii].name, 0x00, 8 );
    cp = user_cmd[ii].name;
    for (jj = 0; jj < 8 && name[jj] != '\0'; jj++)
    {
        *cp++ = (char)name[jj];		// name 저장
    }

    // 명령 도움말 설정
    for (jj = 0; help[jj] != '\0'; jj++)
    {
        user_cmd[ii].help[jj] = help[jj];	// 명령 저장
    }
    user_cmd[ii].help[jj] = '\0';			// 문자열 끝

    // 추가된 명령의 인덱스를 반환
    return (usr_cmds - 1);
}


/**
 * @fn UsrCmdInit
 * @brief Initialize User Commands
 * @return None
 * @date 2023-01-19
 */
static void UsrCmdInit(void)
{
	int	i;

	/* 사용자 명령 초기화 */
	for(i=0;i<USRCMDS;i++)
	{
	    // 사용자 명령 배열의 요소 초기화
	    strcpy(user_cmd[i].name, "NULL"); // 명령 이름 초기화
	    strcpy(user_cmd[i].help, "NULL"); // 명령 도움말 초기화

	    user_cmd[i].cproc = NullCmd;      // 명령 처리 함수 초기화
	}
	usr_cmds=0;

	/* HELP 명령 등록 */
	UsrCmdSet("HE",help_Usrcmd,"Help!!",'N',"\0");
}


/**
 * @fn puts_scc2
 * @brief Print a character to the console
 * @param c - Character to print
 * @return None
 * @date 2023-01-19
 */
static void puts_scc2(char c)
{
	/* 출력 */
	printf("%c", c);		// char 출력
}


/**
 * @fn cmdint
 * @brief Process characters received from the console
 * @param c - Character to process
 * @return None
 * @date 2023-01-19
 */
static void cmdint(char c)
{
    static SInt8 shbuf[SHBUFLEN];    // 명령 입력을 저장하는 버퍼
    static UInt8 escape_char_flag;   // 이스케이프 문자 플래그
    static UInt8 escape_char_count;  // 이스케이프 문자 카운트

    // 새로운 문자를 처리
    if (c != '\r' && c != '\n' && bufcnt < SHBUFLEN - 2)
    {
        // 백스페이스 및 삭제 문자 처리
        if (c == DEL || c == BS)
        {
            if (bufcnt)
            {
                puts_scc2(BS);		// BS 처리
                puts_scc2(' ');		// blank
                puts_scc2(BS);		// BS 처리
                bufcnt--;			// 버퍼 카운트 감소
                return;				// 반환 (종료)
            }
        }

        // 이스케이프 문자 처리
        if (c != 0x1b)
        {
            if (escape_char_flag == 0)
            {
                puts_scc2(c);           // 문자를 화면에 출력
                shbuf[bufcnt++] = c;    // 입력된 문자를 버퍼에 저장
            }
        }
    }

    // Enter 키를 누를 때 명령 실행
    if (c == '\r' || c == '\n')
    {
        escape_char_flag = 0;      // 이스케이프 문자 관련 플래그 초기화
        escape_char_count = 0;     // 이스케이프 문자 카운트 초기화
        puts_scc2('\r');           // '\r'을 화면에 출력
        puts_scc2('\n');           // '\n'을 화면에 출력

        shbuf[bufcnt] = 0;         // 입력된 명령 문자열 종료
        bufcnt = 0;                // 버퍼 카운트 초기화
        lexan(shbuf);              // 입력된 명령 실행
    }
}


/**
 * @fn cmd_anal
 * @brief Analyze and execute user commands
 * @param argc - Argument count
 * @param argv - Argument vector
 * @return None
 * @date 2023-01-19
 */
static void cmd_anal(int argc, char *argv[])
{
    int i;

    // 인수 없을 경우 프롬프트 출력 후 반환
    if (argc == 0)
    {
        printf("%s", promptp);
        return;
    }

    tohigh(argv[0]); // 대문자로 변환

    // 사용자 명령 확인 및 실행
    for (i = 0; i < usr_cmds; i++)
    {
        if (!(strcmp(argv[0], user_cmd[i].name)))
        {
            his_save(); // 히스토리 저장
			user_cmd[i].cproc(argc, argv);
            printf("%s", promptp); // 프롬프트 출력
            cmdflag = 0;
            return;
        }
    }

    cmdflag = 0;
    printf("COMMAND ERROR\r\n"); // 명령 오류 메시지 출력
    printf("%s", promptp);      // 프롬프트 출력
}



/**
 * @fn lexan
 * @brief Lexical analysis and execution of commands
 * @param line - Command string to analyze and execute
 * @return None
 * @date 2023-01-19
 */
static void lexan(char *line)
{
    /*  command history */
    for_his = line; // 히스토리 저장

    lexanal(line); // 명령 해석 및 실행
}

/**
 * @fn lexanal
 * @brief Lexical analysis of a command line and execution
 * @param line - Command string to analyze
 * @return Number of tokens found
 * @date 2023-01-19
 */
static int lexanal(char *line)
{
    static SHVARS Shl;
    char **tokptr;      // 토큰 포인터 배열
    int ntok;           // 토큰 개수
    char *p;            // 문자열 포인터
    char ch;            // 현재 문자
    char *to;           // 토큰 저장 버퍼
    char quote;         // 따옴표 종류

    to = Shl.shargst;        // 토큰 저장 버퍼 초기화
    ntok = 0;
    tokptr = &Shl.shtok[ntok];    // 토큰 포인터 배열 초기화

    for (p = line; *p != '\0' && ntok < SHMAXTOK; )
    {
        while ((ch = *p++) == ' ')
        {
        	// 공백 문자 스킵
        }

        *tokptr++ = to;    // 토큰 시작 포인터 저장
        *to++ = ch;        // 토큰 버퍼에 문자 추가
        ntok++;            // 토큰 개수 증가

        while ((ch = *p) != '\0' &&
               ch != ';' && ch != '=' && ch != ' ' &&
               ch != '"' && ch != '\'' && ch != ':') // 토큰 문자 처리
        {
            *to++ = *p++;
        }
        *to++ = NULLCH; // 문자열 종료
    }

    cmd_anal(ntok, Shl.shtok); // 명령 해석 및 실행
    return (ntok);
}


/**
 * @fn his_save
 * @brief Save a command line to the command history buffer
 * @date 2023-01-19
 */
static void his_save(void)
{
    static SInt8 his_buf[HIS_CNT * 40]; // 히스토리 버퍼
    char *ptr = for_his; // 히스토리 저장 대상 문자열 포인터

    his_ptrs[HIS_NEW & HIS_MSK] = &his_buf[his_loc]; // 히스토리 포인터 설정
    HIS_NEW++; // 히스토리 인덱스 증가

    do
    {
        his_buf[his_loc++] = *ptr; // 히스토리 버퍼에 문자 저장

    } while (*ptr++);
}


/**
 * @fn init_cmd
 * @brief Initialize command-related variables
 * @date 2023-01-19
 */
static void init_cmd(void)
{
    HIS_NEW = 0;   // 히스토리 인덱스 초기화
    his_loc = 0;   // 히스토리 위치 초기화
    cmdflag = 0;   // 명령 플래그 초기화
    bufcnt = 0;    // 버퍼 카운트 초기화
}


/**
 * @fn tohigh
 * @brief Convert a string to uppercase
 * @param line - String to convert
 * @date 2023-01-19
 */
static void tohigh(char *line)
{
    /* 라인 검색 */
    while (*line)
    {
    	/* 소문자 검색 */
        if (*line >= 'a' && *line <= 'z')
        {
            *line -= ' '; 	// 소문자를 대문자로 변환
        }
        line++;				// 라인 증가
    }
}

static int htoi(char *s)
{
  	int	n = 0;
	char	ch;

	if(*s == ':') s++;
	ch=*s;
	while(ch) {
  		if((ch >= '0') && (ch <= '9'))
  			n = (n << 4) + ch - '0';
		else {
 			ch |= ' ';
			if((ch >= 'a') && (ch <= 'f'))
			{
 				n = (n << 4) + ch - 'a' + 10;
			} else {
				return(n);
			}
		}
		s++;ch=*s;
	}
	return(n);
}


static int testTcLogFunc(int argc, char *argv[])
{
	UInt16 usDbgCmd;
	if(argc<2)
	{
		printf( "RS422 Loopback=%u RX bytes=%lu drops=%lu errors=%lu\r\n",
		        (unsigned)usRs422Loop,
		        (unsigned long)UartComm_GetRxByteCount(),
		        (unsigned long)UartComm_GetRxDropCount(),
		        (unsigned long)UartComm_GetRxErrorCount() );
	}
	else
	{
		usDbgCmd = htoi(argv[1]);
		usTcPrn = usDbgCmd;
		printf( "TC LOG cmd : %d\r\n", usTcPrn );

	}

	return(0);					// '0' 리턴
}


/* [TC SPI 모드 토글] tcmode <0/1> : 0=Mode0(CPHA0, 정석/기본), 1=Mode1(CPHA1, 이전).
 * 바꾼 뒤 tcid/tcwr로 ID 0x2X 나오나 확인. 0xFF면 다른 모드도 시험. */
static int testTcModeFunc(int argc, char *argv[])
{
	if( argc >= 2 ) { ADS1263_SetSpiMode( (UInt8)htoi(argv[1]) ); }
	printf( "TC SPI mode = %u (0=Mode0/CPHA0 정석, 1=Mode1/CPHA1). 바꾼 뒤 'tcid'로 확인\r\n",
	        (unsigned)ADS1263_GetSpiMode() );
	return(0);
}

/* [디버그] ADS1263 ID 레지스터(0x00) 읽기 -> SPI 링크 HW/FW 판정
 * 정상이면 상위3비트=001 -> 0x20~0x3F. 0x00/0xFF면 통신 끊김(HW) */
extern UInt8 ADS1263_DbgReadReg( UInt8 dev, UInt8 reg );
/* [검증] TC 채널 raw 차동전압(mV): tcraw <1~6> (1~3=ADS#1 TC1~3, 4~6=ADS#2 TC1~3)
 * 쇼트접점 상온 ≈ 0mV, 접점 가열하면 상승(Type-K ≈ 41µV/°C) -> 열전대/ADC 정상 확인 */
/* [회로도 확정] TC 채널 1~6 -> {dev, AINp, AINm}
 *  U17(ADS#1): SEN1=AIN0/1, SEN2=AIN2/3, SEN3=AIN4/5, SEN4=AIN6/7  (4개)
 *  U18(ADS#2): SEN5=AIN0/1, SEN6=AIN2/3                            (2개) */
static const uint8_t g_tcCh[6][3] = {
	{1U,0U,1U}, {1U,2U,3U}, {1U,4U,5U}, {1U,6U,7U}, {2U,0U,1U}, {2U,2U,3U}
};

static int testTcRawFunc(int argc, char *argv[])
{
	extern void  TcTask_Hold( void );
	extern void  TcTask_Release( void );
	extern float ADS1263_DbgReadmV( UInt8 dev, uint8_t ainp, uint8_t ainm );
	UInt8   ch, dev;
	uint8_t p, n;
	float   mv;
	if( argc < 2 )
	{
		printf("usage: tcraw <1~6>  (1~4=ADS#1 SEN1~4, 5~6=ADS#2 SEN5~6)\r\n");
		printf("       tcraw <dev1|2> <AINp> <AINm>  (자유 페어)\r\n");
		return(0);
	}
	if( argc >= 4 )
	{
		/* 자유 페어: tcraw <dev> <ainp> <ainm> — 실배선 매핑 확정용 */
		dev = (UInt8)atoi( argv[1] );
		p   = (uint8_t)atoi( argv[2] );
		n   = (uint8_t)atoi( argv[3] );
		if( (dev != 1U && dev != 2U) || p > 10U || n > 10U )
		{ printf("dev 1~2, AIN 0~10(10=AINCOM)\r\n"); return(0); }
	}
	else
	{
	ch = (UInt8)atoi( argv[1] );
	if( ch < 1U || ch > 6U ) { printf("ch 1~6\r\n"); return(0); }
	dev = g_tcCh[ch-1U][0]; p = g_tcCh[ch-1U][1]; n = g_tcCh[ch-1U][2];  /* 회로도 매핑 */
	}
	extern float ADS1263_GetTcOffsetCh( UInt8 dev, uint8_t ainp );
	float off;
	TcTask_Hold();
	mv = ADS1263_DbgReadmV( dev, p, n );
	TcTask_Release();
	off = ADS1263_GetTcOffsetCh( dev, p );
	printf( "TC ADS#%u AIN%u-AIN%u = %.3f mV (raw %.3f, off %.3f)  (쇼트≈0; 가열시↑)\r\n",
	        (unsigned)dev, (unsigned)p, (unsigned)n, (double)(mv-off), (double)mv, (double)off );
	return(0);
}

/* TC 0점 보정: 쇼트 입력에서 현재 mV를 채널 offset으로 저장 -> 이후 측정에서 차감 */
static int testTcZeroFunc(int argc, char *argv[])
{
	extern void  TcTask_Hold( void );
	extern void  TcTask_Release( void );
	extern float ADS1263_DbgReadmV( UInt8 dev, uint8_t ainp, uint8_t ainm );
	extern void  ADS1263_SetTcOffsetCh( UInt8 dev, uint8_t ainp, float mv );
	UInt8 ch, dev; uint8_t p, n; float mv;
	if( argc < 2 ) { printf("usage: tczero <1~6>  (쇼트 상태에서 0점 캡처)\r\n"); return(0); }
	ch = (UInt8)atoi(argv[1]);
	if( ch < 1U || ch > 6U ) { printf("ch 1~6\r\n"); return(0); }
	dev = g_tcCh[ch-1U][0]; p = g_tcCh[ch-1U][1]; n = g_tcCh[ch-1U][2];  /* 회로도 매핑 */
	TcTask_Hold();
	mv = ADS1263_DbgReadmV( dev, p, n );
	ADS1263_SetTcOffsetCh( dev, p, mv );
	TcTask_Release();
	printf( "TC zero ADS#%u AIN%u-%u offset=%.3f mV 저장 (이제 쇼트=0)\r\n",
	        (unsigned)dev,(unsigned)p,(unsigned)n,(double)mv );
	return(0);
}

/* [TC 보정/진단] tcadc <vbias|bypass> <0|1> | <cal|itemp|power> [dev]
 *  - vbias 1 : floating 열전대 공통모드 2.5V 레벨시프트(POWER.bit1) ON
 *  - bypass 1: PGA 우회(gain1, rail-to-rail) — 공통모드 문제 A/B 테스트용
 *  - cal     : self offset calibration (칩 offset 보정)
 *  - itemp   : 내부 온도센서(=CJ) °C
 *  - power   : POWER 레지스터 덤프 */
/* LP밸브 RTN 저측(U9) enable: lprtn <0|1> (PD12 레벨). 전류 흐르는 쪽이 enable. */
static int testLpRtnFunc(int argc, char *argv[])
{
	extern void LpRtn_SetPin( UInt8 );
	if( argc < 2 ) { printf("usage: lprtn <0|1>  (PD12=TPD2017 RTN enable; 둘 다 시험)\r\n"); return(0); }
	LpRtn_SetPin( (UInt8)atoi(argv[1]) );
	return(0);
}

static int testTcAdcFunc(int argc, char *argv[])
{
	extern void  TcTask_Hold( void );
	extern void  TcTask_Release( void );
	extern void  ADS1263_SetVbias( UInt8 );
	extern void  ADS1263_SetBypass( UInt8 );
	extern UInt8 ADS1263_GetVbias( void );
	extern UInt8 ADS1263_GetBypass( void );
	extern float ADS1263_ReadInternalTemp( UInt8 dev );
	extern void  ADS1263_SelfOffsetCal( UInt8 dev );
	extern UInt8 ADS1263_DbgReadReg( UInt8 dev, UInt8 reg );
	UInt8 dev;

	if( argc < 2 )
	{
		printf( "usage: tcadc <vbias|bypass> <0|1> | <cal|itemp|power> [dev]\r\n" );
		printf( "  state: VBIAS=%u BYPASS=%u\r\n",
		        (unsigned)ADS1263_GetVbias(), (unsigned)ADS1263_GetBypass() );
		return(0);
	}
	tohigh( argv[1] );

	if( !strcmp(argv[1], "VBIAS") && argc >= 3 )
	{
		ADS1263_SetVbias( (UInt8)atoi(argv[2]) );
		printf( "VBIAS = %u  (재측정: tcraw/tt)\r\n", (unsigned)ADS1263_GetVbias() );
		return(0);
	}
	if( !strcmp(argv[1], "BYPASS") && argc >= 3 )
	{
		ADS1263_SetBypass( (UInt8)atoi(argv[2]) );
		printf( "PGA BYPASS = %u  (0=gain32, 1=gain1)\r\n", (unsigned)ADS1263_GetBypass() );
		return(0);
	}

	dev = (argc >= 3) ? (UInt8)atoi(argv[2]) : 1U;
	if( dev != 1U && dev != 2U ) dev = 1U;

	if( !strcmp(argv[1], "CAL") )
	{
		TcTask_Hold(); ADS1263_SelfOffsetCal( dev ); TcTask_Release();
		printf( "ADS#%u self offset cal done (재측정: tcraw)\r\n", (unsigned)dev );
		return(0);
	}
	if( !strcmp(argv[1], "ITEMP") )
	{
		float t;
		TcTask_Hold(); t = ADS1263_ReadInternalTemp( dev ); TcTask_Release();
		printf( "ADS#%u 내부온도(CJ) = %.2f degC\r\n", (unsigned)dev, (double)t );
		return(0);
	}
	if( !strcmp(argv[1], "POWER") )
	{
		UInt8 p;
		TcTask_Hold(); p = ADS1263_DbgReadReg( dev, 0x01 ); TcTask_Release();
		printf( "ADS#%u POWER=0x%02X (bit0 INTREF, bit1 VBIAS)\r\n", (unsigned)dev, (unsigned)p );
		return(0);
	}
	printf( "usage: tcadc <vbias|bypass> <0|1> | <cal|itemp|power> [dev]\r\n" );
	return(0);
}

static int testTcIdFunc(int argc, char *argv[])
{
	UInt8 id1, id2;

	/* [디버그] 연속 읽기 -> 연속 SCLK (스코프 트리거용): tcid loop [count] */
	if( argc >= 2 )
	{
		tohigh( argv[1] );
		if( !strcmp(argv[1], "LOOP") )
		{
			UInt32 i, n = 20000U;
			if( argc >= 3 ) n = (UInt32)atoi( argv[2] );
			printf( "ADS1263 read loop x%lu (scope SCLK/DOUT @DGND) ...\r\n",
			        (unsigned long)n );
			for( i = 0; i < n; i++ ) { (void)ADS1263_DbgReadReg( 1, 0x00 ); }
			printf( "loop done\r\n" );
			return(0);
		}
		/* SPI1 내부 루프백 셀프테스트: tcid lb -> MCU SPI 수신경로 검증 (칩/배선 무관) */
		if( !strcmp(argv[1], "LB") )
		{
			UInt32 mr = SPI1_REGS->SPI_MR;
			UInt8  rx;
			SPI1_REGS->SPI_CR = SPI_CR_SPIDIS_Msk;
			SPI1_REGS->SPI_MR = mr | SPI_MR_LLB_Msk;       /* 내부 MOSI->MISO */
			SPI1_REGS->SPI_CR = SPI_CR_SPIEN_Msk;
			(void)(SPI1_REGS->SPI_RDR);
			while( (SPI1_REGS->SPI_SR & SPI_SR_TDRE_Msk) == 0U ){}
			SPI1_REGS->SPI_TDR = 0x55U;
			while( (SPI1_REGS->SPI_SR & SPI_SR_RDRF_Msk) == 0U ){}
			rx = (UInt8)(SPI1_REGS->SPI_RDR & 0xFFU);
			SPI1_REGS->SPI_CR = SPI_CR_SPIDIS_Msk;
			SPI1_REGS->SPI_MR = mr;                         /* 복원 */
			SPI1_REGS->SPI_CR = SPI_CR_SPIEN_Msk;
			printf( "SPI1 loopback: tx=0x55 rx=0x%02X  (0x55=MCU RX정상, 0x00=MCU수신손상)\r\n",
			        (unsigned)rx );
			return(0);
		}
		/* HW SPI1 교차검증: tcid hw -> 비트뱅 대신 SPI1 페리페럴(NPCS1)로 dev1 ID 읽기 */
		if( !strcmp(argv[1], "HW") )
		{
			extern void TcTask_Hold( void );
			extern void TcTask_Release( void );
			extern void ADS1263_HwIdTest( UInt8 *rx4 );
			UInt8 rx[4] = {0};
			TcTask_Hold();
			ADS1263_HwIdTest( rx );
			TcTask_Release();
			printf( "tcid HW(SPI1 peripheral, NPCS1=PC25): raw %02X %02X %02X %02X -> ID=0x%02X\r\n",
			        (unsigned)rx[0],(unsigned)rx[1],(unsigned)rx[2],(unsigned)rx[3],(unsigned)rx[2] );
			if( (rx[2] >= 0x20U) && (rx[2] <= 0x3FU) )
				printf( "  ==> HW SPI로 칩 응답! (ID=0x2X) = 비트뱅이 문제였음\r\n" );
			else
				printf( "  ==> HW SPI로도 0x00/무응답 = 칩 디지털 문제 확정 (MCU SPI 완전 배제)\r\n" );
			return(0);
		}
		/* DRDY 인-펌웨어 판정: tcid rdy [dev] -> START 후 공유 DOUT/DRDY(=MISO) 변환완료 신호 관측 */
		if( !strcmp(argv[1], "RDY") )
		{
			extern void TcTask_Hold( void );
			extern void TcTask_Release( void );
			extern void ADS1263_DbgDrdyScan( UInt8 dev, int *pLows, int *pTrans );
			UInt8 dev = ( argc >= 3 ) ? (UInt8)atoi(argv[2]) : 1U;
			int lows = 0, trans = 0;
			if( dev != 2U ) { dev = 1U; }
			TcTask_Hold();
			ADS1263_DbgDrdyScan( dev, &lows, &trans );
			TcTask_Release();
			printf( "DRDY scan dev%u: LOW=%d/200 transitions=%d (DOUT/DRDY=MISO, CS low, ~400ms)\r\n",
			        (unsigned)dev, lows, trans );
			if( trans > 0 )
				printf( "  ==> DRDY 토글 감지 = 칩 변환 동작중! (data 읽기 가능)\r\n" );
			else if( lows >= 200 )
				printf( "  ==> 계속 LOW(stuck): 항상 ready인데 data=0x00 -> 디지털 이상상태\r\n" );
			else
				printf( "  ==> 계속 HIGH: 변환완료 신호 없음 (START 미반영/내부osc 미동작 = 디지털코어 정지)\r\n" );
			return(0);
		}
		/* MISO 읽기경로 자가진단 + 외부전압 주입시험: tcid miso
		 *  - 두 ADS CS deselect로 DOUT Hi-Z -> 내부 풀업/풀다운으로 PC26 읽기경로 검증.
		 *  - 이어서 no-pull 2초 관측: 외부에서 3.3V/GND 주입하면 1/0 따라오는지 확인. */
		if( !strcmp(argv[1], "MISO") )
		{
			extern void TcTask_Hold( void );
			extern void TcTask_Release( void );
			const UInt32 M = (1UL<<26);          /* PC26 = TCADC_SPIMISO (ADS DOUT) */
			UInt32 pup, pdn, i;
			TcTask_Hold();
			/* 두 ADS CS deselect -> DOUT/DRDY Hi-Z (경합 방지) */
			PIOC_REGS->PIO_PER = (1UL<<25); PIOC_REGS->PIO_OER = (1UL<<25);
			PIOC_REGS->PIO_SODR = (1UL<<25);                          /* CS1(PC25) high */
			PIOD_REGS->PIO_PER = (1UL<<28); PIOD_REGS->PIO_OER = (1UL<<28);
			PIOD_REGS->PIO_SODR = (1UL<<28);                          /* CS2(PD28) high */
			/* PC26 입력 전환 */
			PIOC_REGS->PIO_PER = M;
			PIOC_REGS->PIO_ODR = M;
			/* [A] 내부 풀업 read (라인이 Hi-Z면 1) */
			PIOC_REGS->PIO_PPDDR = M;            /* 풀다운 off */
			PIOC_REGS->PIO_PUER  = M;            /* 풀업 on  */
			SYSTICK_DelayMs( 5 );
			pup = (PIOC_REGS->PIO_PDSR >> 26) & 1U;
			/* [A] 내부 풀다운 read (라인이 Hi-Z면 0) */
			PIOC_REGS->PIO_PUDR  = M;            /* 풀업 off */
			PIOC_REGS->PIO_PPDER = M;            /* 풀다운 on */
			SYSTICK_DelayMs( 5 );
			pdn = (PIOC_REGS->PIO_PDSR >> 26) & 1U;
			printf( "MISO(PC26) 자가진단: 풀업read=%lu(기대1) 풀다운read=%lu(기대0)\r\n",
			        (unsigned long)pup, (unsigned long)pdn );
			if( (pup == 1U) && (pdn == 0U) )
				printf( "  ==> MCU 읽기경로 정상(1/0 모두 읽힘). 평소 0x00은 '데이터가 진짜 없음'.\r\n" );
			else if( (pup == 0U) && (pdn == 0U) )
				printf( "  ==> 풀업에도 0 -> 라인을 외부가 LOW로 강하게 구동중 (ADS DOUT 또는 단락)\r\n" );
			else
				printf( "  ==> 비정상 패턴(%lu/%lu) -> 핀/배선 확인\r\n",(unsigned long)pup,(unsigned long)pdn );
			/* [B] no-pull 2초 관측: 외부 3.3V->1 / GND->0 따라오는지 */
			PIOC_REGS->PIO_PUDR  = M;
			PIOC_REGS->PIO_PPDDR = M;            /* 풀 다 off */
			printf( "  [주입시험] PC26 no-pull, 2초 관측. 3.3V->1, GND->0 떠야 함 (★5V 절대금지)\r\n" );
			for( i = 0; i < 20U; i++ )
			{
				UInt32 lvl = (PIOC_REGS->PIO_PDSR >> 26) & 1U;
				printf( "   t=%2lu MISO=%lu\r\n", (unsigned long)i, (unsigned long)lvl );
				SYSTICK_DelayMs( 100 );
			}
			PIOC_REGS->PIO_PUER = M;             /* 진단 풀업 원복 */
			TcTask_Release();
			return(0);
		}
		/* SPI1 모드/속도 스윕: tcid mode <0-3> [scbr] -> 그담에 tcid로 읽기 */
		if( !strcmp(argv[1], "MODE") )
		{
			UInt8  m;
			UInt32 scbr = 150U, csr;
			if( argc < 3 ) { printf("usage: tcid mode <0-3> [scbr]\r\n"); return(0); }
			m = (UInt8)htoi( argv[2] ) & 3U;
			if( argc >= 4 ) { scbr = (UInt32)atoi( argv[3] ); }
			if( scbr < 2U )   scbr = 2U;
			if( scbr > 255U ) scbr = 255U;
			csr = SPI_CSR_BITS_8_BIT | SPI_CSR_SCBR(scbr);
			switch( m )
			{
			case 0: csr |= SPI_CSR_CPOL_IDLE_LOW  | SPI_CSR_NCPHA_VALID_LEADING_EDGE;  break;
			case 1: csr |= SPI_CSR_CPOL_IDLE_LOW  | SPI_CSR_NCPHA_VALID_TRAILING_EDGE; break;
			case 2: csr |= SPI_CSR_CPOL_IDLE_HIGH | SPI_CSR_NCPHA_VALID_LEADING_EDGE;  break;
			case 3: csr |= SPI_CSR_CPOL_IDLE_HIGH | SPI_CSR_NCPHA_VALID_TRAILING_EDGE; break;
			}
			SPI1_REGS->SPI_CSR[0] = csr;
			printf( "TC SPI mode=%u SCBR=%lu (~%lu kHz)\r\n",
			        (unsigned)m, (unsigned long)scbr, (unsigned long)(150000UL/scbr) );
			return(0);
		}
	}

	/* TcTask 잠깐 정지 -> SPI1 단독 점유(경합 제거) + 4바이트 raw 덤프 */
	{
		extern void TcTask_Hold( void );
		extern void TcTask_Release( void );
		extern void ADS1263_DbgRaw4( UInt8 dev, UInt8 reg, UInt8 *rx4 );
		UInt8 rx1[4] = {0}, rx2[4] = {0};
		TcTask_Hold();
		SYSTICK_DelayMs( 5 );
		ADS1263_DbgRaw4( 1, 0x00, rx1 );
		ADS1263_DbgRaw4( 2, 0x00, rx2 );
		TcTask_Release();
		id1 = rx1[2]; id2 = rx2[2];
		printf( "ADS#1 raw: %02X %02X %02X %02X | ADS#2 raw: %02X %02X %02X %02X\r\n",
		        rx1[0],rx1[1],rx1[2],rx1[3], rx2[0],rx2[1],rx2[2],rx2[3] );
	}
	printf( "ADS#1 REG0(ID)=0x%02X   ADS#2 REG0(ID)=0x%02X\r\n",
	        (unsigned)id1, (unsigned)id2 );
	printf( "  -> 0x20~0x3F : SPI OK(HW정상, 설정문제)\r\n" );
	printf( "  -> 0x00/0xFF : SPI 끊김(HW문제)\r\n" );
	return(0);
}

/* [디버그] ADS1263 고정값 왕복 검증: 알려진 값 4개를 OFCAL0(0x07,쓰기가능)에 쓰고 되읽음.
 * 되읽은 값 == 쓴 값 이면 SPI 양방향 + IC 정상. */
static int testTcWrFunc(int argc, char *argv[])
{
	extern void  TcTask_Hold( void );
	extern void  TcTask_Release( void );
	extern UInt8 ADS1263_DbgWrRd( UInt8 dev, UInt8 reg, UInt8 val );
	const UInt8 reg = 0x07U;                       /* OFCAL0: 임의값 보관 가능 레지스터 */
	UInt8 pats[4] = { 0xA5U, 0x5AU, 0xFFU, 0x00U };
	UInt8 i, rb1, rb2, ok1 = 1U, ok2 = 1U;
	(void)argc; (void)argv;

	TcTask_Hold();
	SYSTICK_DelayMs( 5 );
	printf( "ADS1263 WREG->RREG fixed-value test (reg 0x%02X):\r\n", (unsigned)reg );
	for( i = 0U; i < 4U; i++ )
	{
		rb1 = ADS1263_DbgWrRd( 1, reg, pats[i] );
		rb2 = ADS1263_DbgWrRd( 2, reg, pats[i] );
		if( rb1 != pats[i] ) { ok1 = 0U; }
		if( rb2 != pats[i] ) { ok2 = 0U; }
		printf( "  wrote 0x%02X -> ADS#1=0x%02X %s | ADS#2=0x%02X %s\r\n",
		        (unsigned)pats[i], (unsigned)rb1, (rb1==pats[i])?"OK":"X",
		        (unsigned)rb2, (rb2==pats[i])?"OK":"X" );
	}
	TcTask_Release();
	printf( "  => ADS#1 %s / ADS#2 %s\r\n", ok1?"PASS(IC/SPI 정상)":"FAIL", ok2?"PASS(IC/SPI 정상)":"FAIL" );
	printf( "  (전부OK=정상 / 전부00=읽기불가 / 전부FF=MISO HIGH고정 / 일부만=비트단락)\r\n" );
	return(0);
}

/* [디버그] 전원 안정 후 ADS 재-RESET+init. 전원 인가/안정 뒤 수동 실행해서
 * ID가 0x2X로 깨어나면 = 부팅시 전원타이밍 문제(펌웨어로 해결가능),
 * 0x00이면 = 느린 5V가 리셋으로 복구 안 됨(HW 수정 필요) 판정. */
static int testTcResetFunc(int argc, char *argv[])
{
	extern void  TcTask_Hold( void );
	extern void  TcTask_Release( void );
	extern void  ADS1263_Init( void );
	extern UInt8 ADS1263_DbgReadReg( UInt8 dev, UInt8 reg );
	UInt8 id1, id2;
	(void)argc; (void)argv;

	TcTask_Hold();
	SYSTICK_DelayMs( 5 );
	printf( "ADS1263 re-init (RESET+config), 전원 안정 후...\r\n" );
	ADS1263_Init();                          /* RESET 펄스 + 설정 (지금=전원 안정 상태) */
	id1 = ADS1263_DbgReadReg( 1, 0x00 );
	id2 = ADS1263_DbgReadReg( 2, 0x00 );
	TcTask_Release();
	printf( "  re-init 후: ADS#1 ID=0x%02X  ADS#2 ID=0x%02X\r\n", (unsigned)id1, (unsigned)id2 );
	printf( "  (0x2X=깨어남=전원타이밍 문제 확정/펌웨어 해결가능 / 0x00=리셋복구안됨=HW 필요)\r\n" );
	return(0);
}


/* [디버그] AFEC 채널을 최소설정(오프셋0/게인0/프리런off/TAG off)으로 단발 변환.
 * adt <0|1> <ch> -> 설정문제(=이걸로 읽힘) vs 핀자체(=여전히 0) 판별 */
static int testAdtFunc(int argc, char *argv[])
{
	afec_registers_t *r;
	UInt8  afec, ch;
	UInt32 to;
	UInt16 raw;
	if( argc < 3 ) { printf("usage: adt <0|1> <ch>\r\n"); return(0); }
	afec = (UInt8)(htoi(argv[1]) & 1U);
	ch   = (UInt8)(htoi(argv[2]) & 0xFU);
	r = (afec == 1U) ? AFEC1_REGS : AFEC0_REGS;
	r->AFEC_CR  = AFEC_CR_SWRST_Msk;
	r->AFEC_MR  = AFEC_MR_PRESCAL(7U) | AFEC_MR_STARTUP_SUT64 | AFEC_MR_TRANSFER(2U); /* 프리런 없음 */
	r->AFEC_EMR = AFEC_EMR_RES_NO_AVERAGE;                                            /* TAG 없음, DIFFR=0(단일) */
	r->AFEC_CGR = 0U;                                                                  /* 게인 X1 */
	r->AFEC_DIFFR = 0U;                                                                /* 명시적 단일ended */
	r->AFEC_CSELR = ch; r->AFEC_COCR = 0U;                                             /* 오프셋 0 */
	r->AFEC_CHER = (1UL << ch);

	/* --- 읽기 1: PGA ON --- */
	r->AFEC_ACR = AFEC_ACR_PGA0EN_Msk | AFEC_ACR_PGA1EN_Msk | AFEC_ACR_IBCTL(0x3U);
	r->AFEC_CR  = AFEC_CR_START_Msk;
	to = 1000000U; while( ((r->AFEC_ISR & (1UL << ch)) == 0U) && (--to > 0U) ){}
	r->AFEC_CSELR = ch; raw = (UInt16)(r->AFEC_CDR & 0xFFFFU);
	printf( "AFEC%u CH%u PGAon : raw=%u mV=%u%s\r\n", (unsigned)afec,(unsigned)ch,
	        (unsigned)raw,(unsigned)(((UInt32)raw*3300U)/4095U),(to==0U)?" t/o":"" );

	/* --- 읽기 2: PGA OFF --- */
	r->AFEC_ACR = AFEC_ACR_IBCTL(0x3U);
	r->AFEC_CR  = AFEC_CR_START_Msk;
	to = 1000000U; while( ((r->AFEC_ISR & (1UL << ch)) == 0U) && (--to > 0U) ){}
	r->AFEC_CSELR = ch; raw = (UInt16)(r->AFEC_CDR & 0xFFFFU);
	printf( "AFEC%u CH%u PGAoff: raw=%u mV=%u%s\r\n", (unsigned)afec,(unsigned)ch,
	        (unsigned)raw,(unsigned)(((UInt32)raw*3300U)/4095U),(to==0U)?" t/o":"" );
	return(0);
}

/* [자가진단] MCU가 핀을 GPIO출력으로 H/L 구동 후, 같은 핀을 AFEC로 읽음.
 * AFEC가 따라오면 패드~AFEC 내부경로 정상. 안 따라오면 AFEC 입력경로 끊김(전역). */
static UInt16 pself_rd( afec_registers_t *r, UInt8 ch, UInt8 pga )
{
	UInt32 to = 1000000U;
	r->AFEC_CR    = AFEC_CR_SWRST_Msk;
	/* [고임피던스 소스 대응 시험] TRACKTIM 최대(15) = 트래킹 시간 확보, 300k소스 정착용 */
	r->AFEC_MR    = AFEC_MR_PRESCAL(7U) | AFEC_MR_STARTUP_SUT64 | AFEC_MR_TRANSFER(2U) | AFEC_MR_TRACKTIM(15U);
	r->AFEC_EMR   = AFEC_EMR_RES_NO_AVERAGE;
	r->AFEC_CGR   = 0U; r->AFEC_DIFFR = 0U;
	r->AFEC_CSELR = ch; r->AFEC_COCR = 512U;
	r->AFEC_CHER  = (1UL << ch);
	/* IBCTL 최소(0) = PGA 입력 바이어스 전류 최소화 -> 고임피던스 소스 오프셋 감소 */
	r->AFEC_ACR   = pga ? (AFEC_ACR_PGA0EN_Msk|AFEC_ACR_PGA1EN_Msk|AFEC_ACR_IBCTL(0x0U))
	                    : (AFEC_ACR_IBCTL(0x0U));
	r->AFEC_CR    = AFEC_CR_START_Msk;
	while( ((r->AFEC_ISR & (1UL<<ch))==0U) && (--to>0U) ){}
	r->AFEC_CSELR = ch;
	return (UInt16)(r->AFEC_CDR & 0xFFFFU);
}
static void pself_one( const char *nm, pio_registers_t *pio, UInt8 bit, afec_registers_t *r, UInt8 ch )
{
	UInt16 hi0,lo0,hi1,lo1;
	pio->PIO_PER = (1UL<<bit); pio->PIO_OER = (1UL<<bit);     /* GPIO 출력 */
	pio->PIO_SODR = (1UL<<bit); hi0 = pself_rd(r,ch,0); hi1 = pself_rd(r,ch,1);   /* High 구동 */
	pio->PIO_CODR = (1UL<<bit); lo0 = pself_rd(r,ch,0); lo1 = pself_rd(r,ch,1);   /* Low 구동 */
	pio->PIO_ODR = (1UL<<bit);                                 /* 입력 복원 */
	printf("%s drvHI: off=%4u on=%4u | drvLO: off=%4u on=%4u\r\n",
	       nm,(unsigned)hi0,(unsigned)hi1,(unsigned)lo0,(unsigned)lo1);
}
static int testPselfFunc(int argc, char *argv[])
{
	(void)argc;(void)argv;
	printf("AFEC self-test (raw, 정상이면 drvHI~4095 drvLO~0)\r\n");
	pself_one("PB3 /AFEC0CH2", PIOB_REGS, 3U,  AFEC0_REGS, 2U);
	pself_one("PD30/AFEC0CH0", PIOD_REGS, 30U, AFEC0_REGS, 0U);
	pself_one("PE5 /AFEC0CH3", PIOE_REGS, 5U,  AFEC0_REGS, 3U);
	pself_one("PB2 /AFEC0CH5", PIOB_REGS, 2U,  AFEC0_REGS, 5U);
	pself_one("PC29/AFEC1CH4", PIOC_REGS, 29U, AFEC1_REGS, 4U);   /* SEN5V (16번핀) */
	pself_one("PC30/AFEC1CH5", PIOC_REGS, 30U, AFEC1_REGS, 5U);   /* SENVDD(15번핀) */
	pself_one("PC12/AFEC1CH3", PIOC_REGS, 12U, AFEC1_REGS, 3U);   /* P28V_V 정상기준 */
	return(0);
}
/* [역구동+자기되읽기] 핀을 GPIO출력으로 구동하고, MCU가 자기 패드를 PDSR로 되읽음.
 * PDSR이 구동을 따라가면(HIGH->1) 다이패드는 정상 구동됨. 그래도 레그가 0V면 패드↔레그 오픈. */
static void pdrv_one( const char *nm, pio_registers_t *pio, UInt8 bit )
{
	UInt32 m = (1UL<<bit); UInt8 ph, pl;
	pio->PIO_WPMR = 0x50494F00U;              /* PIO write-protect 해제(혹시 켜져있으면) */
	pio->PIO_PER = m; pio->PIO_OER = m; pio->PIO_PUDR = m; pio->PIO_PPDDR = m;
	pio->PIO_SODR = m; ph = (UInt8)((pio->PIO_PDSR >> bit) & 1U);   /* HIGH 구동 후 되읽기 */
	pio->PIO_CODR = m; pl = (UInt8)((pio->PIO_PDSR >> bit) & 1U);   /* LOW 구동 후 되읽기 */
	pio->PIO_SODR = m;                                              /* HIGH로 두고 종료(레그 측정용) */
	printf("%s: 자기되읽기 drvHI->PDSR=%u  drvLO->PDSR=%u  (1/0이면 패드정상)\r\n", nm, ph, pl);
}
static int testPdrvFunc(int argc, char *argv[])
{
	(void)argc;(void)argv;
	pdrv_one("PB3 (31)", PIOB_REGS, 3U);
	pdrv_one("PB2     ", PIOB_REGS, 2U);
	pdrv_one("PD30(34)", PIOD_REGS, 30U);
	pdrv_one("PE5 (28)", PIOE_REGS, 5U);
	pdrv_one("PE4     ", PIOE_REGS, 4U);
	pdrv_one("PC29(16)", PIOC_REGS, 29U);   /* SEN5V */
	pdrv_one("PC30(15)", PIOC_REGS, 30U);   /* SENVDD */
	printf("-> 전부 HIGH로 둠. 이제 레그 전압 측정하세요.\r\n");
	return(0);
}

/* [핀 실태 덤프] 평소 동작상태에서 PC12/15/29/30의 PIO 제어주체/입출력/핀레벨/MUX/AFEC상태를 그대로 출력.
 * !! 리셋 직후, pself/pdrv 돌리기 前에 실행할 것 (그 명령들이 핀을 출력으로 바꿔놓음) */
static void pinst_one( const char *nm, UInt8 bit, UInt8 ch )
{
	pio_registers_t *P = PIOC_REGS;
	UInt8  psr  = (UInt8)((P->PIO_PSR  >> bit) & 1U);   /* 1=PIO제어 0=peripheral */
	UInt8  osr  = (UInt8)((P->PIO_OSR  >> bit) & 1U);   /* 1=출력 0=입력 */
	UInt8  pdsr = (UInt8)((P->PIO_PDSR >> bit) & 1U);   /* 핀 디지털레벨 */
	UInt8  pur  = (UInt8)((P->PIO_PUSR >> bit) & 1U);   /* 1=풀업off */
	UInt8  ab0  = (UInt8)((P->PIO_ABCDSR[0] >> bit) & 1U);
	UInt8  ab1  = (UInt8)((P->PIO_ABCDSR[1] >> bit) & 1U);
	UInt8  mux  = (UInt8)((ab1 << 1) | ab0);            /* 0=A 1=B 2=C 3=D */
	UInt8  en   = (UInt8)((AFEC1_REGS->AFEC_CHSR >> ch) & 1U);   /* 채널 enable */
	UInt8  eoc  = (UInt8)((AFEC1_REGS->AFEC_ISR  >> ch) & 1U);   /* 변환완료 */
	UInt16 cdr;
	AFEC1_REGS->AFEC_CSELR = ch;
	cdr = (UInt16)(AFEC1_REGS->AFEC_CDR & 0xFFFFU);
	printf("%s PSR=%u(%s) OSR=%s PDSR=%u PUoff=%u MUX=%c | CHSR.en=%u EOC=%u CDR=%4u\r\n",
	       nm, psr, psr ? "PIO " : "PERI", osr ? "OUT" : "IN ", pdsr, pur, "ABCD"[mux],
	       en, eoc, (unsigned)cdr);
}
static int testPinstFunc(int argc, char *argv[])
{
	(void)argc; (void)argv;
	printf("PIO/AFEC1 실태 (정상=PSR1 OSR=IN, AFE경로정상이면 CDR가 핀전압 반영)\r\n");
	pinst_one("PC12/CH3(정상기준)", 12U, 3U);
	pinst_one("PC15/CH2         ", 15U, 2U);
	pinst_one("PC29/CH4 SEN5V   ", 29U, 4U);
	pinst_one("PC30/CH5 SENVDD  ", 30U, 5U);
	return(0);
}

/* [전채널 스캔] AFEC1 CH0~11 개별 격리변환(PGA on). 스코프로 본 2.54V(=2540mV부근)가
 * 실제로 어느 채널/AD에 걸려있는지 찾는다. AD↔PC 매핑(데이터시트):
 * AD0=PC13 AD1=PC13? AD2=PC15 AD3=PC12 AD4=PC29 AD5=PC30 AD6=PC31 AD7=PC26 AD8=PC27 */
static int testAscanFunc(int argc, char *argv[])
{
	UInt8 ch; UInt16 r;
	static const char *adpc[12] = {
		"PC13","PC13?","PC15","PC12","PC29","PC30","PC31","PC26","PC27","-","-","-" };
	(void)argc; (void)argv;
	printf("AFEC1 전채널 스캔 (PGA on). SEN_P5V=약2540mV 인 채널 찾기:\r\n");
	for( ch = 0U; ch < 12U; ch++ )
	{
		r = pself_rd( AFEC1_REGS, ch, 1U );
		printf("  CH%2u (AD%u=%-5s): raw=%4u  %4umV\r\n",
		       (unsigned)ch, (unsigned)ch, adpc[ch],
		       (unsigned)r, (unsigned)(((UInt32)r * 3300U) / 4095U));
	}
	return(0);
}

static int testAdcLogFunc(int argc, char *argv[])
{
	UInt16 usDbgCmd;
	if(argc<2)
	{
		printf("cmd err\r\n");
	}
	else
	{
		usDbgCmd = htoi(argv[1]);
		usAdcPrn = usDbgCmd;
		printf( "ADC LOG cmd : %d\r\n", usAdcPrn );

	}

	return(0);					// '0' 리턴
}

/* [28V 전류센스 보정] 2단계 (리플래시 불필요):
 *  1) 실제 0A(28V 무부하/밸브off)에서:  acal off       -> 현재 Iv를 0A offset으로 캡처
 *  2) 알려진 전류 I_A 흘리며:           acal gain <A>  -> apv = A/(Iv-off) 자동계산
 *  인자없이 'acal' = 현재 off/gain/Iv/I 표시. */
static int testAcalFunc(int argc, char *argv[])
{
	extern void OpuSetIsenseCal( float, float );
	extern void OpuGetIsenseCal( float*, float* );
	extern sAdcTemp stAdcTemp;
	float off=0.0f, apv=0.0f, iv;
	OpuGetIsenseCal( &off, &apv );
	iv = stAdcTemp.fP28vIsense;
	if( (argc>=2) && (strcmp(argv[1],"off")==0) )
	{
		OpuSetIsenseCal( iv, apv );
		printf( "I-cal: offset=%.4fV 저장 (이 상태를 0A로 봄). 다음: known전류에서 'acal gain <A>'\r\n", (double)iv );
	}
	else if( (argc>=3) && (strcmp(argv[1],"gain")==0) )
	{
		float ik  = (float)atof(argv[2]);
		float den = iv - off;
		if( (den>0.005f) || (den<-0.005f) )
		{
			apv = ik/den; OpuSetIsenseCal( off, apv );
			printf( "I-cal: gain=%.4f A/V 저장 (Iv=%.4f off=%.4f I=%.2fA)\r\n", (double)apv,(double)iv,(double)off,(double)ik );
		}
		else { printf( "err: Iv-off=%.4f 너무작음 (센스 무신호/포화 3.4V?). 핀전압부터 확인\r\n", (double)den ); }
	}
	else
	{
		printf( "I-cal: off=%.4fV gain=%.4f A/V | Iv=%.4fV -> I=%.2fA\r\n",
		        (double)off,(double)apv,(double)iv,(double)((iv-off)*apv) );
		printf( "  0A에서 'acal off' -> known I에서 'acal gain <A>'\r\n" );
	}
	return(0);
}

/* [압력 게인 보정] 알려진 전압 V를 PRES 입력(쇼트단자)에 주입하고: pcal <V>
 *  -> PRES1이 정확히 V를 읽도록 s_presGain 재계산(전 채널 공통). 인자없이=현재게인. */
static int testPcalFunc(int argc, char *argv[])
{
	extern void  OpuSetPresGain( float );
	extern float OpuGetPresGain( void );
	extern sAdcTemp stAdcTemp;
	float g = OpuGetPresGain();
	if( argc >= 2 )
	{
		float vk   = (float)atof(argv[1]);
		float disp = stAdcTemp.fPres1;          /* 현재 PRES1 표시값 = 핀전압×g */
		if( disp > 0.05f )
		{
			float ng = vk * g / disp;           /* 핀전압=disp/g, 새게인=vk/핀전압 */
			OpuSetPresGain( ng );
			printf( "P-cal: presGain %.4f -> %.4f (PRES1 %.3f -> 목표 %.3fV)\r\n", (double)g,(double)ng,(double)disp,(double)vk );
		}
		else { printf( "err: PRES1=%.3f 너무낮음. 주입전압 인가 후 다시\r\n", (double)disp ); }
	}
	else { printf( "P-cal: presGain=%.4f | 사용: 주입V 인가하고 'pcal <V>'\r\n", (double)g ); }
	return(0);
}

static int testUartLogFunc(int argc, char *argv[])
{
	UInt16 usDbgCmd;
	if(argc<2)
	{
		printf("cmd err\r\n");
	}
	else
	{
		usDbgCmd = htoi(argv[1]);
		usRs422Loop = usDbgCmd;
		printf( "RS422 Loopback Control cmd : %d\r\n", usRs422Loop );

	}

	return(0);					// '0' 리턴
}

/**
 * @fn testLpvFunc
 * @brief LP 밸브(PWM 6ch) 제어 검증 : lpv <1~6> <0/1>
 */
static int testLpvFunc(int argc, char *argv[])
{
	UInt8 ucCh;
	if( argc < 3 )
	{
		printf( "usage: lpv <1~12> <duty 0~100, 10%% step>  (1-4=PWM0,5-8=PWM1,9-12=TC0)\r\n" );
		return(0);
	}
	ucCh  = (UInt8)atoi( argv[1] );
	{
		/* [보정용] lpv <ch 1~12> <duty 0~100, 10% 단위>. 평균전압 ≈ 28V x duty%.
		 * 듀티 바꿔가며 실측 -> 전압경향 확인 -> 역보정. */
		UInt8 ucPct = (UInt8)atoi( argv[2] );
		ucPct = (UInt8)(((ucPct + 5U)/10U)*10U); if( ucPct > 100U ) ucPct = 100U;
		MicroValve_SetDuty( ucCh, ucPct );
		printf( "LP_Valve%02d duty=%u%%  (기대 평균 ≈ %u.%01uV = 28V x %u%%; 실측후 보정)\r\n",
		        (unsigned)ucCh, (unsigned)ucPct,
		        (unsigned)((28U*ucPct)/100U), (unsigned)(((28U*ucPct)%100U)/10U), (unsigned)ucPct );
	}
	return(0);					// '0' 리턴
}


/**
 * @fn testHtrFunc
 * @brief 히터 PWM duty 검증 : htr <1~4> <0~100>
 */
static int testHtrFunc(int argc, char *argv[])
{
	UInt8 ucCh, ucPct;
	if( argc < 3 )
	{
		printf( "usage: htr <1~4> <0~100>  (1=PE0,2=PE1,3=PE3,4=PE4)\r\n" );
		return(0);
	}
	ucCh  = (UInt8)htoi( argv[1] );
	ucPct = (UInt8)atoi( argv[2] );     /* duty는 10진 % */
	if( ucPct > 100U ) ucPct = 100U;    /* [변경] 1% 단위 (HW분해능 1/7500=0.013%) */
	Heater_SetDuty( ucCh, ucPct );
	printf( "  (기대 평균 ≈ %u.%01uV = 28V x %u%%; 실측후 보정)\r\n",
	        (unsigned)((28U*ucPct)/100U), (unsigned)(((28U*ucPct)%100U)/10U), (unsigned)ucPct );
	return(0);					// '0' 리턴
}

static int testSpFunc(int argc, char *argv[])
{
	UInt8 ucOn;

	if( argc < 2 )
	{
		printf( "usage: sp <0|1>  (PC5 GPIO, 1=ON)\r\n" );
		return(0);
	}

	ucOn = (UInt8)atoi( argv[1] );
	SparkPlug_Set( (ucOn != 0U) ? 1U : 0U );
	printf( "SP = %u\r\n", (unsigned)((ucOn != 0U) ? 1U : 0U) );
	return(0);
}

/* [디버그] TC3(히터) 레지스터 덤프 -> 타이머 동작/duty 설정 확인 (핀 측정 불필요) */
static int testHtrRegFunc(int argc, char *argv[])
{
	UInt32 cv1 = TC3_REGS->TC_CHANNEL[0].TC_CV;
	UInt32 cv2 = TC3_REGS->TC_CHANNEL[0].TC_CV;
	printf( "TC3 CV: %lu -> %lu  (값이 변하면 타이머 동작중)\r\n",
	        (unsigned long)cv1, (unsigned long)cv2 );
	printf( "TC3 RA(H1)=%lu RB(H2)=%lu RC=%lu\r\n",
	        (unsigned long)TC3_REGS->TC_CHANNEL[0].TC_RA,
	        (unsigned long)TC3_REGS->TC_CHANNEL[0].TC_RB,
	        (unsigned long)TC3_REGS->TC_CHANNEL[0].TC_RC );
	printf( "TC3 CMR=0x%08lX\r\n",
	        (unsigned long)TC3_REGS->TC_CHANNEL[0].TC_CMR );
	return(0);
}

/* [디버그] RS485(USART1) 단독 송신 테스트 : rs485 [count] [hexbyte]
 * PA22=DE, PA24=/RE를 TX 동안만 High로 올려 차동라인(Y/Z) 송신.
 * 기본 0x55를 2000회 연속 송신 -> 스코프로 차동 파형/보레이트 확인 */
static int testRs485Func(int argc, char *argv[])
{
	UInt32 n = 2000U;
	UInt8  b = 0x55U;
	if( argc >= 2 ) n = (UInt32)atoi( argv[1] );
	if( argc >= 3 ) b = (UInt8)htoi( argv[2] );
	printf( "RS485 TX: 0x%02X x %lu @921600 ...\r\n", (unsigned)b, (unsigned long)n );
	UartComm_SendByteRepeatBlocking( b, n );
	printf( "RS485 TX done\r\n" );
	return(0);
}


/**
 * @fn testHpvFunc
 * @brief HP 밸브 인터페이스(DRV3946) 검증
 *   hpv              : SPI read 프레임(0x8000) 송신 -> RX + nFAULT
 *   hpv <hex16>      : 지정 16bit SPI 프레임 송수신
 *   hpv en1 <0/1>    : EN1 제어 (PD23)
 *   hpv en2 <0/1>    : EN2 제어 (PD24)
 *   hpv kill <0/1>   : KILL_ALL 제어 (PD26)
 *   hpv fault        : nFAULT 상태 읽기 (PD27)
 */
/* [안전] 비상 안전상태: 모든 액추에이터 강제 OFF -> safe */
static int testSafeFunc(int argc, char *argv[])
{
	(void)argc; (void)argv;
	EnterSafeState();
	printf( "SAFE STATE: HP(EN/KILL) off, micro PWM stop, heaters 0%%\r\n" );
	return(0);
}

/* [검증] 압력 4ch (사양: PT-F1~F2, PT-O1~O2 / Metallux ME750 / 0.5~4.5V, 5VDC excitation)
 * 매핑: PT-F1=PRES1(PC31,AFEC1CH6), PT-F2=PRES2(PD30), PT-O1=PRES3(PB3), PT-O2=PRES4(PE5)  ※보드태그와 대조요 */
static int testPtFunc(int argc, char *argv[])
{
	extern sAdcTemp stAdcTemp;
	float p[4]; const char *tag[4] = { "PT-F1", "PT-F2", "PT-O1", "PT-O2" };
	int i;
	(void)argc; (void)argv;
	p[0] = stAdcTemp.fPres1; p[1] = stAdcTemp.fPres2;
	p[2] = stAdcTemp.fPres3; p[3] = stAdcTemp.fPres4;
	printf( "[Pressure] Metallux ME750  signal 0.5~4.5V (5VDC exc)\r\n" );
	for( i = 0; i < 4; i++ )
	{
		float pct = (p[i] - 0.5f) / 4.0f * 100.0f;   /* 0.5V=0%, 4.5V=100% */
		printf( "  %s : %6.3f V  (%5.1f%% FS)\r\n", tag[i], (double)p[i], (double)pct );
	}
	return(0);
}

/* [검증] 온도 6ch (사양: TT-F1~F3, TT-O1~O3 / K-Type)
 * 매핑: TT-F1~F3=ADS#1 Ch1~3, TT-O1~O3=ADS#2 Ch1~3 (+CJ 냉접점)  ※보드태그와 대조요
 * 주의: 열전대 미연결(오픈)이면 값 큰/ nan 정상. 실제 TC 연결 후 의미있음. */
static int testTtFunc(int argc, char *argv[])
{
	extern sTcTemp stTcTemp[2];
	(void)argc; (void)argv;
	printf( "[Temperature] K-Type Thermocouple x6 (deg C)\r\n" );
	printf( "  ADS#1  SEN1:%7.2f  SEN2:%7.2f  SEN3:%7.2f  SEN4:%7.2f   [CJ1:%7.2f]\r\n",
	        (double)stTcTemp[0].fTempCh1, (double)stTcTemp[0].fTempCh2,
	        (double)stTcTemp[0].fTempCh3, (double)stTcTemp[0].fTempCh4,
	        (double)stTcTemp[0].fTempCJ );
	printf( "  ADS#2  SEN5:%7.2f  SEN6:%7.2f                            [CJ2:%7.2f]\r\n",
	        (double)stTcTemp[1].fTempCh1, (double)stTcTemp[1].fTempCh2,
	        (double)stTcTemp[1].fTempCJ );
	return(0);
}

/* [추가] off : 전 출력 OFF (HP 8밸브 + LP 12 + 히터 4 + SP). 한 방에 안전정지. */
static int testOffFunc(int argc, char *argv[])
{
	extern UInt16 DRV3946_ChCtrl( UInt8 ch1, UInt8 ch2 );
	extern UInt8  g_drvNode;
	UInt8 n;
	(void)argc; (void)argv;
	/* HP: EN핀 글로벌 클리어(8밸브 즉시 off) + 전 노드 ChCtrl shutoff */
	DRV3946Q1_EN1_Clear(); DRV3946Q1_EN2_Clear();
	for( n = 0U; n < 4U; n++ ) { g_drvNode = n; (void)DRV3946_ChCtrl( 0U, 0U ); }
	/* LP 12채널 OFF (ch8은 L출력이라 MicroValve_SetDuty(0) 경유해야 정상 off) */
	for( n = 1U; n <= 12U; n++ ) { MicroValve_SetDuty( n, 0U ); }
	/* 히터 4채널 + SP OFF */
	for( n = 1U; n <= 4U; n++ ) { Heater_SetDuty( n, 0U ); }
	SparkPlug_Set( 0U );
	printf( "ALL OFF (HP 8 + LP 12 + Heater 4 + SP)\r\n" );
	return(0);
}

static int testHpvFunc(int argc, char *argv[])
{
	UInt16 usOut, usIn;
	UInt8  ucVal;
	extern UInt8 g_drvNode;            /* 활성 DRV3946 노드(0~3, 4칩 공유버스) */

	if( argc >= 2 )
	{
		tohigh( argv[1] );

		/* [간단버전] hpv <1~8> [f] : HP밸브 N번 ON. 기본=전류레귤(peak당김->저전류 유지),
		 * f=Force100%(연속 최대전류). node·ch·wake 자동. 끄기는 'off'. (복잡버전은 hpv on/node/wake 그대로) */
		if( argv[1][0] >= '0' && argv[1][0] <= '9' )
		{
			extern UInt16 DRV3946_Wake( UInt16 *pS0 );
			extern UInt16 DRV3946_ChCtrl( UInt8 ch1, UInt8 ch2 );
			extern UInt16 DRV3946_Read24( UInt8 reg5, UInt8 node2, UInt8 *echo );
			UInt8 vn = (UInt8)atoi(argv[1]), nd, ch, cc = 0x2U, e2 = 0; UInt16 s0;
			if( vn < 1U || vn > 8U ) { printf("hpv <1~8> [f]   (끄기: off)\r\n"); return(0); }
			nd = (UInt8)((vn-1U)/2U); ch = (UInt8)(((vn-1U)%2U)+1U);
			g_drvNode = nd;
			s0 = DRV3946_Read24( 0x01U, nd, &e2 );
			if( (s0 & 0x2000U) || (s0 == 0xFFFFU) ) { (void)DRV3946_Wake( &s0 ); }   /* POR/무응답이면 wake */
			if( argc >= 3 ) { tohigh(argv[2]); if( !strcmp(argv[2],"F") ) cc = 0x3U; }
			DRV3946Q1_EN1_OutputEnable(); DRV3946Q1_EN2_OutputEnable(); DRV3946Q1_KILL_ALL_OutputEnable();
			DRV3946Q1_KILL_ALL_Set();
			if( ch == 1U ) { DRV3946Q1_EN1_Set(); (void)DRV3946_ChCtrl( cc, 0U ); }
			else           { DRV3946Q1_EN2_Set(); (void)DRV3946_ChCtrl( 0U, cc ); }
			{ UInt8 e3=0; (void)DRV3946_Read24( 0x01U, nd, &e3 ); }   /* CMD1 즉시 적용 */
			printf( "HP%u ON (node%u ch%u, %s) nFAULT=%d  유지중 -> 끄기: off\r\n",
			        (unsigned)vn, (unsigned)nd, (unsigned)ch,
			        (cc==0x3U)?"Force100%/28V연속":"전류레귤 peak->hold", (int)DRV3946Q1_nFAULT_Get() );
			return(0);
		}

		/* [추가] hpv node <0-3> : 활성 칩 선택. 인자없으면 현재값 표시.
		 * hpv nad로 살아있는 노드 확인 -> hpv node N -> hpv wake/on/off/stat. */
		if( !strcmp(argv[1], "NODE") )
		{
			if( argc >= 3 ) g_drvNode = (UInt8)(htoi(argv[2]) & 0x3U);
			printf( "DRV3946 active node = %u  (5.6k=0/HP1-2, 12k=1/HP3-4, 27k=2/HP5-6, 56k=3/HP7-8)\r\n",
			        (unsigned)g_drvNode );
			return(0);
		}

		/* [추가] hpv meas [node] : MEAS5(0x0C)/MEAS6(0x0D) = 칩이 INIT2에서 강제전류로 실측한
		 * IPROPI 저항/전압 (datasheet 1888, §8.3.2-4). node1로 양품 IPROPI 실측값 확인용. */
		if( !strcmp(argv[1], "MEAS") )
		{
			extern UInt16 DRV3946_Read24( UInt8 reg5, UInt8 node2, UInt8 *echo );
			UInt8 e=0; UInt16 m5, m6;
			if( argc >= 3 ) g_drvNode = (UInt8)(htoi(argv[2]) & 0x3U);
			m5 = DRV3946_Read24( 0x0CU, g_drvNode, &e );
			m6 = DRV3946_Read24( 0x0DU, g_drvNode, &e );
			printf( "DRV MEAS[node%u]: MEAS5(CH1 IPROPI)=0x%04X  MEAS6(CH2 IPROPI)=0x%04X  echo=0x%02X\r\n",
			        (unsigned)g_drvNode, (unsigned)m5, (unsigned)m6, (unsigned)e );
			printf( "  (0xFFFF=무응답. 실측값 나오면 그 칩 IPROPI 정상 인식 = INIT2 도달 증거)\r\n" );
			return(0);
		}

		/* [추가] hpv fnad <node> : FORCE_NAD(NAD_OVERRIDE)+ASSIGNED_NAD 브로드캐스트 (datasheet §8.3.1-6).
		 * CMD2 상위바이트: bit13=NAD_OVERRIDE(0x20), bits12-11=ASSIGNED_NAD(node<<3).
		 * NAD_ERR 상태 칩에 주소 강제. set->clear->STATUS 읽기. (NAD 미스매핑 복구 시도) */
		if( !strcmp(argv[1], "FNAD") )
		{
			extern UInt16 DRV3946_Cmd24( UInt8 hdr, UInt8 cmd, UInt8 *echo );
			extern UInt16 DRV3946_Read24( UInt8 reg5, UInt8 node2, UInt8 *echo );
			UInt8 nd = ( argc >= 3 ) ? (UInt8)(htoi(argv[2]) & 0x3U) : 0U;
			UInt8 e=0; UInt16 s1;
			(void)DRV3946_Cmd24( 0x3CU, (UInt8)(0x20U | (nd << 3)), &e );   /* NAD_OVERRIDE set + ASSIGNED_NAD=nd */
			SYSTICK_DelayMs( 3 );
			(void)DRV3946_Cmd24( 0x3CU, 0x00U, &e );                        /* FORCE_NAD clear (확인) */
			SYSTICK_DelayMs( 3 );
			s1 = DRV3946_Read24( 0x02U, nd, &e );
			printf( "FORCE_NAD->node%u: STATUS1[node%u]=0x%04X DEVID=0x%X NAD=%u echo=0x%02X(NAD_ERR b6=%u)\r\n",
			        (unsigned)nd, (unsigned)nd, (unsigned)s1, (unsigned)((s1>>10)&3U),
			        (unsigned)((s1>>14)&3U), (unsigned)e, (unsigned)((e>>6)&1U) );
			printf( "  (DEVID=0x2 나오면 강제주소 성공. 단 NAD_ERR 칩만 처리됨)\r\n" );
			return(0);
		}

		/* [추가] hpv v <1~8> [i] : HP밸브 N번 개별 ON (node·ch 자동선택 + 자동 wake).
		 * 끄기는 그냥 'hpv off' (EN 글로벌이라 8밸브 전부 OFF). 1·2=node0, 3·4=1, 5·6=2, 7·8=3. */
		if( !strcmp(argv[1], "V") )
		{
			extern UInt16 DRV3946_Wake( UInt16 *pS0 );
			extern UInt16 DRV3946_ChCtrl( UInt8 ch1, UInt8 ch2 );
			extern UInt16 DRV3946_Read24( UInt8 reg5, UInt8 node2, UInt8 *echo );
			UInt8 vn, nd, ch, cc = 0x2U, e2 = 0; UInt16 s0;   /* 기본=0x2 전류레귤(peak->hold 유지) */
			if( argc < 3 ) { printf( "usage: hpv v <1~8> [f]   (기본=전류레귤/유지, f=Force100%%/28V; 끄기: hpv off)\r\n" ); return(0); }
			vn = (UInt8)atoi( argv[2] );
			if( vn < 1U || vn > 8U ) { printf( "hpv v: 1~8\r\n" ); return(0); }
			nd = (UInt8)((vn - 1U) / 2U);          /* HP1·2=node0, 3·4=1, 5·6=2, 7·8=3 */
			ch = (UInt8)(((vn - 1U) % 2U) + 1U);   /* 홀수밸브=ch1, 짝수=ch2 */
			g_drvNode = nd;
			s0 = DRV3946_Read24( 0x01U, nd, &e2 );
			if( (s0 & 0x2000U) || (s0 == 0xFFFFU) ) { (void)DRV3946_Wake( &s0 ); }   /* POR/무응답이면 wake */
			if( argc >= 4 ) { tohigh(argv[3]); if( !strcmp(argv[3],"F") ) cc = 0x3U; }   /* f = Force 100% */
			DRV3946Q1_EN1_OutputEnable(); DRV3946Q1_EN2_OutputEnable(); DRV3946Q1_KILL_ALL_OutputEnable();
			DRV3946Q1_KILL_ALL_Set();
			if( ch == 1U ) { DRV3946Q1_EN1_Set(); (void)DRV3946_ChCtrl( cc, 0U ); }
			else           { DRV3946Q1_EN2_Set(); (void)DRV3946_ChCtrl( 0U, cc ); }
			{ UInt8 e3 = 0; (void)DRV3946_Read24( 0x01U, nd, &e3 ); }   /* CMD1 즉시 적용 */
			printf( "HP_Valve%u ON (node%u ch%u, %s) nFAULT=%d  유지중, 끄기: hpv off\r\n",
			        (unsigned)vn, (unsigned)nd, (unsigned)ch,
			        (cc==0x3U)?"Force100%/28V":"전류레귤 peak->hold", (int)DRV3946Q1_nFAULT_Get() );
			return(0);
		}

		if( !strcmp(argv[1], "FAULT") )
		{
			printf( "DRV3946 nFAULT = %d\r\n", (int)DRV3946Q1_nFAULT_Get() );
			return(0);
		}
		if( !strcmp(argv[1],"EN1") || !strcmp(argv[1],"EN2") || !strcmp(argv[1],"KILL") )
		{
			if( argc < 3 ) { printf("need 0/1\r\n"); return(0); }
			ucVal = (UInt8)htoi( argv[2] );
			if( !strcmp(argv[1],"EN1") )  { if(ucVal) DRV3946Q1_EN1_Set();      else DRV3946Q1_EN1_Clear(); }
			if( !strcmp(argv[1],"EN2") )  { if(ucVal) DRV3946Q1_EN2_Set();      else DRV3946Q1_EN2_Clear(); }
			if( !strcmp(argv[1],"KILL") ) { if(ucVal) DRV3946Q1_KILL_ALL_Set(); else DRV3946Q1_KILL_ALL_Clear(); }
			printf( "DRV3946 %s = %d\r\n", argv[1], ucVal );
			return(0);
		}

		/* [사양] HP 밸브 개폐: hpv open|close <1=SV-F1 | 2=SV-O1>
		 * 구동 상세: Peak 28V ~1.0A / Hold ~0.07A (전류제어 = DRV3946 내부 레귤레이션)
		 * ※주의1: 채널 ON에는 EN핀 + CMD1 CHx_CTRL(SPI) 둘 다 필요 -> DRV SPI 복구 후 완전동작.
		 * ※주의2: KILL_ALL 폴라리티/28V 인가 전 반드시 실측 확인(여기선 Clear=정상운전 가정). */
		if( !strcmp(argv[1],"OPEN") || !strcmp(argv[1],"CLOSE") )
		{
			UInt8 ch = ( argc >= 3 ) ? (UInt8)atoi(argv[2]) : 0U;
			UInt8 on = (UInt8)(!strcmp(argv[1],"OPEN"));
			const char *sv = (ch==1U) ? "SV-F1" : (ch==2U) ? "SV-O1" : "?";
			if( ch != 1U && ch != 2U ) { printf("usage: hpv open|close <1=SV-F1|2=SV-O1>\r\n"); return(0); }
			DRV3946Q1_EN1_OutputEnable(); DRV3946Q1_EN2_OutputEnable(); DRV3946Q1_KILL_ALL_OutputEnable();
			if( on )
			{
				DRV3946Q1_KILL_ALL_Clear();                 /* 정상운전 (KILL 해제) — ※폴라리티 확인 */
				if( ch==1U ) DRV3946Q1_EN1_Set(); else DRV3946Q1_EN2_Set();
				/* TODO(SPI 복구 후): REINIT_NAD+CLR_FAULT -> CONFIG(peak~1A/hold~0.07A) -> CMD1 CHx_CTRL=enable */
			}
			else
			{
				if( ch==1U ) DRV3946Q1_EN1_Clear(); else DRV3946Q1_EN2_Clear();
			}
			printf( "HP %s(ch%u) %s : EN%u=%d, nFAULT=%d\r\n",
			        sv, (unsigned)ch, on?"OPEN":"CLOSE", (unsigned)ch, on?1:0, (int)DRV3946Q1_nFAULT_Get() );
			printf( "  ※전류제어(Peak28V~1A/Hold~0.07A)=DRV 내부. 실구동은 SPI CONFIG+CMD1 필요(현재 SPI 복구 대기)\r\n" );
			return(0);
		}
		/* [정식 wake — 데이터시트 8.3.2] hpv wake : CONFIG_A/B(+CRC) -> CLR_FAULT -> STANDBY */
		if( !strcmp(argv[1],"WAKE") )
		{
			extern UInt16 DRV3946_Wake( UInt16 *pS0 );
			extern UInt8  g_drvPC[2], g_drvHC[2];
			UInt16 s0 = 0, s1;
			if( argc >= 3 ) g_drvNode = (UInt8)(htoi(argv[2]) & 0x3U);   /* hpv wake [node] */
			s1 = DRV3946_Wake( &s0 );
			printf( "DRV3946 wake[node%u]: CONFIG_A/B written (CH1 PC=0x%02X HC=0x%02X, CH2 PC=0x%02X HC=0x%02X) + CLR_FAULT\r\n",
			        (unsigned)g_drvNode, g_drvPC[0], g_drvHC[0], g_drvPC[1], g_drvHC[1] );
			printf( "  STATUS0=0x%04X STATUS1=0x%04X DEVICE_ID=0x%X(기대0x2)\r\n",
			        (unsigned)s0, (unsigned)s1, (unsigned)((s1>>10)&0x3U) );
			printf( "  S1 CRC경고 A=%u B=%u (0=정상수락=STANDBY 진입)\r\n",
			        (unsigned)((s1>>1)&1U), (unsigned)(s1&1U) );
			{	/* CONFIG_A0 되읽기 -> 쓰기가 먹었는지 확정 (ED01=성공 / C040=리셋=실패) */
				extern UInt16 DRV3946_Read24( UInt8 reg5, UInt8 node2, UInt8 *echo );
				UInt8 e2=0; UInt16 a0;
				a0 = DRV3946_Read24( 0x10U, g_drvNode, &e2 );
				printf( "  CONFIG_A0 readback=0x%04X (ED01=쓰기성공/STANDBY, C040=리셋=INIT2갇힘) echo=0x%02X\r\n",
				        (unsigned)a0, (unsigned)e2 );
			}
			return(0);
		}
		/* hpv cur <ch1|2> <PC 0-255> <HC 0-255> : peak/hold 레지스터값 설정 (wake 전에) */
		if( !strcmp(argv[1],"CUR") )
		{
			extern UInt8 g_drvPC[2], g_drvHC[2];
			UInt8 ch; UInt32 pc, hc;
			if( argc < 5 ) { printf("usage: hpv cur <1|2> <PC 0-255> <HC 0-255>\r\n"
			                        "  I=(N+17)/272 x 20000 x 3V / R_IPROPI. [실장 R_IPROPI=20k] PC=74->1.00A, HC=1->0.198A\r\n"); return(0); }
			ch = (UInt8)atoi(argv[2]); pc = (UInt32)atoi(argv[3]); hc = (UInt32)atoi(argv[4]);
			if( (ch != 1U && ch != 2U) || pc > 255U || hc > 255U ) { printf("range err\r\n"); return(0); }
			g_drvPC[ch-1U] = (UInt8)pc; g_drvHC[ch-1U] = (UInt8)hc;
			printf( "DRV CH%u PC=%lu HC=%lu  (20k실장기준 peak=%lumA hold=%lumA; wake로 반영)\r\n",
			        (unsigned)ch, (unsigned long)pc, (unsigned long)hc,
			        (unsigned long)(((pc+17UL)*20000UL*3UL)/(272UL*20UL)),
			        (unsigned long)(((hc+17UL)*20000UL*3UL)/(272UL*20UL)) );
			return(0);
		}
		/* hpv on <1|2> / hpv off : EN핀 + CMD1 CHx_CTRL=0x2(턴온) / 0x0(셧오프) */
		if( !strcmp(argv[1],"ON") || !strcmp(argv[1],"OFF") )
		{
			extern UInt16 DRV3946_ChCtrl( UInt8 ch1, UInt8 ch2 );
			UInt8 on = (UInt8)(!strcmp(argv[1],"ON"));
			UInt8 ch = ( argc >= 3 ) ? (UInt8)atoi(argv[2]) : 0U;
			DRV3946Q1_EN1_OutputEnable(); DRV3946Q1_EN2_OutputEnable(); DRV3946Q1_KILL_ALL_OutputEnable();
			if( on )
			{
				/* 모드: 인자 'i'면 전류레귤(0x2, IPROPI필요), 기본은 Force 100%(0x3, 전압 on/off, IPROPI불필요) */
				UInt8 cc = 0x3U;   /* 0x3 = Force 100% duty (전류레귤 OFF) -> 28V full */
				if( argc >= 4 ) { tohigh(argv[3]); if( !strcmp(argv[3],"I") ) cc = 0x2U; }
				if( ch != 1U && ch != 2U ) { printf("usage: hpv on <1|2> [i=전류레귤]\r\n"); return(0); }
				/* [추가] STANDBY 이탈(POR=1: 워치독/브라운아웃/리셋) 시 자동 재wake -> 수동 wake 불필요 */
				{
					extern UInt16 DRV3946_Wake( UInt16 *pS0 );
					extern UInt16 DRV3946_Read24( UInt8 reg5, UInt8 node2, UInt8 *echo );
					UInt8  e2=0; UInt16 s0 = DRV3946_Read24( 0x01U, g_drvNode, &e2 );
					if( s0 & 0x2000U ) { (void)DRV3946_Wake( &s0 ); printf("  (POR감지 -> 자동 재wake)\r\n"); }
				}
				DRV3946Q1_KILL_ALL_Set();                        /* 출력 허용 */
				if( ch == 1U ) { DRV3946Q1_EN1_Set(); (void)DRV3946_ChCtrl( cc, 0U ); }
				else           { DRV3946Q1_EN2_Set(); (void)DRV3946_ChCtrl( 0U, cc ); }
					/* [수정] CMD1은 "다음 SPI 프레임"에서 적용됨 -> 더미 STATUS 읽기로 즉시 푸시
					 * (없으면 ON 직후 안 켜지고 다음 명령에서야 켜지던 증상) */
					{ extern UInt16 DRV3946_Read24( UInt8 reg5, UInt8 node2, UInt8 *echo );
					  UInt8 e3=0; (void)DRV3946_Read24( 0x01U, g_drvNode, &e3 ); }
				printf( "HP CH%u ON (EN%u=1 + CMD1 CHx_CTRL=0x%X %s) nFAULT=%d\r\n",
				        (unsigned)ch, (unsigned)ch, (unsigned)cc,
				        (cc==0x3U)?"Force100%=28V":"내부전류레귤", (int)DRV3946Q1_nFAULT_Get() );
				if( cc==0x3U ) printf( "  ->코일 양단(HOT-RTN) 멀티미터 DC ~28V 확인. hpv off로 0V\r\n" );
				else           printf( "  ->IPROPI 저항 실장 필수. 코일에 전류계 직렬로 peak/hold 확인\r\n" );
			}
			else
			{
				(void)DRV3946_ChCtrl( 0U, 0U );                  /* 양채널 셧오프 */
				DRV3946Q1_EN1_Clear(); DRV3946Q1_EN2_Clear();
				printf( "HP all OFF (CMD1=shutoff + EN1/2=0)\r\n" );
			}
			return(0);
		}
		/* HP 밸브 상태: hpv stat -> nFAULT/EN/KILL 핀 + DRV STATUS */
		if( !strcmp(argv[1],"STAT") )
		{
			extern UInt16 DRV3946_Read24( UInt8 reg5, UInt8 node2, UInt8 *echo );
			UInt8 e0=0, e1=0; UInt16 s0, s1;
			if( argc >= 3 ) g_drvNode = (UInt8)(htoi(argv[2]) & 0x3U);   /* hpv stat [node] */
			s0 = DRV3946_Read24( 0x01U, g_drvNode, &e0 );
			s1 = DRV3946_Read24( 0x02U, g_drvNode, &e1 );
			printf( "HP valve status [node%u]:\r\n", (unsigned)g_drvNode );
			printf( "  pins: nFAULT=%d EN1(SV-F1)=%d EN2(SV-O1)=%d KILL=%d\r\n",
			        (int)DRV3946Q1_nFAULT_Get(), (int)DRV3946Q1_EN1_Get(),
			        (int)DRV3946Q1_EN2_Get(), (int)DRV3946Q1_KILL_ALL_Get() );
			printf( "  DRV STATUS0=0x%04X STATUS1=0x%04X (DEVICE_ID=0x%X, 0x2=정상 / 0xFFFF=SPI무응답)\r\n",
			        (unsigned)s0, (unsigned)s1, (unsigned)((s1>>10)&0x3U) );
			if( s0 != 0xFFFFU )
			{
				static const char *chst[8] = { "OFF", "ON-정상", "ON->TSD차단", "ON->OCP_LS차단",
				    "ON->OCP_HS차단", "ON->PVDD_UV차단", "ON->UCLO차단", "ON->PVDD_OV/SPI_WD차단" };
				printf( "  S0: POR=%u DEV_ERR=%u WARN=%u | CH1_offdiag=%u CH1_STAT=[%s] | CH2_offdiag=%u CH2_STAT=[%s]\r\n",
				        (unsigned)((s0>>13)&1U), (unsigned)((s0>>9)&1U), (unsigned)((s0>>8)&1U),
				        (unsigned)((s0>>7)&1U), chst[(s0>>4)&7U],
				        (unsigned)((s0>>3)&1U), chst[s0&7U] );
				printf( "  S1: PVDD_UV=%u PVDD_OV=%u OT=%u ABIST_W=%u RIPROPI1_W=%u RIPROPI2_W=%u CRC_A=%u CRC_B=%u\r\n",
				        (unsigned)((s1>>7)&1U), (unsigned)((s1>>6)&1U), (unsigned)((s1>>5)&1U), (unsigned)((s1>>4)&1U),
				        (unsigned)((s1>>3)&1U), (unsigned)((s1>>2)&1U), (unsigned)((s1>>1)&1U), (unsigned)(s1&1U) );
				printf( "  ★CH1_STAT=[ON-정상]이면 출력 구동중 -> 밸브 안열리면 코일/배선. off-diag=open/short(부하)\r\n" );
			}
			return(0);
		}
		/* [진단] 연속 SCLK 버스트 (스코프용): hpv clk [n]
		 * U4(ADuM162N) pin4 VIN_B(MCU측 in) vs pin15 VOUT_B=SVHP_SCLK(필드측 out) 비교용 */
		if( !strcmp(argv[1], "CLK") )
		{
			extern UInt16 DRV3946_Read24( UInt8 reg5, UInt8 node2, UInt8 *echo );
			UInt32 i, n = ( argc >= 3 ) ? (UInt32)atoi(argv[2]) : 20000U;
			UInt8 e = 0;
			printf( "DRV SCLK burst x%lu (scope U4 pin4 VIN_B vs pin15 VOUT_B=SVHP_SCLK)...\r\n",
			        (unsigned long)n );
			for( i = 0; i < n; i++ ) { (void)DRV3946_Read24( 0x02U, 0U, &e ); }
			printf( "done\r\n" );
			return(0);
		}
		/* [진단] 연속 프레임 읽기: hpv rdn <reg> [nframes] -> 응답이 몇번째 프레임에 나오나 */
		if( !strcmp(argv[1], "RDN") )
		{
			extern void DRV3946_ReadFrames( UInt8 reg5, UInt8 node2, UInt8 *out, int nframes );
			UInt8 reg = ( argc >= 3 ) ? (UInt8)htoi(argv[2]) : 0x01U;
			int   nf  = ( argc >= 4 ) ? atoi(argv[3]) : 6;
			UInt8 b[24]; int f;
			if( nf < 1 ) nf = 1; if( nf > 8 ) nf = 8;
			DRV3946_ReadFrames( reg, 0U, b, nf );
			printf( "DRV rdn reg=0x%02X echo기대=0x%02X (프레임별 [b0 b1 b2]):\r\n",
			        (unsigned)reg, (unsigned)((reg<<1)|1) );
			for( f = 0; f < nf; f++ )
			{
				printf( "  F%d: %02X %02X %02X\r\n", f+1, b[f*3], b[f*3+1], b[f*3+2] );
			}
			printf( "  (어느 프레임 b1/b2에 실데이터=STATUS0 0x2500급 나오나)\r\n" );
			return(0);
		}
		/* [진단] 커맨드 CRC 알고리즘 분석: [hdr,cmd,c] 무차별 후 해당 레지스터 되읽기로 수락된 c 탐색.
		 * hpv ccb [hdr] [cmd] [rdreg]  기본 hdr=0x3A(CMD1) cmd=0x18(CH1_CTRL=3) rdreg=0x1D(CMD1).
		 * 표준 CRC(0x97/0xFF)와 비교 출력 -> 알고리즘 차이(최종XOR 등) 파악. */
		if( !strcmp(argv[1], "CCB") )
		{
			extern UInt16 DRV3946_CmdRaw( UInt8 b0, UInt8 b1, UInt8 b2 );
			extern UInt16 DRV3946_Read24( UInt8 reg5, UInt8 node2, UInt8 *echo );
			UInt8 e=0, hdr=0x3AU, cmd=0x18U, rd=0x1DU, std; UInt16 r=0; int c,k,found=-1;
			if( argc>=3 ) { hdr=(UInt8)strtoul(argv[2],NULL,16); }
			if( argc>=4 ) { cmd=(UInt8)strtoul(argv[3],NULL,16); }
			if( argc>=5 ) { rd =(UInt8)strtoul(argv[4],NULL,16); }
			/* 표준 CRC(poly0x97 init0xFF MSB) of [hdr,cmd] */
			{ UInt8 d2[2]={hdr,cmd}; std=0xFFU; for(int i=0;i<2;i++){ std^=d2[i]; for(k=0;k<8;k++) std=(std&0x80U)?(UInt8)((std<<1)^0x97U):(UInt8)(std<<1);} }
			for( c=0; c<256; c++ )
			{
				(void)DRV3946_CmdRaw( hdr, cmd, (UInt8)c );
				r = DRV3946_Read24( rd, 0U, &e );
				if( (UInt8)(r>>8) == cmd ){ found=c; break; }   /* 레지스터에 cmd 반영 = 수락 */
			}
			if( found>=0 ) printf( "★ [0x%02X,0x%02X] 수락 CRC=0x%02X (표준0x%02X, XOR차=0x%02X) CMD readback=0x%04X\r\n",
			                       (unsigned)hdr,(unsigned)cmd,(unsigned)found,(unsigned)std,(unsigned)(found^std),(unsigned)r );
			else           printf( "256개 모두 미수락 (hdr=0x%02X cmd=0x%02X 표준CRC=0x%02X 마지막 rd=0x%04X)\r\n",
			                       (unsigned)hdr,(unsigned)cmd,(unsigned)std,(unsigned)r );
			return(0);
		}
		/* [진단] 커맨드 CRC 무차별: CLR_FAULT 프레임 [hdr,cmd,c] c=0~255, POR(STATUS0 bit13) 끄는 값 탐색.
		 * hpv cmdb [hdr] [cmd]  기본 hdr=0x3C(CMD2) cmd=0x80(CLR_FAULT). hdr=0x38(CMD0)도 시험 권장. */
		if( !strcmp(argv[1], "CMDB") )
		{
			extern UInt16 DRV3946_CmdRaw( UInt8 b0, UInt8 b1, UInt8 b2 );
			extern UInt16 DRV3946_Read24( UInt8 reg5, UInt8 node2, UInt8 *echo );
			UInt8 e=0, hdr=0x3CU, cmd=0x80U; UInt16 s0=0; int c, found=-1;
			if( argc>=3 ) { hdr=(UInt8)strtoul(argv[2],NULL,16); }
			if( argc>=4 ) { cmd=(UInt8)strtoul(argv[3],NULL,16); }
			for( c=0; c<256; c++ )
			{
				(void)DRV3946_CmdRaw( hdr, cmd, (UInt8)c );
				s0 = DRV3946_Read24( 0x01U, 0U, &e );
				if( (s0 & 0x2000U)==0U ){ found=c; break; }   /* POR cleared */
			}
			if( found>=0 ) printf( "★ CLR_FAULT CRC = 0x%02X 에서 POR 해제! STATUS0=0x%04X (hdr=0x%02X cmd=0x%02X)\r\n",
			                       (unsigned)found, (unsigned)s0, (unsigned)hdr, (unsigned)cmd );
			else           printf( "256개 모두 POR 안 꺼짐 (hdr=0x%02X cmd=0x%02X, 마지막 STATUS0=0x%04X)\r\n",
			                       (unsigned)hdr, (unsigned)cmd, (unsigned)s0 );
			return(0);
		}
		/* [진단] CONFIG_B CRC 무차별 대입: B4(0x1B)에 0~0xFF, CONFIG_B_CRC_W(bit0)=0 되는 값 탐색. */
		if( !strcmp(argv[1], "CRBB") )
		{
			extern UInt16 DRV3946_Write24( UInt8 reg5, UInt16 data, UInt8 *echo );
			extern UInt16 DRV3946_Read24( UInt8 reg5, UInt8 node2, UInt8 *echo );
			UInt16 B[4] = { 0x2623U, 0x0040U, 0x0B0BU, 0xA200U };
			UInt8 e=0; UInt16 s1=0; int i,c,t,found=-1;
			for( i=0;i<4;i++ ){ for(t=0;t<3;t++){ (void)DRV3946_Write24((UInt8)(0x17U+i),B[i],&e); if(DRV3946_Read24((UInt8)(0x17U+i),0U,&e)==B[i])break; } }
			printf( "실제 B0-B3: %04X %04X %04X %04X\r\n",
			        (unsigned)DRV3946_Read24(0x17U,0U,&e),(unsigned)DRV3946_Read24(0x18U,0U,&e),
			        (unsigned)DRV3946_Read24(0x19U,0U,&e),(unsigned)DRV3946_Read24(0x1AU,0U,&e) );
			for( c=0; c<256; c++ )
			{
				for( t=0;t<3;t++ ){ (void)DRV3946_Write24( 0x1BU, (UInt16)c, &e ); }
				s1 = DRV3946_Read24( 0x02U, 0U, &e );
				if( (s1&1U)==0U ){ found=c; break; }
			}
			if( found>=0 ) printf( "★ CONFIG_B CRC = 0x%02X 에서 해제! STATUS1=0x%04X\r\n", (unsigned)found, (unsigned)s1 );
			else           printf( "256개 모두 실패 (마지막 STATUS1=0x%04X)\r\n", (unsigned)s1 );
			return(0);
		}
		/* [진단] CONFIG_B CRC 변형 시험 (실제 readback 기준). B0-B3=0x17~0x1A, B4(0x1B)=CRC.
		 * CONFIG_B_CRC_W=STATUS1 bit0. 9바이트(B0-B3 + B4hi) hi-first가 정답일 것. */
		if( !strcmp(argv[1], "CRTB") )
		{
			extern UInt16 DRV3946_Write24( UInt8 reg5, UInt16 data, UInt8 *echo );
			extern UInt16 DRV3946_Read24( UInt8 reg5, UInt8 node2, UInt8 *echo );
			UInt16 B[4] = { 0x2623U, 0x0040U, 0x0B0BU, 0xA200U };
			UInt16 act[4]; UInt8 e=0, buf[20], crc; UInt16 s1; int i,k,v,n,t;
			for( i=0;i<4;i++ ){ for(t=0;t<3;t++){ (void)DRV3946_Write24((UInt8)(0x17U+i),B[i],&e); if(DRV3946_Read24((UInt8)(0x17U+i),0U,&e)==B[i])break; } }
			for( i=0;i<4;i++ ){ act[i]=DRV3946_Read24((UInt8)(0x17U+i),0U,&e); }
			printf( "실제 B0-B3: %04X %04X %04X %04X\r\n",
			        (unsigned)act[0],(unsigned)act[1],(unsigned)act[2],(unsigned)act[3] );
			for( v=0; v<4; v++ )
			{
				n=0;
				if( v==0 ) { for(i=0;i<4;i++){ buf[n++]=(UInt8)(act[i]>>8); buf[n++]=(UInt8)act[i]; } buf[n++]=0x00; }       /* hi 9 */
				else if( v==1 ) { for(i=0;i<4;i++){ buf[n++]=(UInt8)act[i]; buf[n++]=(UInt8)(act[i]>>8); } buf[n++]=0x00; } /* lo 9 */
				else if( v==2 ) { for(i=0;i<4;i++){ buf[n++]=(UInt8)(act[i]>>8); buf[n++]=(UInt8)act[i]; } }                /* hi 8 */
				else { for(i=0;i<4;i++){ buf[n++]=(UInt8)(act[i]>>8); buf[n++]=(UInt8)act[i]; } buf[n++]=0x00; buf[n++]=0x00; } /* hi 10 */
				crc=0xFFU;
				for(i=0;i<n;i++){ crc^=buf[i]; for(k=0;k<8;k++){ crc = (crc&0x80U)?(UInt8)((crc<<1)^0x97U):(UInt8)(crc<<1);} }
				for( t=0;t<3;t++ ){ (void)DRV3946_Write24( 0x1BU, (UInt16)crc, &e ); }   /* B4=CRC */
				s1 = DRV3946_Read24( 0x02U, 0U, &e );
				printf( "  v%d(n=%d) crc=0x%02X -> STATUS1=0x%04X CONFIG_B_CRC_W=%u%s\r\n",
				        v, n, (unsigned)crc, (unsigned)s1, (unsigned)(s1&1U),
				        ((s1&1U)==0U)?"  <<<< 통과!":"" );
			}
			return(0);
		}
		/* [진단] CONFIG_A CRC 무차별 대입: A6에 0x00~0xFF 다 넣어 CONFIG_A_CRC_W=0 만드는 값 탐색.
		 * 실제 디바이스 내용에 맞는 CRC를 확정 + A6 쓰기 가능 여부 동시 확인. */
		if( !strcmp(argv[1], "CRCB") )
		{
			extern UInt16 DRV3946_Write24( UInt8 reg5, UInt16 data, UInt8 *echo );
			extern UInt16 DRV3946_Read24( UInt8 reg5, UInt8 node2, UInt8 *echo );
			UInt16 A[6] = { 0xED01U, 0xED01U, 0x2424U, 0x0088U, 0x130CU, 0x8000U };
			UInt8 e=0; UInt16 s1=0; int i,c,t,found=-1;
			for( i=0;i<6;i++ ){ for(t=0;t<3;t++){ (void)DRV3946_Write24((UInt8)(0x10U+i),A[i],&e); if(DRV3946_Read24((UInt8)(0x10U+i),0U,&e)==A[i])break; } }
			printf( "실제 A0-A5: %04X %04X %04X %04X %04X %04X\r\n",
			        (unsigned)DRV3946_Read24(0x10U,0U,&e),(unsigned)DRV3946_Read24(0x11U,0U,&e),
			        (unsigned)DRV3946_Read24(0x12U,0U,&e),(unsigned)DRV3946_Read24(0x13U,0U,&e),
			        (unsigned)DRV3946_Read24(0x14U,0U,&e),(unsigned)DRV3946_Read24(0x15U,0U,&e) );
			for( c=0; c<256; c++ )
			{
				for( t=0;t<3;t++ ){ (void)DRV3946_Write24( 0x16U, (UInt16)c, &e ); }   /* A6=c (위치실패 대비 3회) */
				s1 = DRV3946_Read24( 0x02U, 0U, &e );
				if( ((s1>>1)&1U)==0U ){ found=c; break; }
			}
			if( found>=0 ) printf( "★ CONFIG_A CRC = 0x%02X 에서 CONFIG_A_CRC_W 해제! STATUS1=0x%04X\r\n", (unsigned)found, (unsigned)s1 );
			else           printf( "256개 모두 실패 -> A6 미기록 또는 데이터 불안정 (마지막 STATUS1=0x%04X)\r\n", (unsigned)s1 );
			return(0);
		}
		/* [진단] CONFIG_A CRC 실측: A0-A5 연속쓰기 landing 확인 + CRC 변형 4종 각각 시험.
		 * 어떤 CRC 계산이 CONFIG_A_CRC_W(STATUS1 bit1)를 0으로 만드는지 찾는다. */
		if( !strcmp(argv[1], "CRCT") )
		{
			extern UInt16 DRV3946_Write24( UInt8 reg5, UInt16 data, UInt8 *echo );
			extern UInt16 DRV3946_Read24( UInt8 reg5, UInt8 node2, UInt8 *echo );
			UInt16 A[6] = { 0xED01U, 0xED01U, 0x2424U, 0x0088U, 0x130CU, 0x8000U };
			UInt16 act[6]; UInt8 e=0, buf[20], crc; UInt16 s1; int i,k,v,n,t;
			for( i=0;i<6;i++ ){ for(t=0;t<3;t++){ (void)DRV3946_Write24((UInt8)(0x10U+i),A[i],&e); if(DRV3946_Read24((UInt8)(0x10U+i),0U,&e)==A[i])break; } }
			for( i=0;i<6;i++ ){ act[i]=DRV3946_Read24((UInt8)(0x10U+i),0U,&e); }
			printf( "실제 A0-A5: %04X %04X %04X %04X %04X %04X\r\n",
			        (unsigned)act[0],(unsigned)act[1],(unsigned)act[2],(unsigned)act[3],(unsigned)act[4],(unsigned)act[5] );
			printf( "[실제값 기준 CRC 4변형 — CONFIG_A_CRC_W=0 되는 것이 정답]\r\n" );
			for( v=0; v<4; v++ )
			{
				n=0;
				if( v==0 ) { for(i=0;i<6;i++){ buf[n++]=(UInt8)(act[i]>>8); buf[n++]=(UInt8)act[i]; } buf[n++]=0x00; }       /* hi-first 13 */
				else if( v==1 ) { for(i=0;i<6;i++){ buf[n++]=(UInt8)act[i]; buf[n++]=(UInt8)(act[i]>>8); } buf[n++]=0x00; } /* lo-first 13 */
				else if( v==2 ) { for(i=0;i<6;i++){ buf[n++]=(UInt8)(act[i]>>8); buf[n++]=(UInt8)act[i]; } }                /* hi-first 12 */
				else { for(i=0;i<6;i++){ buf[n++]=(UInt8)(act[i]>>8); buf[n++]=(UInt8)act[i]; } buf[n++]=0x00; buf[n++]=0x00; } /* hi 14 */
				crc=0xFFU;
				for(i=0;i<n;i++){ crc^=buf[i]; for(k=0;k<8;k++){ crc = (crc&0x80U)?(UInt8)((crc<<1)^0x97U):(UInt8)(crc<<1);} }
				for( t=0;t<3;t++ ){ (void)DRV3946_Write24( 0x16U, (UInt16)crc, &e ); }  /* A6=CRC, 위치실패 대비 3회 */
				s1 = DRV3946_Read24( 0x02U, 0U, &e );
				printf( "  v%d(n=%d) crc=0x%02X -> STATUS1=0x%04X CONFIG_A_CRC_W=%u%s\r\n",
				        v, n, (unsigned)crc, (unsigned)s1, (unsigned)((s1>>1)&1U),
				        (((s1>>1)&1U)==0U)?"  <<<< 통과!":"" );
			}
			return(0);
		}
		/* [진단] 단독 쓰기 검증: hpv cfgv [reg16] [val16] -> 유휴 후 첫 쓰기+read-back.
		 * 기본 reg=0x10(A0) val=0xED01. 예: hpv cfgv 11 1234 (A1 단독 시험) */
		if( !strcmp(argv[1], "CFGV") )
		{
			extern UInt16 DRV3946_Write24( UInt8 reg5, UInt16 data, UInt8 *echo );
			extern UInt16 DRV3946_Read24( UInt8 reg5, UInt8 node2, UInt8 *echo );
			UInt8 e=0, reg=0x10U; UInt16 pr, rd, val=0xED01U;
			if( argc>=3 ) { reg = (UInt8)strtoul(argv[2],NULL,16); }
			if( argc>=4 ) { val = (UInt16)strtoul(argv[3],NULL,16); }
			pr = DRV3946_Write24( reg, val, &e );          /* 단독 쓰기, 반환=이전값 */
			rd = DRV3946_Read24( reg, 0U, &e );            /* 즉시 read-back */
			printf( "CFGV reg0x%02X <- 0x%04X : prior=0x%04X readback=0x%04X %s\r\n",
			        (unsigned)reg, (unsigned)val, (unsigned)pr, (unsigned)rd, (rd==val)?"OK":"FAIL" );
			return(0);
		}
		/* [진단] CLR_FAULT 단독: hpv clrf -> CLR_FAULT 후 STATUS0 읽어 POR 클리어 확인 */
		if( !strcmp(argv[1], "CLRF") )
		{
			extern UInt16 DRV3946_Cmd24( UInt8 hdr, UInt8 cmd, UInt8 *echo );
			extern UInt16 DRV3946_Read24( UInt8 reg5, UInt8 node2, UInt8 *echo );
			UInt8 e=0; UInt16 s0a, s0b;
			s0a = DRV3946_Read24( 0x01U, 0U, &e );             /* 전 */
			(void)DRV3946_Cmd24( 0x3CU, 0x80U, &e );           /* CLR_FAULT (CMD2 b15) */
			SYSTICK_DelayMs( 5 );
			s0b = DRV3946_Read24( 0x01U, 0U, &e );             /* 후 */
			printf( "CLR_FAULT: STATUS0 전=0x%04X 후=0x%04X  POR(bit13) %u->%u (0이면 CLR_FAULT 먹음)\r\n",
			        (unsigned)s0a, (unsigned)s0b, (unsigned)((s0a>>13)&1U), (unsigned)((s0b>>13)&1U) );
			return(0);
		}
		/* [진단] SPI0 MR 출력 (LLB 비트7 확인) */
		if( !strcmp(argv[1], "MR") )
		{
			UInt32 mr = SPI0_REGS->SPI_MR;
			printf( "SPI0 MR=0x%08lX  LLB(bit7)=%lu (1=내부루프백=읽기깨짐)\r\n",
			        (unsigned long)mr, (unsigned long)((mr>>7)&1U) );
			return(0);
		}
		/* [진단] NAD 노드 스캔: hpv nad -> 노드0~3로 STATUS1(0x02) 읽어 응답 노드 탐색.
		 * 실데이터(DEVICE_ID=0x2 in bits[11:10], NAD in [15:14])가 나오는 노드 = 칩의 실제 주소.
		 * data==(echo<<8)면 루프백(무응답). */
		if( !strcmp(argv[1], "NAD") )
		{
			extern UInt16 DRV3946_Read24( UInt8 reg5, UInt8 node2, UInt8 *echo );
			UInt8 nd, e; UInt16 d;
			printf( "NAD scan (STATUS1 reg0x02, 각 노드):\r\n" );
			for( nd = 0; nd < 4; nd++ )
			{
				(void)DRV3946_Read24( 0x02U, nd, &e );      /* 파이프라인 대비 1회 버림 */
				d = DRV3946_Read24( 0x02U, nd, &e );
				printf( "  node%u: data=0x%04X echo=0x%02X  NAD=%u DEVID=0x%X %s\r\n",
				        (unsigned)nd, (unsigned)d, (unsigned)e,
				        (unsigned)((d>>14)&3U), (unsigned)((d>>10)&3U),
				        (((d>>8)==(((0x02<<1)|1)|(nd<<6)))?"(루프백=무응답)":"<-- 응답?") );
			}
			printf( "  DEVID=0x2 + 루프백아님 인 노드 = 칩 실제주소\r\n" );
			return(0);
		}
		/* [진단] 루프백 판별: hpv pat -> [hdr,AA,55,CC,33,00,00,00] 보내고 RX 덤프.
		 * RX에 AA 55 CC 33이 그대로 보이면 = MISO가 우리 MOSI 되읽음(SDO 단선/루프백).
		 * 안 보이면 = DRV가 SDO 구동(데이터 or Hi-Z). */
		if( !strcmp(argv[1], "PAT") )
		{
			extern void DRV3946_XferRaw( const UInt8 *tx, UInt8 *rx, int n );
			UInt8 tx[8] = { 0x21U, 0xAAU, 0x55U, 0xCCU, 0x33U, 0x00U, 0x00U, 0x00U };
			UInt8 rx[8]; int i;
			DRV3946_XferRaw( tx, rx, 8 );
			printf( "DRV pat TX: 21 AA 55 CC 33 00 00 00\r\n" );
			printf( "DRV pat RX:" );
			for( i = 0; i < 8; i++ ) { printf( " %02X", rx[i] ); }
			printf( "\r\n  (RX에 AA 55 CC 33 보이면=MOSI루프백/SDO단선, 안보이면=DRV가 SDO구동)\r\n" );
			return(0);
		}
		/* [진단] raw 스트림 덤프: hpv raw <reg> [n] -> CS유지 n바이트 연속 (정렬 확정용) */
		if( !strcmp(argv[1], "RAW") )
		{
			extern void DRV3946_ReadRaw( UInt8 reg5, UInt8 *out, int n );
			UInt8 reg = ( argc >= 3 ) ? (UInt8)htoi(argv[2]) : 0x10U;
			int   nn  = ( argc >= 4 ) ? atoi(argv[3]) : 8;
			UInt8 b[12]; int i;
			if( nn < 3 ) nn = 3; if( nn > 12 ) nn = 12;
			DRV3946_ReadRaw( reg, b, nn );
			printf( "DRV raw reg=0x%02X (echo기대=0x%02X):", (unsigned)reg, (unsigned)((reg<<1)|1) );
			for( i = 0; i < nn; i++ ) { printf( " %02X", b[i] ); }
			printf( "\r\n  (reg값이 어느 바이트에 박히나 확인: reset A0=0xC040, 우리가쓴=0xED01)\r\n" );
			return(0);
		}
		/* [진단] 파이프라인 읽기 정렬 확인: hpv rdx [reg] -> 2프레임 6바이트 덤프 */
		if( !strcmp(argv[1], "RDX") )
		{
			extern void DRV3946_ReadDump( UInt8 reg, UInt8 *out6 );
			UInt8 reg = ( argc >= 3 ) ? (UInt8)htoi(argv[2]) : 0x02U;
			UInt8 b[6] = {0};
			DRV3946_ReadDump( reg, b );
			printf( "DRV rdx reg=0x%02X: F1[%02X %02X %02X]  F2[%02X %02X %02X]\r\n",
			        (unsigned)reg, b[0],b[1],b[2], b[3],b[4],b[5] );
			printf( "  ([정정]Table6-13: READ는 같은 프레임 byte1/2가 실데이터. F1=F2=정상이면 hi/lo가 reg값)\r\n" );
			return(0);
		}


/* ---------- [1] 함수 추가 ---------- */
        if( !strcmp(argv[1], "MODE") )
        {
            UInt8  m;
            UInt32 scbr = 150U;       /* 기본 1MHz (150MHz/150). 절연지연 크면 키워서 느리게 */
            UInt32 csr;

            if( argc < 3 ) { printf("usage: hpv mode <0-3> [scbr 2-255]\r\n"); return(0); }
            m = (UInt8)htoi( argv[2] ) & 3U;
            if( argc >= 4 ) { scbr = (UInt32)atoi( argv[3] ); }
            if( scbr < 2U )   scbr = 2U;
            if( scbr > 255U ) scbr = 255U;

            {
                extern void DRV3946_SetCsrMode( uint32_t cpolNcpha );
                switch( m )
                {
                case 0:  csr = SPI_CSR_CPOL_IDLE_LOW  | SPI_CSR_NCPHA_VALID_LEADING_EDGE;  break;
                case 1:  csr = SPI_CSR_CPOL_IDLE_LOW  | SPI_CSR_NCPHA_VALID_TRAILING_EDGE; break;
                case 2:  csr = SPI_CSR_CPOL_IDLE_HIGH | SPI_CSR_NCPHA_VALID_LEADING_EDGE;  break;
                default: csr = SPI_CSR_CPOL_IDLE_HIGH | SPI_CSR_NCPHA_VALID_TRAILING_EDGE; break;
                }
                DRV3946_SetCsrMode( csr );   /* [수정] 드라이버 전역에 반영(직접 CSR쓰기는 Read24가 덮어써 무효였음) */
                (void)scbr;
                printf( "DRV3946 SPI mode=%u 적용 -> 'hpv wake'로 DEVICE_ID 확인 (0=M0 1=M1기본 2=M2 3=M3)\r\n", (unsigned)m );
            }
            return(0);
        }
/* ---------- [1] 함수 추가  END ---------- */

        /* SPI0 내부 루프백 셀프테스트: hpv lb -> MCU SPI0 수신경로 검증 */
        if( !strcmp(argv[1], "LB") )
        {
            UInt32 mr = SPI0_REGS->SPI_MR;
            UInt16 rx;
            SPI0_REGS->SPI_CR = SPI_CR_SPIDIS_Msk;
            SPI0_REGS->SPI_MR = mr | SPI_MR_LLB_Msk;
            SPI0_REGS->SPI_CR = SPI_CR_SPIEN_Msk;
            (void)(SPI0_REGS->SPI_RDR);
            while( (SPI0_REGS->SPI_SR & SPI_SR_TDRE_Msk) == 0U ){}
            SPI0_REGS->SPI_TDR = 0x55AAU;
            while( (SPI0_REGS->SPI_SR & SPI_SR_RDRF_Msk) == 0U ){}
            rx = (UInt16)(SPI0_REGS->SPI_RDR & 0xFFFFU);
            SPI0_REGS->SPI_CR = SPI_CR_SPIDIS_Msk;
            SPI0_REGS->SPI_MR = mr;
            SPI0_REGS->SPI_CR = SPI_CR_SPIEN_Msk;
            printf( "SPI0 loopback: tx=0x55AA rx=0x%04X  (0x55AA=MCU RX정상)\r\n",
                    (unsigned)rx );
            return(0);
        }

        /* [정식] DRV3946 웨이크업+통신확인 : hpv init
         * 데이터시트 시퀀스: tREADY 대기 -> REINIT_NAD(CMD2 B14) -> CLR_FAULT(CMD2 B15)
         *   -> STATUS0(1h, 리셋0x2500)/STATUS1(2h, 리셋0x0803) 읽기.
         * 판정: STATUS1 DEVICE_ID(b11:10)=0x2 면 SPI 통신 정상! 0xFFFF면 무응답(SDO Hi-Z). */
        if( !strcmp(argv[1], "INIT") )
        {
            extern void   DRV3946_SPI_Init( void );
            extern UInt16 DRV3946_Cmd24( UInt8 hdr, UInt8 cmd, UInt8 *echo );
            extern UInt16 DRV3946_Read24( UInt8 reg5, UInt8 node2, UInt8 *echo );
            UInt8  ec = 0, e0 = 0, e1 = 0;
            UInt16 st0, st1;
            unsigned did;

            DRV3946_SPI_Init();
            SYSTICK_DelayMs( 3 );                         /* tREADY(max 1ms) 여유 */
            (void)DRV3946_Cmd24( 0x3CU, 0x40U, &ec );     /* CMD2 REINIT_NAD (B14=0x40) */
            SYSTICK_DelayMs( 2 );
            (void)DRV3946_Cmd24( 0x3CU, 0x80U, &ec );     /* CMD2 CLR_FAULT  (B15=0x80) */
            SYSTICK_DelayMs( 2 );
            st0 = DRV3946_Read24( 0x01U, 0U, &e0 );       /* STATUS0 */
            st1 = DRV3946_Read24( 0x02U, 0U, &e1 );       /* STATUS1 */
            did = (unsigned)((st1 >> 10) & 0x3U);

            printf( "DRV3946 wake: REINIT_NAD + CLR_FAULT (CMD2 broadcast) sent\r\n" );
            printf( "  STATUS0=0x%04X echo=0x%02X  [POR(b13)=%u nFLT(b10)=%u DEV_ERR(b9)=%u WARN(b8)=%u]\r\n",
                    (unsigned)st0, (unsigned)e0,
                    (unsigned)((st0>>13)&1U), (unsigned)((st0>>10)&1U),
                    (unsigned)((st0>>9)&1U),  (unsigned)((st0>>8)&1U) );
            printf( "  STATUS1=0x%04X echo=0x%02X  [DEVICE_ID=0x%X]\r\n",
                    (unsigned)st1, (unsigned)e1, did );
            printf( "  echo비트(B23..16): VDD_ERR(b7)=%u NAD_ERR(b6)=%u SPI_ERR(b5)=%u\r\n",
                    (unsigned)((e1>>7)&1U), (unsigned)((e1>>6)&1U), (unsigned)((e1>>5)&1U) );
            if( did == 0x2U )
                printf( "  ==> SPI 통신 정상! (DEVICE_ID=0x2)\r\n" );
            else if( (st0 == 0xFFFFU) && (st1 == 0xFFFFU) )
                printf( "  ==> 무응답(SDO Hi-Z=0xFFFF): 소자 미기동/전원(VDD)/주소(NAD) 확인\r\n" );
            else
                printf( "  ==> 응답은 있으나 ID불일치: 위 echo/STATUS 비트로 원인 판정\r\n" );
            return(0);
        }

        /* [정식] DRV3946 24-bit 레지스터 읽기 : hpv rd <reg5> [node]
         * reg 0x10/0x11 기본값=0xC040, 0x12=0x2424 이면 SPI 정상! */
        if( !strcmp(argv[1], "RD") )
        {
            extern UInt16 DRV3946_Read24( UInt8 reg5, UInt8 node2, UInt8 *echo );
            UInt8  reg  = ( argc >= 3 ) ? (UInt8)htoi( argv[2] ) : 0x10U;
            UInt8  node = ( argc >= 4 ) ? (UInt8)htoi( argv[3] ) : 0U;
            UInt8  echo = 0;
            UInt16 data = DRV3946_Read24( reg, node, &echo );
            printf( "DRV3946 RD reg=0x%02X node=%u -> DATA=0x%04X (echo=0x%02X)\r\n",
                    (unsigned)reg, (unsigned)node, (unsigned)data, (unsigned)echo );
            printf( "  expect: reg0x10/0x11=0xC040, 0x12=0x2424 (echo bit6=1이면 SPI_ERR)\r\n" );
            return(0);
        }

        /* [디버그] 2프레임(파이프라인) 읽기 : hpv r2 [addr16]
         * w1=주소요청, w2=NOP -> RX2가 진짜 데이터면 칩이 파이프라인 읽기 */
        if( !strcmp(argv[1], "R2") )
        {
            extern UInt16 DRV3946_Xfer2( UInt16 w1, UInt16 w2, UInt16 *rx1 );
            UInt16 w1 = ( argc >= 3 ) ? (UInt16)htoi( argv[2] ) : 0x8000U;
            UInt16 r1 = 0, r2;
            r2 = DRV3946_Xfer2( w1, 0x0000U, &r1 );
            printf( "DRV3946 2frame: TX1=0x%04X RX1=0x%04X | TX2=0x0000 RX2=0x%04X\r\n",
                    (unsigned)w1, (unsigned)r1, (unsigned)r2 );
            return(0);
        }

        /* [디버그] 프레임 반복 송신 -> 연속 SCLK/NSCS (스코프 트리거용)
         * hpv loop [count]  (기본 20000회) */
        if( !strcmp(argv[1], "LOOP") )
        {
            UInt32 i, n = 20000U;
            if( argc >= 3 ) n = (UInt32)atoi( argv[2] );
            printf( "DRV3946 SPI loop x%lu (scope SCLK/NSCS/SDO @DGND_ISO) ...\r\n",
                    (unsigned long)n );
            for( i = 0; i < n; i++ ) { (void)DRV3946_Xfer16( 0x8000U ); }
            printf( "loop done\r\n" );
            return(0);
        }

	}

	/* SPI 프레임 송수신 (링크 검증) */
	usOut = ( argc < 2 ) ? 0x8000 : (UInt16)htoi( argv[1] );
	usIn  = DRV3946_Xfer16( usOut );
	printf( "DRV3946 SPI TX=0x%04X RX=0x%04X  nFAULT=%d\r\n",
			usOut, usIn, (int)DRV3946Q1_nFAULT_Get() );
	return(0);					// '0' 리턴
}
/* ---------- [1] 함수 추가 ---------- */
static int testAdcRawFunc(int argc, char *argv[])
{
    UInt8 ch; UInt32 to; UInt16 raw;

    /* ---- AFEC0 : CH0~CH11, CH10(PB0=디버그RX) 제외 ---- */
    for( ch = 0; ch < 12; ch++ ){ if(ch==10) continue;
        AFEC0_REGS->AFEC_CSELR = ch; AFEC0_REGS->AFEC_COCR = 512U; }
    AFEC0_REGS->AFEC_CHER = 0x0BFFU;            /* bit10 제외 */
    AFEC0_REGS->AFEC_CR   = AFEC_CR_START_Msk;
    for( ch = 0; ch < 12; ch++ ){ if(ch==10) continue;
        to = 1000000U;
        while( ((AFEC0_REGS->AFEC_ISR & (1UL<<ch))==0U) && (--to>0U) ){}
        AFEC0_REGS->AFEC_CSELR = ch;
        raw = (UInt16)(AFEC0_REGS->AFEC_CDR & 0xFFFFU);
        printf("AFEC0 CH%02u raw=%4u mV=%4u%s\r\n",(unsigned)ch,(unsigned)raw,
               (unsigned)(((UInt32)raw*3300U)/4095U),(to==0U)?" t/o":"");
    }
    AFEC0_REGS->AFEC_CHDR = 0x0BFFU & ~(AFEC_CHER_CH2_Msk|AFEC_CHER_CH3_Msk);

    /* ---- AFEC1 : CH2~CH11 (CH0=PB1 디버그TX 제외) ---- */
    for( ch = 2; ch < 12; ch++ ){
        AFEC1_REGS->AFEC_CSELR = ch; AFEC1_REGS->AFEC_COCR = 512U; }
    AFEC1_REGS->AFEC_CHER = 0x0FFCU;
    AFEC1_REGS->AFEC_CR   = AFEC_CR_START_Msk;
    for( ch = 2; ch < 12; ch++ ){
        to = 1000000U;
        while( ((AFEC1_REGS->AFEC_ISR & (1UL<<ch))==0U) && (--to>0U) ){}
        AFEC1_REGS->AFEC_CSELR = ch;
        raw = (UInt16)(AFEC1_REGS->AFEC_CDR & 0xFFFFU);
        printf("AFEC1 CH%02u raw=%4u mV=%4u%s\r\n",(unsigned)ch,(unsigned)raw,
               (unsigned)(((UInt32)raw*3300U)/4095U),(to==0U)?" t/o":"");
    }
    AFEC1_REGS->AFEC_CHDR = 0x0FFCU &
        ~(AFEC_CHER_CH2_Msk|AFEC_CHER_CH3_Msk|AFEC_CHER_CH4_Msk|AFEC_CHER_CH5_Msk);
    return(0);
}
 
/* ---------- 내부: AFEC0/1 전채널 캡처 (printf 없음) ---------- */
static void afec_scan_all( UInt16 *buf )    /* buf[24] = AFEC0 0-11, AFEC1 0-11 */
{
    UInt8  ch;
    UInt32 to;

    for( ch = 0; ch < 12; ch++ )
    {
        AFEC0_REGS->AFEC_CSELR = ch;  AFEC0_REGS->AFEC_COCR = 512U;
        AFEC1_REGS->AFEC_CSELR = ch;  AFEC1_REGS->AFEC_COCR = 512U;
    }
    AFEC0_REGS->AFEC_CHER = 0x0FFFU;
    AFEC1_REGS->AFEC_CHER = 0x0FFFU;
    AFEC0_REGS->AFEC_CR   = AFEC_CR_START_Msk;
    AFEC1_REGS->AFEC_CR   = AFEC_CR_START_Msk;

    for( ch = 0; ch < 12; ch++ )
    {
        to = 1000000U;
        while( ((AFEC0_REGS->AFEC_ISR & (1UL << ch)) == 0U) && (--to > 0U) ){}
        AFEC0_REGS->AFEC_CSELR = ch;
        buf[ch] = (UInt16)(AFEC0_REGS->AFEC_CDR & 0xFFFFU);

        to = 1000000U;
        while( ((AFEC1_REGS->AFEC_ISR & (1UL << ch)) == 0U) && (--to > 0U) ){}
        AFEC1_REGS->AFEC_CSELR = ch;
        buf[12U + ch] = (UInt16)(AFEC1_REGS->AFEC_CDR & 0xFFFFU);
    }

    /* 원복: 운용 채널만 enable 상태로 복귀 */
    AFEC0_REGS->AFEC_CHDR = 0x0FFFU & ~(AFEC_CHER_CH2_Msk | AFEC_CHER_CH3_Msk);
    AFEC1_REGS->AFEC_CHDR = 0x0FFFU &
        ~(AFEC_CHER_CH2_Msk | AFEC_CHER_CH3_Msk |
          AFEC_CHER_CH4_Msk | AFEC_CHER_CH5_Msk);
}

/* ---------- pcscan 명령 ---------- */
static int testPcScanFunc(int argc, char *argv[])
{
    static UInt16 hi[24], lo[24];
    UInt8 ch, idx;

    /* PC12 출력 High 구동 -> 스캔 */
    PIOC_REGS->PIO_PER  = (1UL << 12);
    PIOC_REGS->PIO_OER  = (1UL << 12);
    PIOC_REGS->PIO_SODR = (1UL << 12);
    afec_scan_all( hi );

    /* Low 구동 -> 스캔 */
    PIOC_REGS->PIO_CODR = (1UL << 12);
    afec_scan_all( lo );

    /* PC12 입력 원복 */
    PIOC_REGS->PIO_ODR  = (1UL << 12);

    printf( "         HIGH   LOW  (mV)\r\n" );
    for( idx = 0; idx < 24; idx++ )
    {
        ch = idx % 12U;
        printf( "AFEC%u CH%02u %4u  %4u%s\r\n",
                (unsigned)(idx / 12U), (unsigned)ch,
                (unsigned)(((UInt32)hi[idx] * 3300U) / 4095U),
                (unsigned)(((UInt32)lo[idx] * 3300U) / 4095U),
                ((hi[idx] > 3500U) && (lo[idx] < 600U)) ? "  <== PC12!" : "" );
    }
    return(0);
}


typedef struct {
    const char *name;
    UInt8  ch;          /* AFEC1 채널 번호 */
    UInt8  bit;         /* PIOC 비트 번호 */
} sPinChk;

static UInt16 afec1_read_one( UInt8 ch )
{
    UInt32 to = 1000000U;

    AFEC1_REGS->AFEC_CSELR = ch;
    AFEC1_REGS->AFEC_COCR  = 512U;
    AFEC1_REGS->AFEC_CHER  = (1UL << ch);
    AFEC1_REGS->AFEC_CR    = AFEC_CR_START_Msk;
    while( ((AFEC1_REGS->AFEC_ISR & (1UL << ch)) == 0U) && (--to > 0U) ){}
    AFEC1_REGS->AFEC_CSELR = ch;
    return (UInt16)(AFEC1_REGS->AFEC_CDR & 0xFFFFU);
}

static int testPcs2Func(int argc, char *argv[])
{
    static const sPinChk tbl[2] = {
        { "PC30/CH5(ctrl)", 5U, 30U },
        { "PC12/CH3(tgt) ", 3U, 12U },
    };
    UInt8  i;
    UInt8  pdsrH, pdsrL;
    UInt16 cdrH, cdrL;
    volatile UInt32 d;

    for( i = 0; i < 2U; i++ )
    {
        UInt32 msk = (1UL << tbl[i].bit);

        /* 1) 아날로그 분리 + GPIO 출력 설정 */
        AFEC1_REGS->AFEC_CHDR = (1UL << tbl[i].ch);
        PIOC_REGS->PIO_PER = msk;
        PIOC_REGS->PIO_OER = msk;

        /* 2) HIGH 구동 -> PDSR */
        PIOC_REGS->PIO_SODR = msk;
        for( d = 0; d < 10000U; d++ ){}
        pdsrH = (UInt8)((PIOC_REGS->PIO_PDSR >> tbl[i].bit) & 0x1U);

        /* 3) 채널 ON -> CDR */
        cdrH = afec1_read_one( tbl[i].ch );

        /* 4) LOW 구동 -> PDSR, CDR */
        AFEC1_REGS->AFEC_CHDR = (1UL << tbl[i].ch);
        PIOC_REGS->PIO_CODR = msk;
        for( d = 0; d < 10000U; d++ ){}
        pdsrL = (UInt8)((PIOC_REGS->PIO_PDSR >> tbl[i].bit) & 0x1U);
        cdrL  = afec1_read_one( tbl[i].ch );

        /* 5) 원복: 입력 + 채널 enable 유지 */
        PIOC_REGS->PIO_ODR = msk;
        AFEC1_REGS->AFEC_CHER = (1UL << tbl[i].ch);

        printf( "%s  PDSR H=%u L=%u   CDR H=%4umV L=%4umV\r\n",
                tbl[i].name, (unsigned)pdsrH, (unsigned)pdsrL,
                (unsigned)(((UInt32)cdrH * 3300U) / 4095U),
                (unsigned)(((UInt32)cdrL * 3300U) / 4095U) );
    }
    return(0);
}
/* =============================================================
 * amap : AFEC 전 채널 생존 지도 (내부 풀업/풀다운 방식, 무구동·안전)
 * 적용: dbg_task.c
 *
 * 원리:
 *  - 각 아날로그 핀에 내부 풀업(~100k) -> 변환 -> 풀다운 -> 변환
 *  - 풀저항은 약해서 외부 저임피던스 소스(op-amp, UART 드라이버)와
 *    충돌하지 않음 -> 모든 핀에 안전
 *  - PDSR(디지털 리드백)도 함께 기록 -> 디지털/아날로그 분리 판정
 *
 * 자동 판정:
 *  ALIVE : PU/PD 간 CDR 차이 큼 (개방 핀 + 채널 정상)
 *  EXT   : PU/PD 무관 일정 전압 (외부 소스가 핀을 잡고 있음 = 채널 정상)
 *  DEAD? : 핀 상태와 무관하게 CDR이 0 부근 (채널 손상 의심)
 *          단, 외부 넷이 0V로 잡혀 있어도 동일하게 보이므로
 *          PDSR이 PU=1/PD=0으로 움직였다면(개방 핀) DEAD 확정 강화
 *
 * 주의:
 *  - 실행 중(약 1~2초) 콘솔/RS422 핀이 잠깐 아날로그로 전환됨.
 *    키 입력 금지. 출력은 캡처 후 일괄 인쇄라 깨지지 않음.
 *  - 끝나면 풀업/풀다운 전부 해제, 운용 채널만 재enable.
 * ============================================================= */
 
typedef struct {
    UInt8 afec;                 /* 0 / 1 */
    UInt8 ch;                   /* 채널 번호 */
    volatile UInt32 *pioBase;   /* PIOx_REGS 베이스(아래 매크로로 접근) */
    pio_registers_t *pio;       /* 포트 */
    UInt8 bit;                  /* 핀 비트 */
    const char *name;           /* 핀 이름 */
} sAmapEnt;
 
typedef struct {
    UInt16 cdrPu, cdrPd;
    UInt8  pdsrPu, pdsrPd;
} sAmapRes;
 
static UInt16 amap_conv( UInt8 afec, UInt8 ch )
{
    afec_registers_t *r = (afec == 0U) ? AFEC0_REGS : AFEC1_REGS;
    UInt32 to = 1000000U;
 
    r->AFEC_CSELR = ch;
    r->AFEC_COCR  = 512U;
    r->AFEC_CHER  = (1UL << ch);
    r->AFEC_CR    = AFEC_CR_START_Msk;
    while( ((r->AFEC_ISR & (1UL << ch)) == 0U) && (--to > 0U) ){}
    r->AFEC_CSELR = ch;
    return (UInt16)(r->AFEC_CDR & 0xFFFFU);
}
 
static int testAmapFunc(int argc, char *argv[])
{
    /* 핀-채널 표 (MCC core.yml / SAMV71 데이터시트 기준) */
    static const sAmapEnt tbl[] = {
        { 0, 0, 0, PIOD_REGS, 30, "PD30" },
        { 0, 1, 0, PIOA_REGS, 21, "PA21 RS422RX" },
        { 0, 2, 0, PIOB_REGS,  3, "PB3  PRES3" },
        { 0, 3, 0, PIOE_REGS,  5, "PE5  PRES4" },
        { 0, 4, 0, PIOE_REGS,  4, "PE4" },
        { 0, 5, 0, PIOB_REGS,  2, "PB2" },
        { 0, 6, 0, PIOA_REGS, 17, "PA17" },
        { 0, 7, 0, PIOA_REGS, 18, "PA18" },
        { 0, 8, 0, PIOA_REGS, 19, "PA19" },
        { 0, 9, 0, PIOA_REGS, 20, "PA20" },
        { 0,10, 0, PIOB_REGS,  0, "PB0  DBG_RX" },
        { 1, 0, 0, PIOB_REGS,  1, "PB1  DBG_TX" },
        { 1, 1, 0, PIOC_REGS, 13, "PC13 LPV4" },
        { 1, 2, 0, PIOC_REGS, 15, "PC15 P28V_I" },
        { 1, 3, 0, PIOC_REGS, 12, "PC12 P28V_V" },
        { 1, 4, 0, PIOC_REGS, 29, "PC29 SEN5V" },
        { 1, 5, 0, PIOC_REGS, 30, "PC30 SENVDD" },
        { 1, 6, 0, PIOC_REGS, 31, "PC31" },
        { 1, 7, 0, PIOC_REGS, 26, "PC26 SPI1MISO" },
        { 1, 8, 0, PIOC_REGS, 27, "PC27 SPI1MOSI" },
        { 1, 9, 0, PIOC_REGS,  0, "PC0" },
        { 1,10, 0, PIOE_REGS,  3, "PE3" },
        { 1,11, 0, PIOE_REGS,  0, "PE0  HTR1" },
    };
    #define AMAP_N  (sizeof(tbl)/sizeof(tbl[0]))
 
    static sAmapRes res[AMAP_N];
    UInt8 i;
    UInt32 msk;
    const char *verdict;
 
    /* ---- 캡처 (printf 금지 구간) ---- */
    for( i = 0; i < AMAP_N; i++ )
    {
        msk = (1UL << tbl[i].bit);
 
        /* 풀업 ON */
        tbl[i].pio->PIO_PPDDR = msk;            /* 풀다운 해제 먼저 */
        tbl[i].pio->PIO_PUER  = msk;
        SYSTICK_DelayMs( 20 );                  /* 외부 필터캡 정착 */
        res[i].pdsrPu = (UInt8)((tbl[i].pio->PIO_PDSR >> tbl[i].bit) & 0x1U);
        res[i].cdrPu  = amap_conv( tbl[i].afec, tbl[i].ch );
 
        /* 풀다운 ON */
        tbl[i].pio->PIO_PUDR  = msk;
        tbl[i].pio->PIO_PPDER = msk;
        SYSTICK_DelayMs( 20 );
        res[i].pdsrPd = (UInt8)((tbl[i].pio->PIO_PDSR >> tbl[i].bit) & 0x1U);
        res[i].cdrPd  = amap_conv( tbl[i].afec, tbl[i].ch );
 
        /* 풀 해제 + 채널 OFF */
        tbl[i].pio->PIO_PPDDR = msk;
        ((tbl[i].afec == 0U) ? AFEC0_REGS : AFEC1_REGS)->AFEC_CHDR = (1UL << tbl[i].ch);
    }
 
    /* 운용 채널 재enable */
    AFEC0_REGS->AFEC_CHER = AFEC_CHER_CH2_Msk | AFEC_CHER_CH3_Msk;
    AFEC1_REGS->AFEC_CHER = AFEC_CHER_CH2_Msk | AFEC_CHER_CH3_Msk |
                            AFEC_CHER_CH4_Msk | AFEC_CHER_CH5_Msk;
 
    /* ---- 일괄 출력 ---- */
    printf( "                    PU(pdsr/mV)  PD(pdsr/mV)  verdict\r\n" );
    for( i = 0; i < AMAP_N; i++ )
    {
        UInt16 dHi = (res[i].cdrPu > res[i].cdrPd) ?
                     (res[i].cdrPu - res[i].cdrPd) : (res[i].cdrPd - res[i].cdrPu);
 
        if( dHi > 500U )                                     verdict = "ALIVE";
        else if( (res[i].cdrPu > 200U) || (res[i].cdrPd > 200U) ) verdict = "EXT  ";
        else if( (res[i].pdsrPu == 1U) && (res[i].pdsrPd == 0U) ) verdict = "DEAD?";
        else                                                 verdict = "GND/?";
 
        printf( "AFEC%u CH%02u %-14s %u/%4u     %u/%4u     %s\r\n",
                (unsigned)tbl[i].afec, (unsigned)tbl[i].ch, tbl[i].name,
                (unsigned)res[i].pdsrPu,
                (unsigned)(((UInt32)res[i].cdrPu * 3300U) / 4095U),
                (unsigned)res[i].pdsrPd,
                (unsigned)(((UInt32)res[i].cdrPd * 3300U) / 4095U),
                verdict );
    }
    return(0);
}
/**
 * @fn UsrCmdList
 * @brief Initialize and list user commands
 * @date 2023-01-19
 */
static void UsrCmdList(void)
{
	UsrCmdSet( "pself", testPselfFunc, "AFEC self-test: MCU drives pin H/L then ADC reads",'N',"\0");
	UsrCmdSet( "pdrv", testPdrvFunc, "GPIO drive pressure pins: pdrv <0/1> (measure legs)",'N',"\0");
	// 인터페이스 검증 명령 (인터페이스 1:1)
	UsrCmdSet( "tc",   testTcLogFunc, "TC ADS1263 SPI1 log on/off",'N',"\0");
	UsrCmdSet( "tcid", testTcIdFunc,  "TC ADS1263 ID reg read (SPI link test)",'N',"\0");
	UsrCmdSet( "tcmode",testTcModeFunc,"TC SPI 모드 토글: tcmode <0/1> (0=Mode0 정석/기본)",'N',"\0");
	UsrCmdSet( "tcraw",testTcRawFunc, "TC raw diff voltage mV (열전대 검증: 쇼트≈0, 가열시↑)",'N',"\0");
	UsrCmdSet( "tcadc",testTcAdcFunc, "TC ADS보정: vbias/bypass/cal/itemp/power",'N',"\0");
	UsrCmdSet( "tczero",testTcZeroFunc,"TC 0점보정(쇼트상태): tczero <1~6>",'N',"\0");
	UsrCmdSet( "lprtn",testLpRtnFunc, "LP valve RTN enable(PD12): lprtn <0|1>",'N',"\0");
	UsrCmdSet( "tcwr", testTcWrFunc,  "TC ADS1263 WREG/RREG fixed-value test (IC verify)",'N',"\0");
	UsrCmdSet( "tcreset", testTcResetFunc, "ADS1263 re-init after supplies stable (power-timing test)",'N',"\0");
	UsrCmdSet( "adc",  testAdcLogFunc,"AFEC analog log on/off",'N',"\0");
	UsrCmdSet( "acal", testAcalFunc,  "28V 전류센스 보정: acal off(0A) / acal gain <A>(known I)",'N',"\0");
	UsrCmdSet( "pcal", testPcalFunc,  "압력 게인 보정: 주입V 인가하고 pcal <V>",'N',"\0");
    UsrCmdSet( "uart", testUartLogFunc,"RS422 USART1 status or loopback: uart [0|1]",'N',"\0");
    UsrCmdSet( "safe", testSafeFunc,  "EMERGENCY: all actuators OFF (HP/micro/heater)",'N',"\0");
    UsrCmdSet( "pt",   testPtFunc,    "Pressure verify: PT-F1/F2/O1/O2 (0.5~4.5V, %FS)",'N',"\0");
    UsrCmdSet( "tt",   testTtFunc,    "Temperature verify: TT-F1~F3/O1~O3 (K-type, degC)",'N',"\0");
    UsrCmdSet( "hpv",  testHpvFunc,   "HP valve: hpv <1-8> [f] | cycle on/off | node/wake/on/off/stat/init",'N',"\0");
    UsrCmdSet( "off",  testOffFunc,   "ALL OFF (HP 8 + LP 12 + Heater 4 + SP)",'N',"\0");
    UsrCmdSet( "lpv",  testLpvFunc,   "LP valve PWM/cycle: lpv <1-12> <0-100> | cycle on/off",'N',"\0");
    UsrCmdSet( "htr",  testHtrFunc,   "Heater: htr <1-4> <0-100>",'N',"\0");
    UsrCmdSet( "sp",   testSpFunc,    "Spark plug: sp <0|1> (PC5 GPIO)",'N',"\0");
    UsrCmdSet( "htrreg", testHtrRegFunc, "TC3 reg dump (timer/duty check)",'N',"\0");
    UsrCmdSet( "rs485", testRs485Func, "RS485 USART1 TX test: rs485 [count] [hexbyte]",'N',"\0");
    UsrCmdSet( "araw", testAdcRawFunc, "AFEC1 raw scan CH0-11",'N',"\0");
    UsrCmdSet( "pinst", testPinstFunc, "PC12/15/29/30 PIO+AFEC 실태덤프",'N',"\0");
    UsrCmdSet( "ascan", testAscanFunc, "AFEC1 전채널 스캔(2.54V 위치찾기)",'N',"\0");
    UsrCmdSet( "pcs", testPcScanFunc, "PC12 drive + AFEC all-ch scan",'N',"\0");
    UsrCmdSet( "pcs2", testPcs2Func, "pin-level PDSR/CDR drive test",'N',"\0");
    UsrCmdSet( "amap", testAmapFunc, "AFEC all-pin survival map",'N',"\0");
    UsrCmdSet( "adt", testAdtFunc, "AFEC bare 1-shot read: adt <0|1> <ch>",'N',"\0");
}


/**
 * @fn DbgTask
 * @brief Debug task function
 * @param pvParameters - Task parameters (not used in this function)
 * @date 2023-01-19
 */
void DbgTask( void *pvParameters )
{
    /* 명령창 커서문구 */
	promptp="SAM_CTL>";

	/* 초기화 */
	init_cmd();			// 명령 초기화
	UsrCmdInit();		// 사용자 명령 초기화
	UsrCmdList();		// 사용자 명령 등록

	/* 명령창 출력 */
	printf("%s",promptp);

    while(1)
    {
    	/* 명령 수신 */
    	uart_rx_check();

    	/* 동작 지연시간 */
    	vTaskDelay( 10 );
    }
}
