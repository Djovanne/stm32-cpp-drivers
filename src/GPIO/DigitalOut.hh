/*
 * DigitalOut.hh
 *
 *  Created on: Nov 30, 2025
 *      Author: giova
 */

#ifndef DIGITALOUT_HH_
#define DIGITALOUT_HH_

#include "main.h"

class DigitalOut
{
public:

	DigitalOut(GPIO_TypeDef* gate, uint16_t pin);
	DigitalOut(PinName PPin);
	~DigitalOut();

	void write(bool state);
	bool state();
	void toggle();

	DigitalOut& operator = (bool value);
	operator bool();

private:

	GPIO_TypeDef* gate;
	uint16_t      pin;
};

#endif /* DIGITALOUT_HH_ */
