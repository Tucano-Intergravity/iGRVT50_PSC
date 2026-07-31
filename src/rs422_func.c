/**
 * @file rs422_func.c
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

/*==============================================================================
 * Local Variables
 *============================================================================*/
#define RS485_DE_PA22_MASK      (1UL << 22)    /* UART1_DE  */
#define RS485_NRE_PA24_MASK     (1UL << 24)    /* UART1_nRE */
#define RS485_DIR_MASK          (RS485_DE_PA22_MASK | RS485_NRE_PA24_MASK)

/*==============================================================================
 * Local Function
 *============================================================================*/
static void RS485_GpioInit( void );

/*==============================================================================
 * Functions
 *============================================================================*/
static void RS485_GpioInit( void )
{
    /* PA22=DE, PA24=/RE. PA24 is generated as USART1_RTS1, so recover it as GPIO. */
    PIOA_REGS->PIO_PER = RS485_DIR_MASK;
    PIOA_REGS->PIO_OER = RS485_DIR_MASK;
    PIOA_REGS->PIO_PUDR = RS485_DIR_MASK;
    PIOA_REGS->PIO_PPDDR = RS485_DIR_MASK;

    RS485_SetTransmit( 0U );
}

void RS485_SetTransmit( UInt8 ucEnable )
{
    if( ucEnable != 0U )
    {
        PIOA_REGS->PIO_SODR = RS485_DIR_MASK;     /* TX: DE=1, /RE=1 */
    }
    else
    {
        PIOA_REGS->PIO_CODR = RS485_DIR_MASK;     /* RX/Idle: DE=0, /RE=0 */
    }
}

void RS422_Init( UInt32 uiBaudRate )
{
    USART_SERIAL_SETUP stSetup;

    /* USART1 = RS422 (PA21 RXD1 / PB4 TXD1) */
    stSetup.baudRate  = uiBaudRate;
    stSetup.parity    = USART_PARITY_NONE;
    stSetup.dataWidth = USART_DATA_8_BIT;
    stSetup.stopBits  = USART_STOP_1_BIT;

    /* srcClkFreq=0 -> plib가 내부 클럭(USART1_FrequencyGet) 사용 */
    USART1_SerialSetup( &stSetup, 0 );

    USART1_ReadCallbackRegister( USART1_ReadCallback, 0 );
    RS485_GpioInit();
}


/*******************************************************************************
 End of File
*/
