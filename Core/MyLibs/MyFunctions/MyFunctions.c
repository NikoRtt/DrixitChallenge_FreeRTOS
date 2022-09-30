/*****************************************************************************
* General functions for the project
*
* Autor: Nicolas Rasitt
* Date: 30/09/22
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

//---------------------------------
//-Store in a buffer all the receive data (circular buffer)
//---------------------------------

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

//---------------------------------
//-Decode the data received from uart
//---------------------------------

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

//---------------------------------
//-Prints string
//---------------------------------

void PrintString( UART_HandleTypeDef printer, char* string, uint16_t size ){

	HAL_UART_Transmit( &printer, (uint8_t*)string, size, HAL_MAX_DELAY );
}

//---------------------------------
//-Prints enter
//---------------------------------

void PrintEnter( UART_HandleTypeDef printer ){

    PrintString( printer, PRINT_ENTER_STRING, sizeof(PRINT_ENTER_STRING) );
}

//---------------------------------
//-Prints integer
//---------------------------------

void PrintIntFormat( UART_HandleTypeDef printer, int16_t number ){

	char DataNumber[10];

	memset (DataNumber,'\0', 10);

	itoa(number, DataNumber, 10);

	PrintString( printer, DataNumber, sizeof(DataNumber) );
}

//---------------------------------
//-Leer y Escribir combinados el I2C como master
//---------------------------------
/*
bool_t I2C_WriteRead( uint8_t i2cSlaveAddress, uint8_t* dataToReadBuffer, uint16_t dataToReadBufferSize, bool_t sendWriteStop,
						uint8_t* receiveDataBuffer, uint16_t receiveDataBufferSize ){

	bool_t retVal = TRUE;

	if( (dataToReadBuffer == NULL)  || (dataToReadBufferSize < 0) || (receiveDataBuffer == NULL) ){ // Check Errors:

	   return FALSE;
	}

	I2C_TransferSeq_TypeDef i2cTransfer; // Transfer structure

	i2cTransfer.addr = i2cSlaveAddress << 1; // Set I2C transfer address

	I2C_TransferReturn_TypeDef result; // Initialize I2C transfer

	if( sendWriteStop ){ // S+ADDR(W)+DATA0+P+S+ADDR(R)+DATA1+P.

        if( dataToReadBufferSize > 0 ){ // First Write: S+ADDR(W)+DATA0+P

            retVal &= I2C_Write( i2cSlaveAddress, dataToReadBuffer, dataToReadBufferSize);
        }

        // Then Read: S+ADDR(R)+DATA1+P.
        i2cTransfer.flags         = I2C_FLAG_READ;
        i2cTransfer.buf[0].data   = receiveDataBuffer;
        i2cTransfer.buf[0].len    = receiveDataBufferSize;

	}

	else { // S+ADDR(W)+DATA0+Sr+ADDR(R)+DATA1+P.

        // Write and Read (without write stop)
        i2cTransfer.flags         = I2C_FLAG_WRITE_READ;
        i2cTransfer.buf[0].data   = dataToReadBuffer;
        i2cTransfer.buf[0].len    = dataToReadBufferSize;

        // Note that WRITE_WRITE this is tx2 data
        i2cTransfer.buf[1].data   = receiveDataBuffer;
        i2cTransfer.buf[1].len    = receiveDataBufferSize;
	}

	result = I2C_TransferInit( I2C0, &i2cTransfer ); // Set up the transfer

    static delay_t Delay_5ms; // Estructura con la que voy a esperar 1 segundo no bloqueante.(Aloja tiempo)

	DELAYNonBlock_Config(&Delay_5ms, 5); // 5 ms

	while( result != i2cTransferDone ){ // Do it until the transfer is done

		result = I2C_Transfer(I2C0);

		if( DELAY_Read(&Delay_5ms) ){

			return FALSE;
		}
	}

	return retVal;
}
*/
//---------------------------------
//-Leer el I2C como master
//---------------------------------
/*
bool_t I2C_Read( uint8_t i2cSlaveAddress, uint8_t* receiveDataBuffer, uint16_t receiveDataBufferSize ){

    bool_t retVal = TRUE;

	if( (receiveDataBuffer == NULL)  || (receiveDataBufferSize < 0) ){ // Check Errors:

	   return FALSE;
	}

    I2C_TransferSeq_TypeDef i2cTransfer; // Transfer structure

    // Initialize I2C transfer
    i2cTransfer.addr = i2cSlaveAddress << 1; // Set I2C transfer address
    i2cTransfer.flags         = I2C_FLAG_READ;
    i2cTransfer.buf[0].data   = receiveDataBuffer;
    i2cTransfer.buf[0].len    = receiveDataBufferSize;

    // Note that WRITE_WRITE this is tx2 data
	i2cTransfer.buf[1].data   = 0;
	i2cTransfer.buf[1].len    = 0;

	I2C_TransferReturn_TypeDef result;

	result = I2C_TransferInit( I2C0, &i2cTransfer ); // Set up the transfer

    static delay_t Delay_5ms; // Estructura con la que voy a esperar 1 segundo no bloqueante.(Aloja tiempo)

	DELAYNonBlock_Config(&Delay_5ms, 5); // 5 ms

	while( result != i2cTransferDone ){ // Do it until the transfer is done

		result = I2C_Transfer(I2C0);

		if( DELAY_Read(&Delay_5ms) ){

			return FALSE;
		}
	}

	return retVal;
}
*/
//---------------------------------
//-Escribir el I2C como master
//---------------------------------
/*
bool_t I2C_Write( uint8_t i2cSlaveAddress, uint8_t* transmitDataBuffer, uint16_t transmitDataBufferSize){

	if( (transmitDataBuffer == NULL)  || (transmitDataBufferSize < 0) ){ // Check Errors:

	   return FALSE;
	}

	I2C_TransferSeq_TypeDef i2cTransfer; // Transfer structure

	// Initialize I2C transfer
	i2cTransfer.addr          = i2cSlaveAddress << 1;
	i2cTransfer.flags         = I2C_FLAG_WRITE; // I2C_FLAG_WRITE_READ;
	i2cTransfer.buf[0].data   = transmitDataBuffer;
	i2cTransfer.buf[0].len    = transmitDataBufferSize;

	// Note that WRITE_WRITE this is tx2 data
	i2cTransfer.buf[1].data   = 0;
	i2cTransfer.buf[1].len    = 0;

	I2C_TransferReturn_TypeDef result;

	result = I2C_TransferInit(I2C0, &i2cTransfer); // Set up the transfer

    static delay_t Delay_5ms; // Estructura con la que voy a esperar 1 segundo no bloqueante.(Aloja tiempo)

	DELAYNonBlock_Config(&Delay_5ms, 5); // 5 ms

	while( result != i2cTransferDone ){ // Do it until the transfer is done

		result = I2C_Transfer(I2C0);

		if( DELAY_Read(&Delay_5ms) ){

			return FALSE;
		}
	}

	return TRUE;
}
*/
//---------------------------------
//-Lectura de registro del I2C
//---------------------------------
/*
bool_t I2C_ReadRegisterU8(uint8_t I2C_Address, uint8_t Register, uint8_t* RegisterValue){

	uint8_t transmitDataBuffer = Register, retVal = FALSE;

	uint8_t receivedDataBuffer = 0;

	retVal = I2C_WriteRead( I2C_Address, &transmitDataBuffer, 1, FALSE, &receivedDataBuffer, 1);

	if(retVal == TRUE){

		*RegisterValue = receivedDataBuffer;

		return TRUE;
	}

	else { return FALSE; }
}
*/
//---------------------------------
//-Lectura de 2 registros del I2C
//---------------------------------
/*
bool_t I2C_ReadRegisterU16(uint8_t I2C_Address, uint8_t Register, uint16_t* RegisterValue){

	uint8_t transmitDataBuffer = Register, retVal = FALSE;

	uint8_t receivedDataBuffer[2];

	retVal = I2C_WriteRead( I2C_Address, &transmitDataBuffer, 1, FALSE, &receivedDataBuffer, 2);

	if(retVal == TRUE){

		*RegisterValue = (receivedDataBuffer[0] << 8) | receivedDataBuffer[1];

		return TRUE;
	}

	else { return FALSE; }
}
*/
//---------------------------------
//-Lectura de 2 registro Signados del I2C
//---------------------------------
/*
bool_t I2C_ReadRegisterS16(uint8_t I2C_Address, uint8_t Register, int16_t* RegisterValue){

	uint16_t temp = 0;

	if( I2C_ReadRegisterU16(I2C_Address, Register, &temp) == TRUE) {

		*RegisterValue = (int16_t)temp;

		return TRUE;
	}

	else return FALSE;
}
*/
//---------------------------------
//-Escritura de registros del I2C
//---------------------------------
/*
bool_t I2C_WriteRegisterU8(uint8_t I2C_Address, uint8_t Register, uint8_t RegisterValue){

	uint8_t transmitDataBuffer [2], retVal = FALSE;

	transmitDataBuffer[0] = Register;

	transmitDataBuffer[1] = RegisterValue;

	retVal = I2C_Write( I2C_Address, &transmitDataBuffer, 2);

	if(retVal == TRUE){ return TRUE; }

	else { return FALSE; }
}
*/
/*==================[end of file]============================================*/
