/*
 * M95P32.h
 *
 *  Created on: Mar 31, 2025
 *      Author: Giovanni
 */

#ifndef INC_M95P32_H_
#define INC_M95P32_H_

#include "main.h"
#include <string.h>
#include <stdio.h>

extern SPI_HandleTypeDef hspi1;

#define initTransmission HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET) //inicia a transmissão
#define endTransmission  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET) //inicia a transmissão

#define SPI_DELAY 		 100  //100 ms

#define DATAIN_COMMAND   0x02 //comando para escrita
#define DATAOUTS_COMMAND 0x03 //comando para saída de dado simpres (pelo MISO apenas)
#define WRITEOFF_COMMAND 0x04
#define RSTATUS_COMMAND	 0x05
#define WRITEEN_COMMAND  0x06
#define JEDEC_ID 		 0x9F

HAL_StatusTypeDef M95P32_Check();

uint8_t M95P32_ReadStatus();

HAL_StatusTypeDef M95P32_WriteOption(uint8_t select);

HAL_StatusTypeDef M95P32_ReadData(uint32_t address, uint8_t* dataIn, uint32_t lenght);

HAL_StatusTypeDef M95P32_WriteData(uint32_t address, uint8_t* dataIn, uint32_t lenght);

#endif /* INC_M95P32_H_ */
