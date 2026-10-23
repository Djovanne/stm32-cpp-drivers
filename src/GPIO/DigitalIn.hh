/*
 * DigitalIn.hh
 *
 *  Created on: Nov 30, 2025
 *      Author: giova
 */

#ifndef DIGITALIN_HH_
#define DIGITALIN_HH_

#include "main.h"

class DigitalIn
{
public:

    DigitalIn(GPIO_TypeDef* gate, uint16_t pin);
    DigitalIn(PinName PPin);
    ~DigitalIn();

    bool read();
    operator int();

private:
    GPIO_TypeDef* gate;
    uint16_t      pin;
};



#endif /* DIGITALIN_HH_ */
