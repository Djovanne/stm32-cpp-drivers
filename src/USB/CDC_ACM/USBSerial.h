/*
 * USBSerial.h
 *
 *  Created on: Aug 4, 2026
 *      Author: giova
 */

#ifndef USB_USBSERIAL_H_
#define USB_USBSERIAL_H_

#include <stdint.h>

extern volatile uint8_t g_cdc_terminal_open;

// coloque dentro do usbd_cdc_if, CDC_SET_CONTROL_LINE_STATE para recepcao depender de terminal conectado
void DTR_Control(uint8_t* pbuf);

#endif /* USB_USBSERIAL_H_ */
