################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../USB/Class/CompositeBuilder/Src/usbd_composite_builder.c 

C_DEPS += \
./USB/Class/CompositeBuilder/Src/usbd_composite_builder.d 

OBJS += \
./USB/Class/CompositeBuilder/Src/usbd_composite_builder.o 


# Each subdirectory must supply rules for building sources it contributes
USB/Class/CompositeBuilder/Src/%.o USB/Class/CompositeBuilder/Src/%.su USB/Class/CompositeBuilder/Src/%.cyclo: ../USB/Class/CompositeBuilder/Src/%.c USB/Class/CompositeBuilder/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/giova/OneDrive/Documentos/STM32/USB_COMPO_test/Application" -I"C:/Users/giova/OneDrive/Documentos/STM32/USB_COMPO_test/USB/Class/CompositeBuilder/Inc" -I"C:/Users/giova/OneDrive/Documentos/STM32/USB_COMPO_test/USB/Class/CDC/Inc" -I"C:/Users/giova/OneDrive/Documentos/STM32/USB_COMPO_test/USB/Class/MSC/Inc" -I"C:/Users/giova/OneDrive/Documentos/STM32/USB_COMPO_test/USB/Conf/Inc" -I"C:/Users/giova/OneDrive/Documentos/STM32/USB_COMPO_test/USB/Core/Inc" -I"C:/Users/giova/OneDrive/Documentos/STM32/USB_COMPO_test/USB/App" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-USB-2f-Class-2f-CompositeBuilder-2f-Src

clean-USB-2f-Class-2f-CompositeBuilder-2f-Src:
	-$(RM) ./USB/Class/CompositeBuilder/Src/usbd_composite_builder.cyclo ./USB/Class/CompositeBuilder/Src/usbd_composite_builder.d ./USB/Class/CompositeBuilder/Src/usbd_composite_builder.o ./USB/Class/CompositeBuilder/Src/usbd_composite_builder.su

.PHONY: clean-USB-2f-Class-2f-CompositeBuilder-2f-Src

