################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../USB/Conf/Src/usbd_conf.c \
../USB/Conf/Src/usbd_desc.c 

C_DEPS += \
./USB/Conf/Src/usbd_conf.d \
./USB/Conf/Src/usbd_desc.d 

OBJS += \
./USB/Conf/Src/usbd_conf.o \
./USB/Conf/Src/usbd_desc.o 


# Each subdirectory must supply rules for building sources it contributes
USB/Conf/Src/%.o USB/Conf/Src/%.su USB/Conf/Src/%.cyclo: ../USB/Conf/Src/%.c USB/Conf/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/giova/OneDrive/Documentos/STM32/USB_COMPO_test/Application" -I"C:/Users/giova/OneDrive/Documentos/STM32/USB_COMPO_test/USB/Class/CompositeBuilder/Inc" -I"C:/Users/giova/OneDrive/Documentos/STM32/USB_COMPO_test/USB/Class/CDC/Inc" -I"C:/Users/giova/OneDrive/Documentos/STM32/USB_COMPO_test/USB/Class/MSC/Inc" -I"C:/Users/giova/OneDrive/Documentos/STM32/USB_COMPO_test/USB/Conf/Inc" -I"C:/Users/giova/OneDrive/Documentos/STM32/USB_COMPO_test/USB/Core/Inc" -I"C:/Users/giova/OneDrive/Documentos/STM32/USB_COMPO_test/USB/App" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-USB-2f-Conf-2f-Src

clean-USB-2f-Conf-2f-Src:
	-$(RM) ./USB/Conf/Src/usbd_conf.cyclo ./USB/Conf/Src/usbd_conf.d ./USB/Conf/Src/usbd_conf.o ./USB/Conf/Src/usbd_conf.su ./USB/Conf/Src/usbd_desc.cyclo ./USB/Conf/Src/usbd_desc.d ./USB/Conf/Src/usbd_desc.o ./USB/Conf/Src/usbd_desc.su

.PHONY: clean-USB-2f-Conf-2f-Src

