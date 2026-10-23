/*
 * DigitalIn.cpp
 *
 *  Created on: Nov 30, 2025
 *      Author: giova
 */

#include "DigitalIn.hh"

DigitalIn::DigitalIn(GPIO_TypeDef* gate, uint16_t pin)
{
    this->gate = gate;
    this->pin  = pin;
}

DigitalIn::DigitalIn(PinName PPin) : DigitalIn(PPin.gate,PPin.pin){}

DigitalIn::~DigitalIn() {}

bool DigitalIn::read()
{
    GPIO_PinState estado = HAL_GPIO_ReadPin(gate, pin);

    if (estado == GPIO_PIN_SET) return true;
    else return false;
}

DigitalIn::operator int()
{
    return read();
}
