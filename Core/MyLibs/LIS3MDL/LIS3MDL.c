/*****************************************************************************
* Functions for the LIS3MDL Sensor
*
* Autor: Nicolas Rasitt
* Created: 30/09/22
*
*****************************************************************************/
/*==================[inclusions]=============================================*/

#include "LIS3MDL.h" /* <= own header */
#include <math.h>    /* <= Math functions */

/*==================[global variables]=======================================*/

/*==================[Initialization functions definition]====================*/

bool_t lis3mdl_Init ( LIS3MDL_Data_t* dev, I2C_HandleTypeDef *i2cHandle ){

	// Set the struct parameters first
	dev->i2cHandle = i2cHandle;
	dev->mag_x = 0.0f;
	dev->mag_y = 0.0f;
	dev->mag_z = 0.0f;
	dev->temp = 0.0f;

	// Variable to store the data of a register and the status of the communication
	uint8_t regValue;
	HAL_StatusTypeDef status;

	// Checking the device ID
	status = lis3mdl_ReadRegister(dev, LIS3MDL_REG_WHO_AM_I, regValue);

    if ((status != HAL_OK) || (regValue != LIS3MDL_DEVICE_ID)) {

    	return FALSE;
    }

    // Start sequence recommended
    /* A typical wakeup sequence is summarized as follows:
		1. Write 40h in CTRL_REG2. Sets full scale ±12 Hz.
		2. Write FCh in CTRL_REG1. Sets UHP mode on the X/Y axes, ODR at 80 Hz and
		   activates temperature sensor.
		3. Write 0Ch in CTRL_REG4. Sets UHP mode on the Z-axis.
		4. Write 00h in CTRL_REG3. Sets continuous-measurement mode. */

    // FS = +/-12G
	status = lis3mdl_WriteRegister(dev, LIS3MDL_REG_CTRL_2, LIS3MDL_SCALE_16_GAUSS);

    if ((status != HAL_OK) || (regValue != LIS3MDL_DEVICE_ID)) {

    	return FALSE;
    }

    // Ultra-High performance in X and Y axes, ODR at 80Hz and temperature enable.
    regValue = LIS3MDL_REG_CTL_1_TEMP_EN | (LIS3MDL_PERFORMANCE_ULTRA_HIGH << 5) | (LIS3MDL_DATA_RATE_80_HZ << 2);

	status = lis3mdl_WriteRegister(dev, LIS3MDL_REG_CTRL_1, regValue);

    if (status != HAL_OK) {

    	return FALSE;
    }

    // Ultra-High performance in Z axe.
    regValue = 0x00 | (LIS3MDL_PERFORMANCE_ULTRA_HIGH << 2);

	status = lis3mdl_WriteRegister(dev, LIS3MDL_REG_CTRL_4, regValue);

    if (status != HAL_OK) {

    	return FALSE;
    }

    // Sets continuous-measurement mode.
    regValue = 0x00 | LIS3MDL_MODE_CONTINUOUS_MEASUREMENT;

	status = lis3mdl_WriteRegister(dev, LIS3MDL_REG_CTRL_3, regValue);

    if (status != HAL_OK) {

    	return FALSE;
    }

    return TRUE;
}

/*==================[Measurement functions definition]=======================*/

HAL_StatusTypeDef lis3mdl_ReadTemperature ( LIS3MDL_Data_t* dev ){


}

HAL_StatusTypeDef lis3mdl_ReadMagnetometer ( LIS3MDL_Data_t* dev ){


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
