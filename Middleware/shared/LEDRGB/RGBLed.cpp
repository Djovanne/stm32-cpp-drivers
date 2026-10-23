/*
 * RGBLed.cpp
 *
 *  Created on: Apr 3, 2026
 *      Author: giova
 */

#include "RGBLed.h"

// Implementação do Construtor
RGBLed::RGBLed(TIM_HandleTypeDef* timer, uint32_t chR, uint32_t chG, uint32_t chB) {
    this->htim = timer;
    this->channelR = chR;
    this->channelG = chG;
    this->channelB = chB;

    HAL_TIM_PWM_Start(htim, channelR);
    HAL_TIM_PWM_Start(htim, channelG);
    HAL_TIM_PWM_Start(htim, channelB);
    turnOff(); // Garante que comece apagado
}

// Define os 3 canais simultaneamente
void RGBLed::setColor(uint8_t r, uint8_t g, uint8_t b) {
    __HAL_TIM_SET_COMPARE(htim, channelR, r);
    __HAL_TIM_SET_COMPARE(htim, channelG, g);
    __HAL_TIM_SET_COMPARE(htim, channelB, b);
}

// Desliga tudo
void RGBLed::turnOff() {
    setColor(0, 0, 0);
}

// Controles Individuais
void RGBLed::setRed(uint8_t r) {
    __HAL_TIM_SET_COMPARE(htim, channelR, r);
}

void RGBLed::setGreen(uint8_t g) {
    __HAL_TIM_SET_COMPARE(htim, channelG, g);
}

void RGBLed::setBlue(uint8_t b) {
    __HAL_TIM_SET_COMPARE(htim, channelB, b);
}
