/*****************************************************************************
* Functions for the W25Q80DV Flash Memory
*
* Autor: Nicolas Rasitt
* Created: 01/10/22
*
*****************************************************************************/

#ifndef _W25Q80DV_H_
#define _W25Q80DV_H_

/*==================[inclusions]=============================================*/

#include "stm32f1xx_hal.h"

#include "../MyInc/Datatypes.h"

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/

/*==================[definitions]============================================*/

#define W25Q80DV_WRITE_ENABLE 		0x06	// sets WEL bit, must be set before any write/program/erase
#define W25Q80DV_ENABLE_VOLATILE_SR 0x50	// volatile SR Write Enable
#define W25Q80DV_WRITE_DISABLE 		0x04	// resets WEL bit (state after power-up)
#define W25Q80DV_READ_SR1 			0x05	// read status-register 1
#define W25Q80DV_READ_SR2 			0x35	// read status-register 2
#define W25Q80DV_WRITE_SR 			0x01	// write status-register 1 (8.2.5)
#define W25Q80DV_PAGE_PROGRAM 		0x02	// program page (256bytes) by single SPI line
#define W25Q80DV_SECTOR_ERASE 		0x20	// sets all 4Kbyte sector with 0xFF
#define W25Q80DV_32KB_BLOCK_ERASE 	0x52	// sets all 32Kbyte block with 0xFF
#define W25Q80DV_64KB_BLOCK_ERASE 	0xD8	// sets all 64Kbyte block with 0xFF
#define W25Q80DV_CHIP_ERASE 		0xC7	// fill all the chip with 0xFF
#define W25Q80DV_ERASEPROG_SUSPEND 	0x75	// suspend erase/program operation
#define W25Q80DV_ERASEPROG_RESUME 	0x7A	// resume erase/program operation
#define W25Q80DV_POWERDOWN 			0xB9	// powers down the chip (power-up by reading ID)
#define W25Q80DV_READ_DATA 			0x03	// read data by standard SPI
#define W25Q80DV_FAST_READ 			0x0B	// highest FR speed
#define W25Q80DV_RELEASE_PD			0xAB	// release power down
#define W25Q80DV_DEVID 				0xAB	// read Device ID
#define W25Q80DV_READ_JEDEC_ID 		0x9F	// read JEDEC-standard ID
#define W25Q80DV_READ_UID 			0x4B	// read unique chip 64-bit ID
#define W25Q80DV_READ_SFDP 			0x5A	// read SFDP register parameters
#define W25Q80DV_ERASE_SECURITY_REG 0x44	// erase security registers
#define W25Q80DV_PROG_SECURITY_REG 	0x42	// program security registers
#define W25Q80DV_READ_SECURITY_REG 	0x48	// read security registers
#define W25Q80DV_ENABLE_RST 		0x66	// enable software-reset ability
#define W25Q80DV_RESET 				0x99	// make software reset

#define W25Q80DV_ID				0x13
#define W25Q80DV_DUMMY_BYTE 		0xAA
#define W25Q80DV_STATUSREGISTER_1	0x01
#define W25Q80DV_STATUSREGISTER_2	0x02
#define W25Q80DV_WEL_BIT			0x01
#define W25Q80DV_WEL_ENABLE		0x01

#define W25Q80DV_ERROR_BYTE 		0xFF

#define W25Q80DV_PAGE_SIZE 		256
#define W25Q80DV_SECTOR_SIZE 		4096
#define W25Q80DV_SECTOR_COUNT		256
#define W25Q80DV_PAGE_COUNT		4096
#define W25Q80DV_BLOCK_SIZE		65536
#define W25Q80DV_BLOCK_COUNT		16
#define W25Q80DV_KILOBYTE_SIZE		1024

#define W25Q80DV_FIRST_PAGE_ADDRESS	0x000000
#define W25Q80DV_LAST_PAGE_ADDRESS	0x0FFFF0
#define W25Q80DV_NO_PAGE			0xFFFF

/*==================[typedef]================================================*/

typedef struct {
	// SPI Handle
	SPI_HandleTypeDef* W25Q80DV_SPI;
	// Pin Configuration
	uint16_t csPin;
	GPIO_TypeDef* csPort;
	// Block operation
	bool_t block;
	// Address operation
	uint32_t lastAddress;
	uint16_t lastPage;
} W25Q80DV_Data_t;

/*==================[global variables]=======================================*/

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

//Initialization functions definition

uint8_t w25q80dv_Init( W25Q80DV_Data_t* dev, SPI_HandleTypeDef* spi, uint16_t pin, GPIO_TypeDef* port );

bool_t w25q80dv_InitAddress( W25Q80DV_Data_t* dev );

//Basic functions definition

void w25q80dv_EraseChip( W25Q80DV_Data_t* dev );

void w25q80dv_WriteBytes( W25Q80DV_Data_t* dev, uint8_t* data, uint8_t dataLenght );

void w25q80dv_ReadBytes( W25Q80DV_Data_t* dev, uint8_t* data, uint8_t dataLenght );

bool_t w25q80dv_AddressToWrite( W25Q80DV_Data_t* dev, uint8_t dataLenght, uint8_t* splitDataLenght );

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
}
#endif

/*==================[end of file]============================================*/

#endif /* #ifndef _W25Q80DV_H_ */
