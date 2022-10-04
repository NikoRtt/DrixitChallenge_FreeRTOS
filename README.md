# DrixitChallenge_FreeRTOS by Nicolas Rasitt

## Characteristics
The project is based on the STM32F103C8T6, known as "blue pill". It's an Arm® 32-bit Cortex®-M3 MCU with 64KB Flash and it's configured to operate at 72MHz.

## Pin Configuration

<img src="https://github.com/NikoRtt/DrixitChallenge_FreeRTOS/blob/80589b03df969299461041aa941732c8b0c51c63/PinConfiguration.JPG" alt="Pin configuration"/>

## Project organization

<img src="https://github.com/NikoRtt/DrixitChallenge_FreeRTOS/blob/ea9a8ff8d8b4b712ecd8ad319361af6322595b1b/ProjectOrganization.JPG" alt="Project Organization"/>

The present project has a folder called "MyLibs" that contains 4 folders: first for the LIS3MDL magnetometer (includes .c and .h), second for my functions, it is a group of the common functions that I used in the whole project like "printString" to send data to the usart, third the "MyInc" folder in which I put all the definitions, macros and declarations that I used to launch the whole project.

## Project operation with FreeRTOS

<img src="https://github.com/NikoRtt/DrixitChallenge_FreeRTOS/blob/80589b03df969299461041aa941732c8b0c51c63/SystemConfiguration.JPG" alt="System Organization"/>

In "main.c" all the necessary peripherals will be initialized to be able to work correctly, in our case: USART1, DMA, GPIO, CLOCK, I2C1, SPI1. Once the hardware initialization is finished, it proceeds to initialize the WINBOND W25Q80DV memory and detect if the memory already has preloaded data or if we must initialize it for the first time. This is made by reading the first 4 bytes of the first page of the memory, and if this word matches the expected initialized data then we have valid data in the memory. If we have data, then the next 2 bytes will have the last UID of the sensor data saved in the memory, and in that case, we load it into the global variable for general use. The memory is organized in the following way:

-------------------------------------- 0xFFFFFF<br>
| Space to saved data (Free space)   |<br>
-------------------------------------- 0x000006<br>
| Last Used UID (2 bytes)            |<br>
-------------------------------------- 0x000004<br>
| Memory Init (4 bytes) = 0xAAAAAAAA |<br>
-------------------------------------- 0x000000<br>

We continue to create the FreeRTOS tasks and the communication channels between them, such as the binarySemaphoreUART semaphore and the message queues called queueDataProcessing and queueUsartSender.

### Semaphores

#### BinarySemaphoreUART:

The binarySemaphoreUART is used to wake up the receiveTask and it's the interrupt of the reception of the usart1 which grants the semaphore to the task. The usart1 is configure to works with the DMA and interrupt the program when a full reception in the Rx pin is detected (reception after an idle period).

### Queues

#### QueueDataProcessing:

The queueDataProcessing is a queue that is used to send data from the sensorTask or the receiveTask to the memoryTask so that data can be saved or read in the Winbond memory. It's only used by this three tasks and it carries data of the type LIS3MDLStoreData_t.

#### queueUsartSender:

The queueUsartSender is a queue that is used to receive data in the senderTask and all the task are sending data to this task. It carries data of the type LIS3MDLStoreData_t.

### Tasks

#### Task1: sensorTask

This task will be in charge of initializing the LIS3MDL sensor, measuring the main variables every 1 second, assigning a UID to this set of variables and sending this data structure to the task in charge of recording data in memory, the memoryTask. In the case that there is a problem with the sensor, it will report inmediately to the senderTask. This task will use the LIS3MDL sensor library developed.

#### Task2: memoryTask

This task will be in charge of saving the data from the LIS3MDL sensor in the Winbond memory and reading the data to send it back to the usart1. First, it checks if the memory has initialize correctly, then it will check if it's a saving or a reading operation. For the saving operation, it must check whether there is any space to save the data or not, and when the data is saved, it will overwrite the "Last Used UID" position in memory with the recently saved data. For the reading operation, it must check 3 conditions: the requested uid is a positive number, is equal or smaller than the "Last Used UID" and the address for it is a valid one for the Winbond memory. In the case that there is a problem with the memory, it will report inmediately to the senderTask. This task will use the W25Q80DV memory library developed.

#### Task3: receiveTask

This task will be in charge of extract and decode the data received by the usart1, and send this data to the task in charge of reading data in memory, the memoryTask. This data must be a positive number. In the case that there is a problem with the data received, it will report inmediately to the senderTask. This task will use the MyFunctions library developed.

#### Task4: senderTask

This task will be in charge of sending all the strings to the usart1 by decoding the errorCodes received inside the LIS3MDLStoreData_t struct received with the queueUsartSender.
