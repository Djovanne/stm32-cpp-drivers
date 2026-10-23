/*
 * HM10.cpp
 *
 *  Created on: Jan 7, 2026
 *      Author: giova
 */

#include <HM10.hh>

HM10::HM10(UART_HandleTypeDef* huart) : huart(huart){}

void HM10::sendChar(char c)
{
    // Envia 1 byte, timeout de 10ms
    HAL_UART_Transmit(huart, (uint8_t*)&c, 1, 10);
}

void HM10::print(const char* str) {
    // Envia o array de bytes até encontrar o terminador nulo
    HAL_UART_Transmit(huart, (uint8_t*)str, strlen(str), 100);
}

void HM10::println(const char* str) {
    print(str);
    print("\r\n");
}

void HM10::print(int number) {
    char buffer[16];
    sprintf(buffer, "%d", number);
    print(buffer);
}

void HM10::sendAT(const char* command) {
    print(command);
}

HAL_StatusTypeDef HM10::startListening(uint8_t *cmdByte) {
    // Aponta a interrupção para escrever na variável privada cmdByte
    return HAL_UART_Receive_IT(huart, cmdByte, 1);
}
