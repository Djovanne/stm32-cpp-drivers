/*
 * App_main.hh
 *
 *  Created on: Jan 7, 2026
 *      Author: giova
 */
#include "main.h" // Importa as definições da HAL e do STM32

#ifdef HAL_UART_MODULE_ENABLED

#ifndef HM10_H
#define HM10_H

#include <string.h>
#include <stdio.h>

class HM10
{

public:
    HM10(UART_HandleTypeDef* huart);

    void sendChar(char c);
    void print(const char* str);
    void println(const char* str);
    void print(int number);
    void sendAT(const char* command);

    HAL_StatusTypeDef startListening(uint8_t *cmdByte);

private:
    UART_HandleTypeDef* huart;
};

#endif

#endif
