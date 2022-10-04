# DrixitChallenge_FreeRTOS

## Characteristics
The project is based on the STM32F103C8T6, known as "blue pill". It's an Arm® 32-bit Cortex®-M3 MCU with 64KB Flash and it's configured to operate at 72MHz.


## Project organization

<img src="https://github.com/NikoRtt/DrixitChallenge_FreeRTOS/ProjectOrganization.JPG" alt="Project Organization"/>

The present project has a folder call "MyLibs" that contains 4 folders: first for the magnetometer LIS3MDL(includes the .c and .h), second for my functions, it's a group of the commonly functions I used accross the project like printString to send data to the usart, third the folder myInc in which I put all the definition, macros and declaration I used throw the whole project.