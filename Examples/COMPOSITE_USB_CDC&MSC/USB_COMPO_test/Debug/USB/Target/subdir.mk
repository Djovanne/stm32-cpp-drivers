################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../USB/Target/usbd_conf.c 

C_DEPS += \
./USB/Target/usbd_conf.d 

OBJS += \
./USB/Target/usbd_conf.o 


# Each subdirectory must supply rules for building sources it contributes
USB/Target/%.o USB/Target/%.su USB/Target/%.cyclo: ../USB/Target/%.c USB/Target/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/giova/OneDrive/Documentos/STM32/USB_COMPO_test/Application" -I"C:/Users/giova/OneDrive/Documentos/STM32/USB_COMPO_test/USB/App" -I"C:/Users/giova/OneDrive/Documentos/STM32/USB_COMPO_test/USB/Class/CDC" -I"C:/Users/giova/OneDrive/Documentos/STM32/USB_COMPO_test/USB/Core/Inc" -I"C:/Users/giova/OneDrive/Documentos/STM32/USB_COMPO_test/USB/Target" -I"C:/Users/giova/OneDrive/Documentos/STM32/USB_COMPO_test/USB/Class/COMPOSITE/Inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-USB-2f-Target

clean-USB-2f-Target:
	-$(RM) ./USB/Target/usbd_conf.cyclo ./USB/Target/usbd_conf.d ./USB/Target/usbd_conf.o ./USB/Target/usbd_conf.su

.PHONY: clean-USB-2f-Target

