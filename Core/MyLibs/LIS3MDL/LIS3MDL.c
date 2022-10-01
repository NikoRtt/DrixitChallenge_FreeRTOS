/*****************************************************************************
* Functions for the LIS3MDL Sensor
*
* Autor: Nicolas Rasitt
* Created: 30/09/22
*
*****************************************************************************/
/*==================[inclusions]=============================================*/

#include "LIS3MDL.h"
#include "../MyInc/Headers.h"

/*==================[global variables]=======================================*/

/*==================[Initialization functions definition]====================*/

uint8_t lis3mdl_Init ( LIS3MDL_Data_t* dev, I2C_HandleTypeDef *i2cHandle ){

	// Set the struct parameters first

	dev->i2cHandle = i2cHandle;
	dev->mag_x = 0.0f;
	dev->mag_y = 0.0f;
	dev->mag_z = 0.0f;
	dev->temp = 0.0f;

	// Variable to accumulate errors to determine how many transaction errors we have.

	uint8_t accumulatedErrors = 0;

	// Variable to store the data of a register and the status of the communication

	uint8_t regValue;
	HAL_StatusTypeDef status;

	// Checking the device ID

	status = lis3mdl_ReadRegister(dev, LIS3MDL_REG_WHO_AM_I, &regValue);

	accumulatedErrors += (status != HAL_OK);

    if (regValue != LIS3MDL_DEVICE_ID) {

    	return FAIL;
    }

    // Start sequence recommended
    /* A typical wakeup sequence is summarized as follows:
		1. Write 40h in CTRL_REG2. Sets full scale ±12 Hz.
		2. Write FCh in CTRL_REG1. Sets UHP mode on the X/Y axes, ODR at 80 Hz and
		   activates temperature sensor.
		3. Write 0Ch in CTRL_REG4. Sets UHP mode on the Z-axis.
		4. Write 00h in CTRL_REG3. Sets continuous-measurement mode. */

    // FS = +/-12G

    regValue = dev->scale << 5;

	status = lis3mdl_WriteRegister(dev, LIS3MDL_REG_CTRL_2, &regValue);

	accumulatedErrors += (status != HAL_OK);

    // Ultra-High performance in X and Y axes, ODR at 80Hz and temperature enable.

    regValue = LIS3MDL_REG_CTL_1_TEMP_EN | (LIS3MDL_PERFORMANCE_ULTRA_HIGH << 5) | (LIS3MDL_DATA_RATE_80_HZ << 2);

	status = lis3mdl_WriteRegister(dev, LIS3MDL_REG_CTRL_1, &regValue);

	accumulatedErrors += (status != HAL_OK);

    // Ultra-High performance in Z axe.

    regValue = 0x00 | (LIS3MDL_PERFORMANCE_ULTRA_HIGH << 2);

	status = lis3mdl_WriteRegister(dev, LIS3MDL_REG_CTRL_4, &regValue);

	accumulatedErrors += (status != HAL_OK);

    // Sets continuous-measurement mode.

    regValue = 0x00 | LIS3MDL_MODE_CONTINUOUS_MEASUREMENT;

	status = lis3mdl_WriteRegister(dev, LIS3MDL_REG_CTRL_3, &regValue);

	accumulatedErrors += (status != HAL_OK);

    return accumulatedErrors;
}

/*==================[Measurement functions definition]=======================*/

HAL_StatusTypeDef lis3mdl_DataReady ( LIS3MDL_Data_t* dev ){

	// Variable to store the data of a register and the status of the communication

	uint8_t regValue;
	HAL_StatusTypeDef status;

	// First we have to know if there is any data ready to read

	status = lis3mdl_ReadRegister(dev, LIS3MDL_REG_STATUS, &regValue);

	if(status == HAL_OK){

		if(!(((regValue & LIS3MDL_STATUS_ZYXDA) >> 3) && ((status & LIS3MDL_STATUS_ZYXOR) >> 7))){

			status = HAL_ERROR;
		}
	}

	return status;
}

/*************************************************************/

HAL_StatusTypeDef lis3mdl_ReadMagnetometer ( LIS3MDL_Data_t* dev ){

	// Variable to store the data of a register and the status of the communication

	uint8_t regValue[6];
	HAL_StatusTypeDef status;

	// Reading the axes registers

	status = lis3mdl_ReadRegisters(dev, LIS3MDL_REG_OUT_TEMP_L, regValue, 6);

	if(status == HAL_OK){

		float sensitivity;

		switch(dev->scale){

			case LIS3MDL_SCALE_4_GAUSS:

				sensitivity = 1.0/6842;

				break;

			case LIS3MDL_SCALE_8_GAUSS:

				sensitivity = 1.0/3421;

				break;

			case LIS3MDL_SCALE_12_GAUSS:

				sensitivity = 1.0/2281;

				break;

			case LIS3MDL_SCALE_16_GAUSS:

				sensitivity = 1.0/1711;

				break;
		}

		dev->mag_x = ((int16_t)((regValue[1] << 8) | regValue[0])) * sensitivity;

		dev->mag_y = ((int16_t)((regValue[3] << 8) | regValue[2])) * sensitivity;

		dev->mag_z = ((int16_t)((regValue[5] << 8) | regValue[4])) * sensitivity;
	}

	return status;
}

/*************************************************************/

HAL_StatusTypeDef lis3mdl_ReadTemperature ( LIS3MDL_Data_t* dev ){

	// Variable to store the data of a register and the status of the communication

	uint8_t regValue[2];
	HAL_StatusTypeDef status;

	// Reading the temperature registers

	status = lis3mdl_ReadRegisters(dev, LIS3MDL_REG_OUT_TEMP_L, regValue, 2);

	if(status == HAL_OK){

		// From the datasheet, the nominal sensitivity is 8 LSB/°C (>>3) and 0 output means T=25 °C

		dev->temp = (((int16_t)((regValue[1] << 8) | regValue[0])) >> 3) + 25.0;

	}

	return status;
}

/*==================[low level functions definition]=========================*/

HAL_StatusTypeDef lis3mdl_ReadRegister( LIS3MDL_Data_t* dev, uint8_t reg, uint8_t* value ){

	return HAL_I2C_Mem_Read(dev->i2cHandle, LIS3MDL_ADDRESS1, reg, I2C_MEMADD_SIZE_8BIT, value, 1, HAL_MAX_DELAY);
}

/*************************************************************/

HAL_StatusTypeDef lis3mdl_WriteRegister( LIS3MDL_Data_t* dev, uint8_t reg, uint8_t* value ){

	return HAL_I2C_Mem_Write(dev->i2cHandle, LIS3MDL_ADDRESS1, reg, I2C_MEMADD_SIZE_8BIT, value, 1, HAL_MAX_DELAY);

}

/*************************************************************/

HAL_StatusTypeDef lis3mdl_ReadRegisters( LIS3MDL_Data_t* dev, uint8_t reg, uint8_t* value, uint8_t lenght ){

	return HAL_I2C_Mem_Read(dev->i2cHandle, LIS3MDL_ADDRESS1, reg, I2C_MEMADD_SIZE_8BIT, value, lenght, HAL_MAX_DELAY);

}


/*==================[end of file]============================================*/
