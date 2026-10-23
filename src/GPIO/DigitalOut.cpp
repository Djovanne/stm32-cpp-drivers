/*
 * DigitalOut.cpp
 *
 *  Created on: Nov 30, 2025
 *      Author: giova
 */

#include "DigitalOut.hh"

DigitalOut::DigitalOut(GPIO_TypeDef* gate, uint16_t pin)
{
	this-> gate = gate;
	this-> pin  = pin;

	write(false);
}

DigitalOut::DigitalOut(PinName PPin) : DigitalOut(PPin.gate, PPin.pin){}

DigitalOut::~DigitalOut(){}

void DigitalOut::write(bool state)
{
	HAL_GPIO_WritePin(gate, pin, (GPIO_PinState)state);
}

bool DigitalOut::state()
{
	return HAL_GPIO_ReadPin(gate, pin);
}
void DigitalOut::toggle()
{
	HAL_GPIO_TogglePin(gate, pin);
}

DigitalOut& DigitalOut::operator =(bool value)
{
	write(value != 0);
	return *this;
}

DigitalOut::operator bool()
{
    return state();
}
