################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/MyLibs/W25Q80DV/W25Q80DV.c 

OBJS += \
./Core/MyLibs/W25Q80DV/W25Q80DV.o 

C_DEPS += \
./Core/MyLibs/W25Q80DV/W25Q80DV.d 


# Each subdirectory must supply rules for building sources it contributes
Core/MyLibs/W25Q80DV/%.o Core/MyLibs/W25Q80DV/%.su: ../Core/MyLibs/W25Q80DV/%.c Core/MyLibs/W25Q80DV/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3 -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-MyLibs-2f-W25Q80DV

clean-Core-2f-MyLibs-2f-W25Q80DV:
	-$(RM) ./Core/MyLibs/W25Q80DV/W25Q80DV.d ./Core/MyLibs/W25Q80DV/W25Q80DV.o ./Core/MyLibs/W25Q80DV/W25Q80DV.su

.PHONY: clean-Core-2f-MyLibs-2f-W25Q80DV

