#include "uartcomm.h"

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "definitions.h"

#define UARTCOMM_RX_QUEUE_DEPTH     2048U
#define UARTCOMM_RX_INT_MASK        (US_IER_USART_RXRDY_Msk | US_IER_USART_FRAME_Msk | US_IER_USART_PARE_Msk | US_IER_USART_OVRE_Msk)

static UInt8 s_rxQueue[UARTCOMM_RX_QUEUE_DEPTH];
static volatile UInt32 s_rxFront = 0U;
static volatile UInt32 s_rxRear = 0U;
static volatile UInt32 s_rxCount = 0U;
static volatile UInt32 s_rxDropCount = 0U;
static volatile UInt32 s_rxByteCount = 0U;
static volatile UInt32 s_rxErrorCount = 0U;
static volatile UInt8 s_rxIsrEnabled = 0U;
static volatile UInt8 s_uartInitialized = 0U;

static void UartComm_ResetRxQueue( void )
{
    __disable_irq();
    s_rxFront = 0U;
    s_rxRear = 0U;
    s_rxCount = 0U;
    s_rxDropCount = 0U;
    s_rxByteCount = 0U;
    s_rxErrorCount = 0U;
    __enable_irq();
}

static void UartComm_FlushHardwareRx( void )
{
    uint32_t dummyData = 0U;

    USART1_REGS->US_CR = US_CR_USART_RSTSTA_Msk;
    while( (USART1_REGS->US_CSR & US_CSR_USART_RXRDY_Msk) != 0U )
    {
        dummyData = USART1_REGS->US_RHR & US_RHR_RXCHR_Msk;
    }

    (void)dummyData;
}

static void UartComm_EnableRxInterrupt( void )
{
    s_rxIsrEnabled = 1U;
    USART1_REGS->US_CR = US_CR_USART_RXEN_Msk;
    USART1_REGS->US_IER = UARTCOMM_RX_INT_MASK;
}

static void UartComm_EnqueueRxByte( UInt8 data )
{
    if( s_rxCount >= UARTCOMM_RX_QUEUE_DEPTH )
    {
        s_rxFront = (s_rxFront + 1U) % UARTCOMM_RX_QUEUE_DEPTH;
        s_rxCount--;
        s_rxDropCount++;
    }

    s_rxQueue[s_rxRear] = data;
    s_rxRear = (s_rxRear + 1U) % UARTCOMM_RX_QUEUE_DEPTH;
    s_rxCount++;
    s_rxByteCount++;
}

void UartComm_Init( UInt32 baudRate )
{
    if( s_uartInitialized != 0U )
    {
        return;
    }

    s_rxIsrEnabled = 0U;
    UartComm_ResetRxQueue();
    RS422_Init( baudRate );
    (void)USART1_ReadAbort();
    UartComm_FlushHardwareRx();
    UartComm_EnableRxInterrupt();
    s_uartInitialized = 1U;
}

void UartComm_SendBlocking( const void *data, UInt32 length )
{
    if( (data == NULL) || (length == 0U) )
    {
        return;
    }

    RS485_SetTransmit( 1U );
    while( USART1_WriteIsBusy() ) { }
    (void)USART1_Write( (void *)data, (size_t)length );
    while( USART1_WriteIsBusy() ) { }
    while( !USART1_TransmitComplete() ) { }
    RS485_SetTransmit( 0U );
}

void UartComm_SendByteRepeatBlocking( UInt8 data, UInt32 count )
{
    UInt32 i;

    if( count == 0U )
    {
        return;
    }

    RS485_SetTransmit( 1U );
    for( i = 0U; i < count; i++ )
    {
        while( USART1_WriteIsBusy() ) { }
        (void)USART1_Write( &data, 1U );
    }
    while( USART1_WriteIsBusy() ) { }
    while( !USART1_TransmitComplete() ) { }
    RS485_SetTransmit( 0U );
}

void UartComm_SendStringBlocking( const char *text )
{
    if( text == NULL )
    {
        return;
    }

    UartComm_SendBlocking( text, (UInt32)strlen( text ) );
}

SInt32 UartComm_Read( sRbData *rxData )
{
    UInt8 rxByte;
    UInt32 remaining;

    if( rxData == NULL )
    {
        return -1;
    }

    __disable_irq();
    if( s_rxCount == 0U )
    {
        __enable_irq();
        return -1;
    }

    rxByte = s_rxQueue[s_rxFront];
    s_rxFront = (s_rxFront + 1U) % UARTCOMM_RX_QUEUE_DEPTH;
    s_rxCount--;
    remaining = s_rxCount;
    __enable_irq();

    rxData->usSize = 1U;
    rxData->ucData[0] = rxByte;

    return (SInt32)remaining;
}

void UartComm_ServiceLoopback( UInt16 enable )
{
    sRbData rxData;

    while( UartComm_Read( &rxData ) >= 0 )
    {
        if( enable != 0U )
        {
            UartComm_SendBlocking( rxData.ucData, rxData.usSize );
        }
    }
}

void USART1_ReadCallback( uintptr_t context )
{
    (void)context;
}

UInt32 UartComm_GetRxByteCount( void )
{
    return s_rxByteCount;
}

UInt32 UartComm_GetRxDropCount( void )
{
    return s_rxDropCount;
}

UInt32 UartComm_GetRxErrorCount( void )
{
    return s_rxErrorCount;
}

bool USART1_UartCommRxReadyHook( void )
{
    if( s_rxIsrEnabled == 0U )
    {
        return false;
    }

    while( (USART1_REGS->US_CSR & US_CSR_USART_RXRDY_Msk) != 0U )
    {
        UartComm_EnqueueRxByte( (UInt8)(USART1_REGS->US_RHR & US_RHR_RXCHR_Msk) );
    }

    return true;
}

bool USART1_UartCommErrorHook( uint32_t errorStatus )
{
    if( s_rxIsrEnabled == 0U )
    {
        return false;
    }

    (void)errorStatus;
    s_rxErrorCount++;
    UartComm_FlushHardwareRx();
    USART1_REGS->US_IER = UARTCOMM_RX_INT_MASK;

    return true;
}
