################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../USB/Memory/EEPROM_M95P32/M95P32.c 

C_DEPS += \
./USB/Memory/EEPROM_M95P32/M95P32.d 

OBJS += \
./USB/Memory/EEPROM_M95P32/M95P32.o 


# Each subdirectory must supply rules for building sources it contributes
USB/Memory/EEPROM_M95P32/%.o USB/Memory/EEPROM_M95P32/%.su USB/Memory/EEPROM_M95P32/%.cyclo: ../USB/Memory/EEPROM_M95P32/%.c USB/Memory/EEPROM_M95P32/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/giova/OneDrive/Documentos/STM32/USB_COMPO_test/Application" -I"C:/Users/giova/OneDrive/Documentos/STM32/USB_COMPO_test/USB/App" -I"C:/Users/giova/OneDrive/Documentos/STM32/USB_COMPO_test/USB/Class/MSC/Inc" -I"C:/Users/giova/OneDrive/Documentos/STM32/USB_COMPO_test/USB/Core/Inc" -I"C:/Users/giova/OneDrive/Documentos/STM32/USB_COMPO_test/USB/Target" -I"C:/Users/giova/OneDrive/Documentos/STM32/USB_COMPO_test/USB/Memory" -I"C:/Users/giova/OneDrive/Documentos/STM32/USB_COMPO_test/USB/Memory/EEPROM_M95P32" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-USB-2f-Memory-2f-EEPROM_M95P32

clean-USB-2f-Memory-2f-EEPROM_M95P32:
	-$(RM) ./USB/Memory/EEPROM_M95P32/M95P32.cyclo ./USB/Memory/EEPROM_M95P32/M95P32.d ./USB/Memory/EEPROM_M95P32/M95P32.o ./USB/Memory/EEPROM_M95P32/M95P32.su

.PHONY: clean-USB-2f-Memory-2f-EEPROM_M95P32

