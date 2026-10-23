/*
 * pinName.h
 *
 *  Created on: Jul 28, 2026
 *      Author: giova
 */

#ifndef CONFIG_PINNAME_H_
#define CONFIG_PINNAME_H_

#include "main.h"

// Definição Struct PinName para ser mais natural na alocação de pinos GPIO

typedef struct
{
	GPIO_TypeDef* port;
	uint16_t pin;
}PinName;

#define GET_PORT(pn) ((pn).port)
#define GET_PIN(pn)  ((pn).pin)

#endif /* CONFIG_PINNAME_H_ */
