/*****************************************************************************
* Functions for the LIS3MDL Sensor
*
* Autor: Nicolas Rasitt
* Created: 30/09/22
*
*****************************************************************************/

#ifndef _LIS3MDL_H_
#define _LIS3MDL_H_

/*==================[inclusions]=============================================*/

#include "stm32f1xx_hal.h"

#include "../MyInc/Datatypes.h"

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/

/*==================[definitions]============================================*/

// We will start working with the assumption that the CS pin is connected to
// VCC, so the interface will be I2C. Also we assume that the SDO/SA1 pin is
// connected to VCC also.

#define LIS3MDL_ADDRESS1  0b00111100   // SDO/SA1 = 1(voltage supply)
//#define LIS3MDL_ADDRESS2  0b00111000   // SDO/SA1 = 0(ground)

#define LIS3MDL_REG_WHO_AM_I       0x0F
#define LIS3MDL_REG_CTRL_1         0x20
#define LIS3MDL_REG_CTRL_2         0x21
#define LIS3MDL_REG_CTRL_3         0x22
#define LIS3MDL_REG_CTRL_4         0x23
#define LIS3MDL_REG_CTRL_5         0x24
#define LIS3MDL_REG_STATUS         0x27
#define LIS3MDL_REG_OUT_X_L        0x28
#define LIS3MDL_REG_OUT_X_H        0x29
#define LIS3MDL_REG_OUT_Y_L        0x2A
#define LIS3MDL_REG_OUT_Y_H        0x2B
#define LIS3MDL_REG_OUT_Z_L        0x2C
#define LIS3MDL_REG_OUT_Z_H        0x2D
#define LIS3MDL_REG_OUT_TEMP_L     0x2E
#define LIS3MDL_REG_OUT_TEMP_H     0x2F
#define LIS3MDL_REG_INT_CFG		   0x30
#define LIS3MDL_REG_INT_SRC		   0x31
#define LIS3MDL_REG_INT_THS_L	   0x32
#define LIS3MDL_REG_INT_THS_H	   0x33

#define LIS3MDL_REG_CTL_1_TEMP     0b10000000
#define LIS3MDL_REG_CTL_1_TEMP_EN  0b10000000
#define LIS3MDL_REG_CTL_1_TEMP_DI  0b00000000

#define LIS3MDL_SCALE_4_GAUSS      0b00
#define LIS3MDL_SCALE_8_GAUSS      0b01
#define LIS3MDL_SCALE_12_GAUSS     0b10
#define LIS3MDL_SCALE_16_GAUSS     0b11

#define LIS3MDL_PERFORMANCE_LOW_POWER  	0b00
#define LIS3MDL_PERFORMANCE_MEDIUM     	0b01
#define LIS3MDL_PERFORMANCE_HIGH       	0b10
#define LIS3MDL_PERFORMANCE_ULTRA_HIGH	0b11

#define LIS3MDL_DATA_RATE_0_625_HZ 0b000
#define LIS3MDL_DATA_RATE_1_25_HZ  0b001
#define LIS3MDL_DATA_RATE_2_5_HZ   0b010
#define LIS3MDL_DATA_RATE_5_HZ     0b011
#define LIS3MDL_DATA_RATE_10_HZ    0b100
#define LIS3MDL_DATA_RATE_20_HZ    0b101
#define LIS3MDL_DATA_RATE_40_HZ    0b110
#define LIS3MDL_DATA_RATE_80_HZ    0b111

#define LIS3MDL_MODE_CONTINUOUS_MEASUREMENT		0b00
#define LIS3MDL_MODE_SINGLE_MEASUREMENT			0b01
#define LIS3MDL_MODE_IDLE						0b11

#define LIS3MDL_STATUS_ZYXOR       0b100000000
#define LIS3MDL_STATUS_ZOR         0b01000000
#define LIS3MDL_STATUS_YOR         0b00100000
#define LIS3MDL_STATUS_XOR         0b00010000
#define LIS3MDL_STATUS_ZYXDA       0b00001000
#define LIS3MDL_STATUS_ZDA         0b00000100
#define LIS3MDL_STATUS_YDA         0b00000010
#define LIS3MDL_STATUS_XDA         0b00000001

#define LIS3MDL_DEVICE_ID          0b00111101

/*==================[typedef]================================================*/

typedef struct {
	// I2C Handle
	I2C_HandleTypeDef* i2cHandle;
	// Unique ID
    uint16_t uid;
    // Magnetometer data from axis X
	float mag_x;
	// Magnetometer data from axis Y
	float mag_y;
	// Magnetometer data from axis Z
	float mag_z;
	// Temperature data in degrees Celsius
	float temp;
	// Scale
	uint8_t scale;
} LIS3MDL_Data_t;

typedef struct {
	// Unique ID
    uint16_t uid;
    // Magnetometer data from axis X
	float mag_x;
	// Magnetometer data from axis Y
	float mag_y;
	// Magnetometer data from axis Z
	float mag_z;
	// Temperature data in degrees Celsius
	float temp;
	// If we must read or save the data
	errorCodes_t statusData;
} LIS3MDL_StoreData_t;

/*==================[global variables]=======================================*/

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

// Initialization
uint8_t lis3mdl_Init ( LIS3MDL_Data_t* dev, I2C_HandleTypeDef *i2cHandle );
// Measurement
HAL_StatusTypeDef lis3mdl_ReadTemperature ( LIS3MDL_Data_t* dev );
HAL_StatusTypeDef lis3mdl_ReadMagnetometer ( LIS3MDL_Data_t* dev );
HAL_StatusTypeDef lis3mdl_DataReady ( LIS3MDL_Data_t* dev );
LIS3MDL_StoreData_t lis3mdl_ConvertToStoreData ( LIS3MDL_Data_t* dev );

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
}
#endif

/*==================[end of file]============================================*/

#endif /* #ifndef _LIS3MDL_H_ */
