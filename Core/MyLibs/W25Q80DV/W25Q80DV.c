/*****************************************************************************
* Functions for the W25Q80DV Flash Memory
*
* Autor: Nicolas Rasitt
* Created: 01/10/22
*
*****************************************************************************/
/*==================[inclusions]=============================================*/

#include "W25Q80DV.h"
#include "../MyInc/Headers.h"
#include <stdio.h>
#include <string.h>
#include "cmsis_os.h"

/*==================[global variables]=======================================*/

/*==================[internal functions declaration]=========================*/

// low level functions definition

HAL_StatusTypeDef w25q80dv_Read( W25Q80DV_Data_t* dev, uint8_t* data, uint8_t dataLenght, uint8_t* value, uint8_t valueLenght );

HAL_StatusTypeDef w25q80dv_Write( W25Q80DV_Data_t* dev, uint8_t* data, uint8_t dataLenght );

// Instruction functions definition

bool_t w25q80dv_InstructionReadID( W25Q80DV_Data_t* dev );

HAL_StatusTypeDef w25q80dv_InstructionWriteEnable( W25Q80DV_Data_t* dev );

HAL_StatusTypeDef w25q80dv_InstructionWriteDisable( W25Q80DV_Data_t* dev );

uint8_t w25q80dv_InstructionReadStatusRegister( W25Q80DV_Data_t* dev, uint8_t selectRegister);

void w25q80dv_InstructionWaitForWriteEnd( W25Q80DV_Data_t* dev );

/*==================[internal functions definition]==========================*/

/*==================[low level functions definition]=========================*/

HAL_StatusTypeDef w25q80dv_Read( W25Q80DV_Data_t* dev, uint8_t* data, uint8_t dataLenght, uint8_t* value, uint8_t valueLenght ){

	uint8_t retVal;

	HAL_GPIO_WritePin(dev->csPort, dev->csPin, GPIO_PIN_RESET);

	HAL_SPI_Transmit(dev->W25Q80DV_SPI, data, dataLenght, HAL_MAX_DELAY);

	retVal = HAL_SPI_Receive(dev->W25Q80DV_SPI, value, valueLenght, HAL_MAX_DELAY);

	HAL_GPIO_WritePin(dev->csPort, dev->csPin, GPIO_PIN_SET);

	return retVal;
}

/*************************************************************/

HAL_StatusTypeDef w25q80dv_Write( W25Q80DV_Data_t* dev, uint8_t* data, uint8_t dataLenght ){

	uint8_t retVal;

	HAL_GPIO_WritePin(dev->csPort, dev->csPin, GPIO_PIN_RESET);

	retVal = HAL_SPI_Transmit(dev->W25Q80DV_SPI, data, dataLenght, HAL_MAX_DELAY);

	HAL_GPIO_WritePin(dev->csPort, dev->csPin, GPIO_PIN_SET);

	return retVal;
}

/*==================[Instruction functions definition]=======================*/

bool_t w25q80dv_InstructionReadID( W25Q80DV_Data_t* dev ){

	uint8_t instruction[] = {W25Q80DV_DEVID, W25Q80DV_DUMMY_BYTE, W25Q80DV_DUMMY_BYTE, W25Q80DV_DUMMY_BYTE};

	uint8_t devID = W25Q80DV_ERROR_BYTE;

	bool_t retVal = FALSE;

	if(w25q80dv_Read(dev, instruction, 4, &devID, 1) == HAL_OK){

		if(devID == W25Q80DV_ID){

			retVal = TRUE;
		}
	}

	return retVal;
}

/*************************************************************/

HAL_StatusTypeDef w25q80dv_InstructionWriteEnable( W25Q80DV_Data_t* dev ){

	uint8_t instruction = W25Q80DV_WRITE_ENABLE;

	return w25q80dv_Write(dev, &instruction, 1);
}

/*************************************************************/

HAL_StatusTypeDef w25q80dv_InstructionWriteDisable( W25Q80DV_Data_t* dev ){

	uint8_t instruction = W25Q80DV_WRITE_DISABLE;

	return w25q80dv_Write(dev, &instruction, 1);
}

/*************************************************************/

uint8_t w25q80dv_InstructionReadStatusRegister( W25Q80DV_Data_t* dev, uint8_t selectRegister){

	uint8_t instruction;

	uint8_t statusRegister = W25Q80DV_ERROR_BYTE;

	if (selectRegister == W25Q80DV_STATUSREGISTER_1 ){

		instruction = W25Q80DV_READ_SR1;

		w25q80dv_Read(dev, &instruction, 1, &statusRegister, 1);
	}

	else if (selectRegister == 2){

		instruction = W25Q80DV_READ_SR2;

		w25q80dv_Read(dev, &instruction, 1, &statusRegister, 1);
	}

	return statusRegister;
}

/*************************************************************/

void w25q80dv_InstructionWaitForWriteEnd( W25Q80DV_Data_t* dev ){

	uint8_t statusRegister1;

	do{
		statusRegister1 = w25q80dv_InstructionReadStatusRegister(dev, W25Q80DV_READ_SR1);

		osDelay(1);

	} while ((statusRegister1 & W25Q80DV_WEL_BIT) == W25Q80DV_WEL_ENABLE);
}

/*==================[Initialization functions definition]====================*/

uint8_t w25q80dv_Init( W25Q80DV_Data_t* dev, SPI_HandleTypeDef* spi, uint16_t pin, GPIO_TypeDef* port ){

	dev->block = TRUE;

	dev->W25Q80DV_SPI = spi;

	dev->csPort = port;

	dev->csPin = pin;

	dev->lastPage = 0;

	dev->lastAddress = W25Q80DV_FIRST_PAGE_ADDRESS;

	// We make sure that the memory start properly

	uint8_t instruction = W25Q80DV_RELEASE_PD;

	w25q80dv_Write(dev, &instruction, 1);

	osDelay(100);

	// Check the flash memory

	bool_t retVal = FALSE;

	if(w25q80dv_InstructionReadID(dev)){

		retVal = TRUE;
	}

	dev->block = FALSE;

	return retVal;
}

/*************************************************************/

bool_t w25q80dv_InitAddress( W25Q80DV_Data_t* dev ){

	dev->block = TRUE;

	// We read the first 6 bytes of the memory where the data of last address
	// and page are store, to know if the memory was used or not.

	dev->lastPage = 0;

	dev->lastAddress = W25Q80DV_FIRST_PAGE_ADDRESS;

	uint8_t data[6];

	w25q80dv_ReadBytes(dev, data, 6);

	// Check the data in memory

	bool_t retVal = FALSE;

	uint32_t _lastAddress = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];

	uint16_t _lastPage = (data[4] << 8) | data[5];

	if(_lastPage != W25Q80DV_NO_PAGE){

		retVal = TRUE;

		dev->lastAddress = _lastAddress;

		dev->lastPage = _lastPage;
	}

	dev->block = FALSE;

	return retVal;
}

/*==================[Basic functions definition]============================*/

void w25q80dv_EraseChip( W25Q80DV_Data_t* dev ){

	while (dev->block == TRUE){

		osDelay(1);
	}

	dev->block = TRUE;

	w25q80dv_InstructionWaitForWriteEnd(dev);

	w25q80dv_InstructionWriteEnable(dev);

	uint8_t instruction = W25Q80DV_CHIP_ERASE;

	w25q80dv_Write(dev, &instruction, 1);

	w25q80dv_InstructionWaitForWriteEnd(dev);

	dev->block = FALSE;
}

/*************************************************************/

void w25q80dv_WriteBytes( W25Q80DV_Data_t* dev, uint8_t* data, uint8_t dataLenght ){

	while (dev->block == TRUE){

		osDelay(1);
	}

	dev->block = TRUE;

	w25q80dv_InstructionWaitForWriteEnd(dev);

	w25q80dv_InstructionWriteEnable(dev);

	uint8_t instruction[4 + W25Q80DV_PAGE_SIZE]; // Instruction + 3 byte Address + Data

	instruction[0] = W25Q80DV_PAGE_PROGRAM;

	instruction[1] = (dev->lastAddress & 0xFF0000) >> 16;

	instruction[2] = (dev->lastAddress & 0xFF00) >> 8;

	instruction[3] = (dev->lastAddress & 0xFF);

	memcpy((instruction + 4), data, dataLenght);

	w25q80dv_Write(dev, instruction, 4 + dataLenght);

	w25q80dv_InstructionWaitForWriteEnd(dev);

	dev->block = FALSE;
}

/*************************************************************/

void w25q80dv_ReadBytes( W25Q80DV_Data_t* dev, uint8_t* data, uint8_t dataLenght ){

	while (dev->block == TRUE){

		osDelay(1);
	}

	dev->block = TRUE;

	uint8_t instruction[4]; // Instruction + 3 byte Address

	instruction[0] = W25Q80DV_READ_DATA;

	instruction[1] = (dev->lastAddress & 0xFF0000) >> 16;

	instruction[2] = (dev->lastAddress & 0xFF00) >> 8;

	instruction[3] = (dev->lastAddress & 0xFF);

	w25q80dv_Read(dev, instruction, 4, data, dataLenght);

	dev->block = FALSE;
}

/*************************************************************/

bool_t w25q80dv_AddressToWrite( W25Q80DV_Data_t* dev, uint8_t dataLenght, uint8_t* splitDataLenght ){

	bool_t retVal = FALSE;

	uint32_t nextAddressPageWrite = (dev->lastPage + 1)*W25Q80DV_PAGE_SIZE;

	if(nextAddressPageWrite <= W25Q80DV_LAST_PAGE_ADDRESS){

		uint32_t offsetNextPage = nextAddressPageWrite - dev->lastAddress;

		if(offsetNextPage > dataLenght){

			*splitDataLenght = 0;
		}

		else {

			*splitDataLenght = dataLenght - offsetNextPage;
		}

		retVal = TRUE;
	}

	return retVal;
}

/*==================[end of file]============================================*/
