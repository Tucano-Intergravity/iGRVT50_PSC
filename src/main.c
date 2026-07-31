/**
 * @file main.c
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
#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes

/* FreeRTOS includes. */
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

/*==============================================================================
 * Local Function
 *============================================================================*/

/*==============================================================================
 * Functions
 *============================================================================*/

/**
 * @brief 메인 함수
 *
 * @return int
 */
int main ( void )
{
    /* Initialize all modules */
    SYS_Initialize ( NULL );
    
	static TaskHandle_t xOpuTask;		// Operational Unit Task
	static TaskHandle_t xDbgTask;		// Debug Unit Task

	printf( "Start Control v0.7.36 (ADC log: real units 28V/A/rail)\r\n" );

	/* OPU Task 생성 */
	xTaskCreate( OpuTask, 					/* The function that implements the task. */
				( const char * ) "OPU", 	/* Text name for the task, provided to assist debugging only. */
				SCDAU_STACK_SIZE*2, 		/* The stack allocated to the task. */
				NULL, 						/* The task parameter is not used, so set to NULL. */
				tskIDLE_PRIORITY+1,			/* The task runs at the idle priority. */
				&xOpuTask );

	/* DBG Task 생성 */
	xTaskCreate( DbgTask, 					/* The function that implements the task. */
				( const char * ) "DBG", 	/* Text name for the task, provided to assist debugging only. */
				SCDAU_STACK_SIZE, 			/* The stack allocated to the task. */
				NULL, 						/* The task parameter is not used, so set to NULL. */
				tskIDLE_PRIORITY,			/* The task runs at the idle priority. */
				&xDbgTask );

	/* Start the tasks and timer running. */
	vTaskStartScheduler();

    return ( EXIT_FAILURE );
}

/*******************************************************************************
 End of File
*/

