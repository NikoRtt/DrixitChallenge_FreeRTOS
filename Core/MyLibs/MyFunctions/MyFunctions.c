/*****************************************************************************
* General functions for the project
*
* Autor: Nicolas Rasitt
* Created: 30/09/22
*
*****************************************************************************/

/*==================[inclusions]=============================================*/

#include "MyFunctions.h"
#include "../MyInc/Headers.h"
#include "main.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>

/*==================[global variables]======================================*/

static uint8_t BufferUsart1[UART_MAX_RECEIVE_DATA], Escritura_BufferUsart1 = 0, Lectura_BufferUsart1 = 0;

/*==================[internal functions definition]==========================*/

bool_t StoreUSARTData(uint8_t* data, uint16_t size){

	bool_t retVal = FALSE;

	uint8_t auxSize = 0;

	if((Escritura_BufferUsart1 + size) >= UART_MAX_RECEIVE_DATA){

		auxSize = (Escritura_BufferUsart1 + size) - UART_MAX_RECEIVE_DATA;

		memcpy((BufferUsart1 + Escritura_BufferUsart1), data, (size - auxSize));

		Escritura_BufferUsart1 = 0;

		memcpy((BufferUsart1 + Escritura_BufferUsart1), (data + (size - auxSize)), auxSize);

		Escritura_BufferUsart1 = auxSize;
	}

	else {

		memcpy((BufferUsart1 + Escritura_BufferUsart1), data, size);

		Escritura_BufferUsart1 = Escritura_BufferUsart1 + size;
	}

	if(BufferUsart1[Escritura_BufferUsart1 - 1] == '\r' || BufferUsart1[Escritura_BufferUsart1 - 1] == '\n'){

		retVal = TRUE;
	}

	return retVal;
}

/*************************************************************/

bool_t DecodeReceivedData( uint16_t* NumberReceived ){

	bool_t retVal = FALSE;

	uint8_t j = 0;

	char DatosRecibidos[UART_MAX_RECEIVE_DATA];

	while( (Lectura_BufferUsart1 != Escritura_BufferUsart1) ){

		// I remove the characters that are not information
		// If it is any of these characters, nothing is done.
		if( BufferUsart1[Lectura_BufferUsart1] == '\r' ||
			BufferUsart1[Lectura_BufferUsart1] == '\n' ||
			BufferUsart1[Lectura_BufferUsart1] == '\0' ||
			BufferUsart1[Lectura_BufferUsart1] == ' '  ||
			BufferUsart1[Lectura_BufferUsart1] == 0xff){}

		else {

			*(DatosRecibidos + j) = BufferUsart1[Lectura_BufferUsart1];

			j++;
		}

		Lectura_BufferUsart1++;

		if(Lectura_BufferUsart1 >= UART_MAX_RECEIVE_DATA){

			Lectura_BufferUsart1 = 0;
		}
	}

	*(DatosRecibidos + j) = '\0';

	*NumberReceived = atoi(DatosRecibidos);

	if(*NumberReceived != 0){

		retVal = TRUE;
	}

	return retVal; //si no recibi nada valido, vuelve por FALSE
}

/*************************************************************/

void PrintString( UART_HandleTypeDef printer, char* string, uint16_t size ){

	HAL_UART_Transmit( &printer, (uint8_t*)string, size, HAL_MAX_DELAY );
}

/*************************************************************/

void PrintEnter( UART_HandleTypeDef printer ){

    PrintString( printer, PRINT_ENTER_STRING, sizeof(PRINT_ENTER_STRING) );
}

/*************************************************************/

void PrintIntFormat( UART_HandleTypeDef printer, int16_t number ){

	char DataNumber[10];

	memset (DataNumber,'\0', 10);

	itoa(number, DataNumber, 10);

	PrintString( printer, DataNumber, sizeof(DataNumber) );
}

/*************************************************************/

void PrintFloat( UART_HandleTypeDef printer, float number, uint8_t afterpoint ){

    char strNumber[65];

    uint16_t sizeStrNumber = floatToString( number, strNumber, afterpoint);

    PrintString( printer, strNumber, sizeStrNumber);
}

/*************************************************************/

uint16_t floatToString( float number, char* String, uint8_t afterpoint ){

    char strNumber[65];

    uint8_t point = 0;

    int32_t ipart = (int32_t)number; // Extract integer part

    itoa(ipart, strNumber, 10); // convert integer part to string

    int32_t aux = ipart;

    if(afterpoint > 0){

    	if(aux < 0){ point++; } //Sumo un lugar para que entre el menos en el string

        while(aux != 0){

            aux = aux/10;

            point++;
        }

        if(point > 0){

            strNumber[point] = '.';  // add dot

            float fpart = number - (float)ipart; // Extract floating part

            if(fpart < 0){fpart = fpart*(-1); }

            aux = 0;

            ipart = (int32_t)(fpart*10);

            if(ipart == 0){  point++; strNumber[point] = '0'; } // Si el primer decimal despues de la coma es 0 lo cargo a mano.

            while(aux != afterpoint){

                fpart = fpart*10;

                aux++;
            }

            char * pointer;

            pointer = strNumber;

            itoa(fpart, (pointer + point + 1), 10); // convert integer part to string
        }
    }

    memcpy(String, strNumber, (point + 1 + aux + 1)); //enteros + punto + decimales + fin de string

    return (point + 1 + aux + 1);//enteros + punto + decimales + fin de string
}

/*==================[end of file]============================================*/
