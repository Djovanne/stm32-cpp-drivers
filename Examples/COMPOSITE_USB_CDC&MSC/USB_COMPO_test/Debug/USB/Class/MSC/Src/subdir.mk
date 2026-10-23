################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../USB/Class/MSC/Src/usbd_msc.c \
../USB/Class/MSC/Src/usbd_msc_bot.c \
../USB/Class/MSC/Src/usbd_msc_data.c \
../USB/Class/MSC/Src/usbd_msc_scsi.c \
../USB/Class/MSC/Src/usbd_msc_storage.c 

C_DEPS += \
./USB/Class/MSC/Src/usbd_msc.d \
./USB/Class/MSC/Src/usbd_msc_bot.d \
./USB/Class/MSC/Src/usbd_msc_data.d \
./USB/Class/MSC/Src/usbd_msc_scsi.d \
./USB/Class/MSC/Src/usbd_msc_storage.d 

OBJS += \
./USB/Class/MSC/Src/usbd_msc.o \
./USB/Class/MSC/Src/usbd_msc_bot.o \
./USB/Class/MSC/Src/usbd_msc_data.o \
./USB/Class/MSC/Src/usbd_msc_scsi.o \
./USB/Class/MSC/Src/usbd_msc_storage.o 


# Each subdirectory must supply rules for building sources it contributes
USB/Class/MSC/Src/%.o USB/Class/MSC/Src/%.su USB/Class/MSC/Src/%.cyclo: ../USB/Class/MSC/Src/%.c USB/Class/MSC/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/giova/OneDrive/Documentos/STM32/USB_COMPO_test/Application" -I"C:/Users/giova/OneDrive/Documentos/STM32/USB_COMPO_test/USB/Class/CompositeBuilder/Inc" -I"C:/Users/giova/OneDrive/Documentos/STM32/USB_COMPO_test/USB/Class/CDC/Inc" -I"C:/Users/giova/OneDrive/Documentos/STM32/USB_COMPO_test/USB/Class/MSC/Inc" -I"C:/Users/giova/OneDrive/Documentos/STM32/USB_COMPO_test/USB/Conf/Inc" -I"C:/Users/giova/OneDrive/Documentos/STM32/USB_COMPO_test/USB/Core/Inc" -I"C:/Users/giova/OneDrive/Documentos/STM32/USB_COMPO_test/USB/App" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-USB-2f-Class-2f-MSC-2f-Src

clean-USB-2f-Class-2f-MSC-2f-Src:
	-$(RM) ./USB/Class/MSC/Src/usbd_msc.cyclo ./USB/Class/MSC/Src/usbd_msc.d ./USB/Class/MSC/Src/usbd_msc.o ./USB/Class/MSC/Src/usbd_msc.su ./USB/Class/MSC/Src/usbd_msc_bot.cyclo ./USB/Class/MSC/Src/usbd_msc_bot.d ./USB/Class/MSC/Src/usbd_msc_bot.o ./USB/Class/MSC/Src/usbd_msc_bot.su ./USB/Class/MSC/Src/usbd_msc_data.cyclo ./USB/Class/MSC/Src/usbd_msc_data.d ./USB/Class/MSC/Src/usbd_msc_data.o ./USB/Class/MSC/Src/usbd_msc_data.su ./USB/Class/MSC/Src/usbd_msc_scsi.cyclo ./USB/Class/MSC/Src/usbd_msc_scsi.d ./USB/Class/MSC/Src/usbd_msc_scsi.o ./USB/Class/MSC/Src/usbd_msc_scsi.su ./USB/Class/MSC/Src/usbd_msc_storage.cyclo ./USB/Class/MSC/Src/usbd_msc_storage.d ./USB/Class/MSC/Src/usbd_msc_storage.o ./USB/Class/MSC/Src/usbd_msc_storage.su

.PHONY: clean-USB-2f-Class-2f-MSC-2f-Src

