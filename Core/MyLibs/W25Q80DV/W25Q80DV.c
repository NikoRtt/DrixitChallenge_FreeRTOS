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

// low level functions declaration

HAL_StatusTypeDef w25q80dv_SPIRead( W25Q80DV_Data_t* dev, uint8_t* data, uint8_t dataLenght, uint8_t* value, uint8_t valueLenght );

HAL_StatusTypeDef w25q80dv_SPIWrite( W25Q80DV_Data_t* dev, uint8_t* data, uint8_t dataLenght );

// Instruction functions declaration

bool_t w25q80dv_InstructionReadID( W25Q80DV_Data_t* dev );

HAL_StatusTypeDef w25q80dv_InstructionWriteEnable( W25Q80DV_Data_t* dev );

HAL_StatusTypeDef w25q80dv_InstructionWriteDisable( W25Q80DV_Data_t* dev );

uint8_t w25q80dv_InstructionReadStatusRegister( W25Q80DV_Data_t* dev, uint8_t selectRegister);

void w25q80dv_InstructionWaitForWriteEnd( W25Q80DV_Data_t* dev );

// Basics functions declaration

void w25q80dv_WriteBytesInPage( W25Q80DV_Data_t* dev, uint8_t* data, uint8_t dataLenght );

/*==================[internal functions definition]==========================*/

/*==================[low level functions definition]=========================*/

HAL_StatusTypeDef w25q80dv_SPIRead( W25Q80DV_Data_t* dev, uint8_t* data, uint8_t dataLenght, uint8_t* value, uint8_t valueLenght ){

	uint8_t retVal;

	HAL_GPIO_WritePin(dev->csPort, dev->csPin, GPIO_PIN_RESET);

	HAL_SPI_Transmit(dev->W25Q80DV_SPI, data, dataLenght, HAL_MAX_DELAY);

	retVal = HAL_SPI_Receive(dev->W25Q80DV_SPI, value, valueLenght, HAL_MAX_DELAY);

	HAL_GPIO_WritePin(dev->csPort, dev->csPin, GPIO_PIN_SET);

	return retVal;
}

/*************************************************************/

HAL_StatusTypeDef w25q80dv_SPIWrite( W25Q80DV_Data_t* dev, uint8_t* data, uint8_t dataLenght ){

	uint8_t retVal;

	HAL_GPIO_WritePin(dev->csPort, dev->csPin, GPIO_PIN_RESET);

	retVal = HAL_SPI_Transmit(dev->W25Q80DV_SPI, data, dataLenght, HAL_MAX_DELAY);

	HAL_GPIO_WritePin(dev->csPort, dev->csPin, GPIO_PIN_SET);

	return retVal;
}

/*==================[Instruction functions definition]=======================*/

/**
 * @brief Instruction to read the ID of the memory. This ID is mention in the
 *  datasheet of the memory and is used to check where we are working with the
 *  correct memory or not.
 * 
 * @param dev Structure to receive information.
 * @return bool_t TRUE if it's the W25Q80DV Winbond memory.
 */
bool_t w25q80dv_InstructionReadID( W25Q80DV_Data_t* dev ){

	uint8_t instruction[] = {W25Q80DV_DEVID, W25Q80DV_DUMMY_BYTE, W25Q80DV_DUMMY_BYTE, W25Q80DV_DUMMY_BYTE};

	uint8_t devID = W25Q80DV_ERROR_BYTE;

	bool_t retVal = FALSE;

	if(w25q80dv_SPIRead(dev, instruction, 4, &devID, 1) == HAL_OK){

		if(devID == W25Q80DV_ID){

			retVal = TRUE;
		}
	}

	return retVal;
}

/*************************************************************/

/**
 * @brief The Write Enable instruction sets the Write Enable 
 * Latch (WEL) bit in the Status Register to a 1. This must be 
 * done every time before a write or erase instruction.
 * 
 * @param dev Structure to receive information.
 * @return HAL_StatusTypeDef 
 */
HAL_StatusTypeDef w25q80dv_InstructionWriteEnable( W25Q80DV_Data_t* dev ){

	uint8_t instruction = W25Q80DV_WRITE_ENABLE;

	return w25q80dv_SPIWrite(dev, &instruction, 1);
}

/*************************************************************/

/**
 * @brief The Write Enable instruction resets the Write Enable 
 * Latch (WEL) bit in the Status Register to a 0.
 * 
 * @param dev Structure to receive information.
 * @return HAL_StatusTypeDef 
 */
HAL_StatusTypeDef w25q80dv_InstructionWriteDisable( W25Q80DV_Data_t* dev ){

	uint8_t instruction = W25Q80DV_WRITE_DISABLE;

	return w25q80dv_SPIWrite(dev, &instruction, 1);
}

/*************************************************************/

/**
 * @brief The Read Status Register instructions allow the 8-bit
 *  Status Registers to be read.
 * 
 * @param dev Structure to receive information.
 * @param selectRegister The register requiered, it can be 1 or
 * 2 for this memory.
 * @return uint8_t 
 */
uint8_t w25q80dv_InstructionReadStatusRegister( W25Q80DV_Data_t* dev, uint8_t selectRegister){

	uint8_t instruction;

	uint8_t statusRegister = W25Q80DV_ERROR_BYTE;

	if (selectRegister == W25Q80DV_STATUSREGISTER_1 ){

		instruction = W25Q80DV_READ_SR1;

		w25q80dv_SPIRead(dev, &instruction, 1, &statusRegister, 1);
	}

	else if (selectRegister == 2){

		instruction = W25Q80DV_READ_SR2;

		w25q80dv_SPIRead(dev, &instruction, 1, &statusRegister, 1);
	}

	return statusRegister;
}

/*************************************************************/

/**
 * @brief Reads the status register to know when the write 
 * finish. It's block function.
 * 
 * @param dev Structure to receive information.
 */
void w25q80dv_InstructionWaitForWriteEnd( W25Q80DV_Data_t* dev ){

	uint8_t statusRegister1;

	do{
		statusRegister1 = w25q80dv_InstructionReadStatusRegister(dev, W25Q80DV_READ_SR1);

		HAL_Delay(1);

	} while ((statusRegister1 & W25Q80DV_WEL_BIT) == W25Q80DV_WEL_ENABLE);
}

/*==================[Initialization functions definition]====================*/

/**
 * @brief Initialize the memory the a correct operation.
 * 
 * @param dev Structure to assign information.
 * @param spi The handler for the communication.
 * @param pin The pin of the /CS (Chip Select).
 * @param port The port of the /CS (Chip Select).
 * @return bool_t TRUE is everything is fine.
 */
bool_t w25q80dv_Init( W25Q80DV_Data_t* dev, SPI_HandleTypeDef* spi, uint16_t pin, GPIO_TypeDef* port ){

	dev->W25Q80DV_SPI = spi;

	dev->csPort = port;

	dev->csPin = pin;

	dev->lastPage = 0;

	dev->lastAddress = W25Q80DV_FIRST_PAGE_ADDRESS;

	dev->statusMemInit = FALSE;

	// We make sure that the memory start properly

	uint8_t instruction = W25Q80DV_RELEASE_PD;

	w25q80dv_SPIWrite(dev, &instruction, 1);

	// Check the flash memory

	if(w25q80dv_InstructionReadID(dev)){

		dev->statusMemInit = TRUE;
	}

	return dev->statusMemInit;
}

/*************************************************************/

/**
 * @brief This function is used to find out is the memory has 
 * information previously saved or not. This is done by reading
 * the first 4 bytes of the memory and comparing them with the
 * initialization word. If there's no match, a chip erase 
 * operation must be done to the correct functioning of the 
 * memory.
 * 
 * @param dev Structure to receive information.
 * @return bool_t TRUE is the memroy has been initialize.
 */
bool_t w25q80dv_isMemInit( W25Q80DV_Data_t* dev ){

	uint8_t data[W25Q80DV_INITIALIZE_SIZE];

	w25q80dv_ReadBytesInAddress(dev, W25Q80DV_FIRST_PAGE_ADDRESS, data, W25Q80DV_INITIALIZE_SIZE);

	// Check the data of initialization in memory

	bool_t retVal = FALSE;

	uint32_t resultValue = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];

	if(resultValue == W25Q80DV_INITIALIZE_MEM){

		retVal = TRUE;
	}

	return retVal;
}

/*==================[Basic functions definition]============================*/

/**
 * @brief A chip erase operation that cleans all the memory.
 * 
 * @param dev Structure to receive information.
 */
void w25q80dv_EraseChip( W25Q80DV_Data_t* dev ){

	w25q80dv_InstructionWaitForWriteEnd(dev);

	w25q80dv_InstructionWriteEnable(dev);

	uint8_t instruction = W25Q80DV_CHIP_ERASE;

	w25q80dv_SPIWrite(dev, &instruction, 1);

	w25q80dv_InstructionWaitForWriteEnd(dev);
}

/*************************************************************/

/**
 * @brief This function is used to write bytes in a single page
 * of the memory(max. 256 bytes), starting at the address 
 * received in the structure. Then it waits for a maximum 
 * period of time that the datasheet ensures that the memory
 * will be written. The next address to write is automatically 
 * loaded in the structure.
 * 
 * @param dev Structure to receive information.
 * @param data The information to write in the memory.
 * @param dataLenght Amount of bytes to be written.
 */
void w25q80dv_WriteBytesInPage( W25Q80DV_Data_t* dev, uint8_t* data, uint8_t dataLenght ){

	w25q80dv_InstructionWaitForWriteEnd(dev);

	w25q80dv_InstructionWriteEnable(dev);

	uint8_t instruction[4 + W25Q80DV_PAGE_SIZE]; // Instruction + 3 byte Address + Data

	instruction[0] = W25Q80DV_PAGE_PROGRAM;

	instruction[1] = (dev->lastAddress & 0xFF0000) >> 16;

	instruction[2] = (dev->lastAddress & 0xFF00) >> 8;

	instruction[3] = (dev->lastAddress & 0xFF);

	memcpy((instruction + 4), data, dataLenght);

	w25q80dv_SPIWrite(dev, instruction, 4 + dataLenght);

	w25q80dv_InstructionWaitForWriteEnd(dev);

	// Maximum time to write a page
	HAL_Delay(3);

	dev->lastAddress = dev->lastAddress + dataLenght;
}

/*************************************************************/

/**
 * @brief This function is used to write bytes in a continuas 
 * way using as most two pages, starting at the address 
 * received in the structure. This is posible checking the 
 * starting address, the lenght of the data and where the write 
 * process will end to know if the write operation should be 
 * split or not. This must be done this way because two pages 
 * cannot be written at the same time, if the amount of data is 
 * longer than a page, that page will be overwritten. The next 
 * address to write is automatically loaded in the structure.
 * 
 * @param dev Structure to receive information.
 * @param data The information to write in the memory.
 * @param dataLenght Amount of bytes to be written.
 * @return bool_t FALSE memory full.
 */
bool_t w25q80dv_WriteBytesInSequence( W25Q80DV_Data_t* dev, uint8_t* data, uint8_t dataLenght ){

	bool_t retVal = FALSE;

	uint8_t splitData;

	if(w25q80dv_AddressToWrite(dev, dataLenght, splitData)){

		retVal = TRUE;

		if(splitData == 0){

			w25q80dv_WriteBytesInPage(dev, data, dataLenght);
		}

		else {

			dev->lastPage++;

			w25q80dv_WriteBytesInPage(dev, data, (dataLenght - splitData));

			w25q80dv_WriteBytesInPage(dev, data[dataLenght - splitData], splitData);
		}

		w25q80dv_InstructionWaitForWriteEnd(dev);
	}

	return retVal;
}

/*************************************************************/

/**
 * @brief This function is used to write bytes in a single page
 * of the memory(max. 256 bytes), starting at the address 
 * received in the address parameter. This function has to be 
 * used carefully so as not to try to write two pages at the 
 * same time. (User caution!)
 * 
 * @param dev Structure to receive information.
 * @param address Starting address to write.
 * @param data The information to write in the memory.
 * @param dataLenght Amount of bytes to be written.
 */
void w25q80dv_WriteBytesInAddress( W25Q80DV_Data_t* dev, uint32_t address, uint8_t* data, uint8_t dataLenght ){

	w25q80dv_InstructionWaitForWriteEnd(dev);

	w25q80dv_InstructionWriteEnable(dev);

	uint8_t instruction[4 + W25Q80DV_PAGE_SIZE]; // Instruction + 3 byte Address + Data

	instruction[0] = W25Q80DV_PAGE_PROGRAM;

	instruction[1] = (address & 0xFF0000) >> 16;

	instruction[2] = (address & 0xFF00) >> 8;

	instruction[3] = (address & 0xFF);

	memcpy((instruction + 4), data, dataLenght);

	w25q80dv_SPIWrite(dev, instruction, 4 + dataLenght);

	w25q80dv_InstructionWaitForWriteEnd(dev);
}

/*************************************************************/

/**
 * @brief This function is used to read bytes of the memory, 
 * starting at the address received in the structure.
 * 
 * @param dev Structure to receive information.
 * @param data The information read in the memory.
 * @param dataLenght Amount of bytes to be read.
 */
void w25q80dv_ReadBytesInSequence( W25Q80DV_Data_t* dev, uint8_t* data, uint8_t dataLenght ){

	uint8_t instruction[4]; // Instruction + 3 byte Address

	instruction[0] = W25Q80DV_READ_DATA;

	instruction[1] = (dev->lastAddress & 0xFF0000) >> 16;

	instruction[2] = (dev->lastAddress & 0xFF00) >> 8;

	instruction[3] = (dev->lastAddress & 0xFF);

	w25q80dv_SPIRead(dev, instruction, 4, data, dataLenght);
}

/*************************************************************/

/**
 * @brief This function is used to read bytes in a specific 
 * address of the memory.
 * 
 * @param dev Structure to receive information.
 * @param address Starting address to read.
 * @param data The information read in the memory.
 * @param dataLenght Amount of bytes to be read.
 */
void w25q80dv_ReadBytesInAddress( W25Q80DV_Data_t* dev, uint32_t address, uint8_t* data, uint8_t dataLenght ){

	uint8_t instruction[4]; // Instruction + 3 byte Address

	instruction[0] = W25Q80DV_READ_DATA;

	instruction[1] = (address & 0xFF0000) >> 16;

	instruction[2] = (address & 0xFF00) >> 8;

	instruction[3] = (address & 0xFF);

	w25q80dv_SPIRead(dev, instruction, 4, data, dataLenght);
}

/*************************************************************/

/**
 * @brief Function used to know if a page or more than one page
 * is going to be written based on the length of the data and 
 * the starting address received in the structure.
 * 
 * @param dev Structure to receive information.
 * @param dataLenght Amount of bytes to be written.
 * @param splitDataLenght Sero if the write operation will be 
 * in a single page, or an amount of bytes that should be 
 * written in the previously page.
 * @return bool_t FALSE memory full.
 */
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
