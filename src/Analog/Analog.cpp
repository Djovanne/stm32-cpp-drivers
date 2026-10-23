/*
 * Analog.cpp
 *
 *  Created on: Apr 1, 2026
 *      Author: giova
 */

#include "Analog.h"
#include <math.h>


Analog::Analog(ADC_HandleTypeDef* hadc)
{
	this->hadc = hadc;
	this->adc_buffer[0] = 0;
	this->adc_buffer[1] = 0;
}

HAL_StatusTypeDef Analog::init()
{
	return HAL_ADC_Start_DMA(hadc, (uint32_t*)adc_buffer, 2);
}

float Analog::getBat()
{
	// tensão real pino
    float voltage_pin = ((float)adc_buffer[0] / ADC_MAX) * VREF;

    // Divisor de tensão
    float voltage_bat = voltage_pin * ((R26 + R27) / R27);

	float S = 0.1;

	if(batt_f == -100)
		batt_f = voltage_bat;
	else
		batt_f = (S*voltage_bat)+((1-S)*batt_f);

    return batt_f;
}

float Analog::getTemp()
{
	//tensão real no pino
	float voltage_pin = ((float)adc_buffer[1] / ADC_MAX) * VREF;

	//resistencia NTC
	float r_ntc = R4 * (voltage_pin / (VREF - voltage_pin));

	// Equação de Steinhart-Hart (Beta parameter equation)
	float tempKI = log(r_ntc/NTC_NOM)/BCOEFFICIENT + 1/TEMP_NOM;
	float tempC (1/tempKI - 273.15f);

	float S = 0.1;

	if(temp_f == -100)
		temp_f = tempC;
	else
		temp_f = (S*tempC)+((1-S)*temp_f);

	return temp_f;
}

int Analog::getADC(uint8_t i)
{
	return (int)adc_buffer[i];
}
