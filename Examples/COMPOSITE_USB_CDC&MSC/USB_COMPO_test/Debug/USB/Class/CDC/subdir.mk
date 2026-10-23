################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../USB/Class/CDC/usbd_cdc.c \
../USB/Class/CDC/usbd_cdc_if.c 

C_DEPS += \
./USB/Class/CDC/usbd_cdc.d \
./USB/Class/CDC/usbd_cdc_if.d 

OBJS += \
./USB/Class/CDC/usbd_cdc.o \
./USB/Class/CDC/usbd_cdc_if.o 


# Each subdirectory must supply rules for building sources it contributes
USB/Class/CDC/%.o USB/Class/CDC/%.su USB/Class/CDC/%.cyclo: ../USB/Class/CDC/%.c USB/Class/CDC/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/giova/OneDrive/Documentos/STM32/USB_COMPO_test/Application" -I"C:/Users/giova/OneDrive/Documentos/STM32/USB_COMPO_test/USB/App" -I"C:/Users/giova/OneDrive/Documentos/STM32/USB_COMPO_test/USB/Class/CDC" -I"C:/Users/giova/OneDrive/Documentos/STM32/USB_COMPO_test/USB/Core/Inc" -I"C:/Users/giova/OneDrive/Documentos/STM32/USB_COMPO_test/USB/Target" -I"C:/Users/giova/OneDrive/Documentos/STM32/USB_COMPO_test/USB/Class/COMPOSITE/Inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-USB-2f-Class-2f-CDC

clean-USB-2f-Class-2f-CDC:
	-$(RM) ./USB/Class/CDC/usbd_cdc.cyclo ./USB/Class/CDC/usbd_cdc.d ./USB/Class/CDC/usbd_cdc.o ./USB/Class/CDC/usbd_cdc.su ./USB/Class/CDC/usbd_cdc_if.cyclo ./USB/Class/CDC/usbd_cdc_if.d ./USB/Class/CDC/usbd_cdc_if.o ./USB/Class/CDC/usbd_cdc_if.su

.PHONY: clean-USB-2f-Class-2f-CDC

