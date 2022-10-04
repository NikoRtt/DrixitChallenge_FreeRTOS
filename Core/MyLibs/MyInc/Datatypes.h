/*****************************************************************************
* Data Type Declarations
*
* Autor: Nicolas Rasitt
* Date: 30/09/22
*
*****************************************************************************/

#ifndef DATATYPES_H
#define DATATYPES_H

/*==================[inclusions]=============================================*/

#include "stdint.h"

/*==================[macros]=================================================*/

// Functional states
#ifndef ON
   #define ON     1
#endif
#ifndef OFF
   #define OFF    0
#endif

// Electrical states
#ifndef HIGH
   #define HIGH   1
#endif
#ifndef LOW
   #define LOW    0
#endif

// Logical states
#ifndef FALSE
   #define FALSE  0
#endif
#ifndef TRUE
   #define TRUE   (!FALSE)
#endif

/*==================[definitions]============================================*/

typedef enum{
	UNKNOWN_ERROR,
	LIS3MDL_ERROR,
	MEMORY_ERROR,
	READ_DATA,
	DATA_SAVE,
	SAVE_DATA,
	DATA_READ,
	MEMORY_FULL,
	WRONG_ID,
	NO_ERROR
}errorCodes_t;

/*==================[typedef]================================================*/

/* Define Boolean Data Type */
typedef uint8_t bool_t;

/*==================[global variables]=======================================*/

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

#endif

/*==================[end of file]============================================*/
