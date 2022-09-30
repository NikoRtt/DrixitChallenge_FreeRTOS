/*****************************************************************************
* Funciones para Operacion de EFM32TG11BF120 Tiny Gecko
*
* Autor: Nicolas Rasitt
* Fecha: 06/08/18
* Version: 1.0.0
*
* Copyright 2019, EXO S.A.
* All rights reserved.
*
*****************************************************************************/

#ifndef EFM32_FUNCTIONS_H
#define EFM32_FUNCTIONS_H

/*==================[inclusions]=============================================*/


#include <sys/_stdint.h>

#include "../MyInc/Datatypes.h"

#include "stm32f1xx_hal.h"

/*==================[macros and definitions]=================================*/

#define PRINT_ENTER_STRING   "\r\n"
#define UART_MAX_RECEIVE_DATA 25

typedef enum{
   BIN_FORMAT = 2,
   DEC_FORMAT = 10,
   HEX_FORMAT = 16
} numberFormat_t;

/*==================[macros]=================================================*/

//#warning "Quitar impresiones de debug"
#define DEBUG_PRINT   ON
//#define DEBUG_PRINT   OFF

#if( DEBUG_PRINT == ON )

    #define debugPrintChar( character );                PrintChar( UART_LE, character );
    #define debugPrintString( string );                 PrintString( UART_LE, string );
    #define debugPrintEnter();                          PrintEnter( UART_LE );
    #define debugPrintIntFormat( number, format );      PrintIntFormat( UART_LE, number, format );
    #define debugPrintUIntFormat( number, format );     PrintUIntFormat( UART_LE, number, format );
    #define debugPrintFloat( number, afterpoint );      PrintFloat( UART_LE, number, afterpoint );

#else

    #define debugPrintChar( character );                ;
    #define debugPrintString( string );                 ;
    #define debugPrintEnter();                          ;
    #define debugPrintIntFormat( number, format );      ;
    #define debugPrintUIntFormat( number, format );     ;
    #define debugPrintFloat( number, afterpoint );      ;

#endif

/*==================[internal functions declaration]=========================*/

bool_t StoreUSARTData ( uint8_t* data, uint16_t size );

bool_t DecodeReceivedData ( uint16_t* NumberReceived );

void PrintString ( UART_HandleTypeDef printer, char* string, uint16_t size );

void PrintEnter ( UART_HandleTypeDef printer );

void PrintIntFormat ( UART_HandleTypeDef printer, int16_t number );

bool_t I2C_WriteRead ( uint8_t , uint8_t* , uint16_t , bool_t , uint8_t* , uint16_t  );

bool_t I2C_Read ( uint8_t i2cSlaveAddress, uint8_t* receiveDataBuffer, uint16_t receiveDataBufferSize );

bool_t I2C_Write ( uint8_t i2cSlaveAddress, uint8_t* transmitDataBuffer, uint16_t transmitDataBufferSize);

bool_t I2C_ReadRegisterU8 (uint8_t I2C_Address, uint8_t Register, uint8_t* RegisterValue);

bool_t I2C_ReadRegisterU16 (uint8_t I2C_Address, uint8_t Register, uint16_t* RegisterValue);

bool_t I2C_ReadRegisterS16 (uint8_t I2C_Address, uint8_t Register, int16_t* RegisterValue);

bool_t I2C_WriteRegisterU8 (uint8_t I2C_Address, uint8_t Register, uint8_t RegisterValue);

#endif

/*****************************************************************************/
