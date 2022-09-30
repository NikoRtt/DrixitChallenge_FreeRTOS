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

// ----------------------------------------------------------------------------
// Macros utilities
// ----------------------------------------------------------------------------

#define BYTES_16_HIGH(val)   ( (uint8_t)((val&(0xFF00))>>8) )
#define BYTES_16_LOW(val)    ( (uint8_t)(val&(0x00FF)) )

#define BYTES_32_HIGH(val)   ( (uint8_t)((val&(0xFF000000))>>24) )
#define BYTES_32_MIDH(val)   ( (uint8_t)((val&(0x00FF0000))>>16) )
#define BYTES_32_MIDL(val)   ( (uint8_t)((val&(0x0000FF00))>>8) )
#define BYTES_32_LOW(val)    ( (uint8_t)( val&(0x000000FF))     )

#define BYTES_64_HIGH(val)   ( (uint8_t)((val&(0xFF00000000000000)) >> 56) )
#define BYTES_64_MIDHH(val)  ( (uint8_t)((val&(0x00FF000000000000)) >> 48) )
#define BYTES_64_MIDHL(val)  ( (uint8_t)((val&(0x0000FF0000000000)) >> 40) )
#define BYTES_64_MIDMH(val)  ( (uint8_t)((val&(0x000000FF00000000)) >> 32) )
#define BYTES_64_MIDML(val)  ( (uint8_t)((val&(0x00000000FF000000)) >> 24) )
#define BYTES_64_MIDLH(val)  ( (uint8_t)((val&(0x0000000000FF0000)) >> 16) )
#define BYTES_64_MIDLL(val)  ( (uint8_t)((val&(0x000000000000FF00)) >>  8) )
#define BYTES_64_LOW(val)    ( (uint8_t)( val&(0x00000000000000FF))        )

/*==================[definitions]============================================*/

typedef enum{
	UNKNOWN_ERR,
	LIS3MDL_NOT_FOUND,
	MEMORY_ERR
}errorCodes_t;

/*==================[typedef]================================================*/

/* Define Boolean Data Type */
typedef uint8_t bool_t;

typedef struct {
    uint16_t id;
	uint16_t X;
	uint16_t Y;
	uint16_t Z;
	uint16_t Temp;
} LogData_t;

/*==================[global variables]=======================================*/

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

#endif

/*==================[end of file]============================================*/
