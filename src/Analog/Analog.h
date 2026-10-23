/*
 * Analog.h
 *
 *  Created on: Apr 1, 2026
 *      Author: giova
 */

#ifndef ANALOG_H
#define ANALOG_H

#include "main.h"

#ifdef HAL_ADC_MODULE_ENABLED

#include "GlobalDefines.h"

class Analog
{
private:

	ADC_HandleTypeDef* hadc;

	uint16_t adc_buffer[2]; // 1° bateria, 2° temperatura

	const float VREF = 3.3;     // tensão referência
	const float ADC_MAX = 4095; // Resolução

	float temp_f = -100;
	float batt_f = -100;

	// Battery
	const float R26 = 10000;
	const float R27 = 4700;
	// Temperature
	const float R4 = 10000;
	const float NTC_NOM = 10000;
	const float TEMP_NOM = 298.15;
	const float BCOEFFICIENT = 3950;

public:

	Analog(ADC_HandleTypeDef* hadc);

	HAL_StatusTypeDef init();

	float getBat();

	float getTemp();

	int getADC(uint8_t i);
};

#endif

#endif
