/*
 * RGBLed.h
 *
 *  Created on: Apr 3, 2026
 *      Author: giova
 */

#ifndef RGB_LED_H
#define RGB_LED_H

#include "main.h"
#include "GlobalDefines.h"

#ifdef HAL_TIM_MODULE_ENABLED

class RGBLed
{
private:
    TIM_HandleTypeDef* htim;
    uint32_t channelR;
    uint32_t channelG;
    uint32_t channelB;

public:
    // Construtor
    RGBLed(TIM_HandleTypeDef* timer, uint32_t chR, uint32_t chG, uint32_t chB);

    // Método principal para definir a cor (0 a 100 no caso)
    void setColor(uint8_t r, uint8_t g, uint8_t b);

    // Métodos
    void turnOff();
    void setRed(uint8_t r);
    void setGreen(uint8_t g);
    void setBlue(uint8_t b);
};

#endif

#endif /* RGB_LED_H */
