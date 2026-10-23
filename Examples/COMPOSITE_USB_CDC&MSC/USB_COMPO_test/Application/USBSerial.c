/*
 * USBSerial.c
 *
 *  Created on: Jan 15, 2026
 *      Author: giova
 *
 * Para usar o USB como serial, basta incluir este arquivo no projeto e ativar o middleware:
 * USB Device - CDC (Virtual Com Port)
 * Se quiser mudar o nome do dispositivo USB, vá em:
 * Middleware -> USB Device -> Configuration Descriptor -> Product String
 */

#include <stdio.h>
#include "main.h"
#include "usbd_cdc_acm_if.h"

#define USB_TIMEOUT 100

//extern UART_HandleTypeDef huart2;

int _write(int file, char *ptr, int len)
{
	uint32_t start = HAL_GetTick();
	uint8_t status;
	do
	{
		//HAL_UART_Transmit(UART_SERIAL, (uint8_t*)ptr, len, MAX_SERIAL_TIMEOUT);
		status = CDC_Transmit(0, (uint8_t*) ptr, len);
		if(status == USBD_OK) return len;

	} while((status == USBD_BUSY) && ((HAL_GetTick() - start) < USB_TIMEOUT));

	return -1;
}
