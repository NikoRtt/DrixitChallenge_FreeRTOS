/*****************************************************************************
* General functions for the project
*
* Autor: Nicolas Rasitt
* Created: 30/09/22
*
*****************************************************************************/

#ifndef EFM32_FUNCTIONS_H
#define EFM32_FUNCTIONS_H

/*==================[inclusions]=============================================*/


#include <sys/_stdint.h>

#include "../MyInc/Datatypes.h"

#include "stm32f1xx_hal.h"

/*==================[definitions]============================================*/

#define PRINT_ENTER_STRING   "\r\n"
#define UART_MAX_RECEIVE_DATA 25

typedef enum{
   BIN_FORMAT = 2,
   DEC_FORMAT = 10,
   HEX_FORMAT = 16
} numberFormat_t;

/*==================[macros]=================================================*/

/*==================[internal functions declaration]=========================*/

bool_t StoreUSARTData ( uint8_t* data, uint16_t size );

bool_t DecodeReceivedData ( uint16_t* NumberReceived );

void PrintString ( UART_HandleTypeDef printer, char* string, uint16_t size );

void PrintEnter ( UART_HandleTypeDef printer );

void PrintIntFormat ( UART_HandleTypeDef printer, int16_t number );

#endif

/*****************************************************************************/
