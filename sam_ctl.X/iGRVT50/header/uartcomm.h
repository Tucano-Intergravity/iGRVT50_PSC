#ifndef IGRVT50_UARTCOMM_H
#define IGRVT50_UARTCOMM_H

#include <stdint.h>

#include "sam_ctl.h"

#define UARTCOMM_PORT_USART1        1U
#define UARTCOMM_DEFAULT_BAUDRATE   921600UL

void UartComm_Init( UInt32 baudRate );
void UartComm_SendBlocking( const void *data, UInt32 length );
void UartComm_SendByteRepeatBlocking( UInt8 data, UInt32 count );
void UartComm_SendStringBlocking( const char *text );
SInt32 UartComm_Read( sRbData *rxData );
void UartComm_ServiceLoopback( UInt16 enable );
UInt32 UartComm_GetRxByteCount( void );
UInt32 UartComm_GetRxDropCount( void );
UInt32 UartComm_GetRxErrorCount( void );

void USART1_ReadCallback( uintptr_t context );

#endif /* IGRVT50_UARTCOMM_H */
