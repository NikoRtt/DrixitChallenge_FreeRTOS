# DrixitChallenge_FreeRTOS

## Characteristics
The project is based on the STM32F103C8T6, known as "blue pill". It's an Arm® 32-bit Cortex®-M3 MCU with 64KB Flash and it's configured to operate at 72MHz.

## Pin Configuration

<img src="https://github.com/NikoRtt/DrixitChallenge_FreeRTOS/blob/ea9a8ff8d8b4b712ecd8ad319361af6322595b1b/ProjectOrganization.JPG" alt="Project Organization"/>

## Project organization

<img src="https://github.com/NikoRtt/DrixitChallenge_FreeRTOS/blob/ea9a8ff8d8b4b712ecd8ad319361af6322595b1b/ProjectOrganization.JPG" alt="Project Organization"/>

The present project has a folder call "MyLibs" that contains 4 folders: first for the magnetometer LIS3MDL(includes the .c and .h), second for my functions, it's a group of the commonly functions I used accross the project like printString to send data to the usart, third the folder myInc in which I put all the definition, macros and declaration I used throw the whole project.

## Project operation with FreeRTOS

In the main of the project, all the necessary peripherals will be initialized to be able to work correctly, in our case: USART1, DMA, GPIO, CLOCK, I2C1, SPI1. Once the hardware initialization is finished, it proceeds to initialize the WINBOND W25Q80DV memory and detect if the memory already has preloaded data or if we must initialize it for the first time. This is made by reading the first 4 bytes of the first page of the memory, and if this word matches the expected initialized data then we have valid data in the meory. If we have data, then the next 4 bytes will have the last UID of the sensor data saved in the memory, and in that case, we load it into the global variable for general use. The memory is organized in the follow way:

-------------------------------------- 0xFFFFFF
| Space to saved data (Free space)   |
-------------------------------------- 0x000006
| Last Used UID (2 bytes)            |
-------------------------------------- 0x000004
| Memory Init (4 bytes) = 0xAAAAAAAA |
-------------------------------------- 0x000000

We continue to create the FreeRTOS tasks and the communication channels between them, such as the binarySemaphoreUART semaphore and the message queues called queueDataProcessing and queueUsartSender.
Last Used UID
### Semaphore: binarySemaphoreUART

The binarySemaphoreUART is used to wake up the receiveTask and it's the interrupt of the reception of the usart1 which grants the semaphore to the task. The usart1 is configure to works with the DMA and interrupt the program when a full reception in the Rx pin is detected (reception after an idle period).

### Queues:

#### QueueDataProcessing:

The queueDataProcessing is a queue that is used to send data from the sensorTask to the memoryTask so that data can be saved in the Winbond memory. It's only used by this two task and it carries data of the type LIS3MDLStoreData_t.

#### queueUsartSender:

The queueUsartSender is a queue that is used to receive data in the senderTask and all the task are sending data to this task. It carries data of the type LIS3MDLStoreData_t.

### Task1: sensorTask

This task will be in charge of initializing the LIS3MDL sensor, measuring the main variables every 1 second, assigning a UID to this set of variables and sending this data structure to the task in charge of recording data in memory, the memoryTask. In the case that there is a problem with the sensor, it will report inmediately to the senderTask. This task will use the LIS3MDL sensor library developed.

### Task2: memoryTask

This task will be in charge of saving the data from the LIS3MDL sensor in the Winbond memory and reading the data to send it back to the usart1. First, it checks if the memory has initialize correctly, then it will check if it's a saving or a reading operation. For the saving operation, it must check whether there is any space to save the data or not, and when the data is saved, it will overwrite the "Last Used UID" position in memory with the recently saved data. For the reading operation, it must check 3 conditions: the requested uid is a positive number, is equal or smaller than the "Last Used UID" and the address for it is a valid one for the Winbond memory. In the case that there is a problem with the memory, it will report inmediately to the senderTask. This task will use the W25Q80DV memory library developed.

### Task3: receiveTask

This task will be in charge of extract and decode the data received by the usart1. This data must be a positive number. In the case that there is a problem with the data received, it will report inmediately to the senderTask. This task will use the MyFunctions library developed.

### Task4: senderTask

This task will be in charge of sending all the strings to the usart1 by decoding the errorCodes received inside the LIS3MDLStoreData_t struct received with the queueUsartSender.
