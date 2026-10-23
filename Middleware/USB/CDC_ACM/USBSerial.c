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
//#include "usbd_cdc_acm_if.h"
#include "usbd_cdc_if.h"

#define USB_TIMEOUT 10

extern USBD_HandleTypeDef hUsbDeviceFS;

// para terminal, cheque o cdc_if_control_FS
volatile uint8_t g_cdc_terminal_open = 0;

//extern UART_HandleTypeDef huart2;

int _write(int file, char *ptr, int len)
{
	// Ignora transmissao se o USB nao estiver conectado
	if (hUsbDeviceFS.dev_state != USBD_STATE_CONFIGURED) return -1;

	if (!g_cdc_terminal_open) return -1;

	uint32_t start = HAL_GetTick();
	uint8_t status;
	do
	{
		//HAL_UART_Transmit(UART_SERIAL, (uint8_t*)ptr, len, MAX_SERIAL_TIMEOUT);
		status = CDC_Transmit_FS((uint8_t*) ptr, len);
		//status = CDC_Transmit(0, (uint8_t*) ptr, len);
		if(status == USBD_OK) return len;

	} while((status == USBD_BUSY) && ((HAL_GetTick() - start) < USB_TIMEOUT));

	return -1;
}

void DTR_Control(uint8_t* pbuf)
{
	// pbuf[2] contém o estado das linhas de controle CDC:
	// Bit 0 = DTR (Data Terminal Ready) -> 1 quando o terminal abre no PC
	// Bit 1 = RTS (Request To Send)
	USBD_SetupReqTypedef *req = (USBD_SetupReqTypedef *)pbuf;
	g_cdc_terminal_open = (req->wValue & 0x0001) ? 1 : 0;
}
