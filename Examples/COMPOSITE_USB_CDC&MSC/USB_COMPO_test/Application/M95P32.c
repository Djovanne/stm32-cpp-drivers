/*
 * M95P32.c
 *
 *  Created on: Mar 31, 2025
 *      Author: Giovanni
 */

#include "M95P32.h"

HAL_StatusTypeDef M95P32_Check()
{
	uint8_t command = JEDEC_ID; //instrução
	uint8_t response[3];          //Código  do fabricante, familia, densidade de memoria, tamanho UID e o proprio UID

	initTransmission;
	HAL_SPI_Transmit(&hspi1, &command, 1, SPI_DELAY);
	HAL_SPI_Receive(&hspi1, response, sizeof(response), SPI_DELAY);
	endTransmission;

	if(response[0] == 0x20)
	{
		debugPrint("EEPROM detectada e funcionando, com seus numeros de identificao:\n");
		debugPrint("Codigo do Fabricante STM: 0x%02X \n", response[0]);
		debugPrint("Codigo da familia do SPI: 0x%02X \n", response[1]);
		debugPrint("Codigo de densi. memoria: 0x%02X \n", response[2]);

		return HAL_OK;
	}

	else
	{
		debugPrint("Falha na comunicacao com a EEPROM\n");
		return HAL_ERROR;
	}
}

uint8_t M95P32_ReadStatus()
{
	HAL_StatusTypeDef staCom = HAL_OK;
	uint8_t command = RSTATUS_COMMAND;
	uint8_t status;

	initTransmission;
	staCom |= HAL_SPI_Transmit(&hspi1, &command, 1, SPI_DELAY);
	staCom |= HAL_SPI_Receive(&hspi1, &status, 1, SPI_DELAY);
	endTransmission;

    // Retorna apenas o status, sem perder tempo a imprimir texto!
	return status;
}

HAL_StatusTypeDef M95P32_WriteOption(uint8_t select)
{
	HAL_StatusTypeDef status = HAL_OK;
	uint8_t command = (select == 1) ? WRITEEN_COMMAND : WRITEOFF_COMMAND;

	initTransmission;
	status |= HAL_SPI_Transmit(&hspi1, &command, 1, SPI_DELAY);
	endTransmission;

	return status;
}

HAL_StatusTypeDef M95P32_ReadData(uint32_t address, uint8_t* dataOut, uint32_t lenght)
{
	HAL_StatusTypeDef status = HAL_OK;
	uint8_t command[4] =
	{
		DATAOUTS_COMMAND,
		(address >> 16) & 0xFF,
		(address >> 8)  & 0xFF,
		address         & 0xFF
	};

	initTransmission;
	status |= HAL_SPI_Transmit(&hspi1, command, 4, SPI_DELAY);
	status |= HAL_SPI_Receive(&hspi1, dataOut, lenght, SPI_DELAY);
	endTransmission;

    // Removemos o for() com printf. O USB precisa desta função rápida!
	return status;
}

HAL_StatusTypeDef M95P32_WriteData(uint32_t address, uint8_t* dataIn, uint32_t length)
{
    HAL_StatusTypeDef status = HAL_OK;
    uint32_t bytesRemaining = length;
    uint32_t currentAddr = address;
    uint8_t* dataPtr = dataIn;

    while (bytesRemaining > 0)
    {
        // 1. Calcula o espaço até o fim da página atual (512 bytes)
        uint32_t maxWrite = 512 - (currentAddr % 512);
        uint32_t sizeToWrite = (bytesRemaining > maxWrite) ? maxWrite : bytesRemaining;

        // 2. Habilita a escrita (Obrigatório antes de cada instrução de modificação) [cite: 2586, 2782]
        M95P32_WriteOption(1);

        // 3. Monta o comando de Page Write (Comando + 3 bytes de endereço) [cite: 3026, 3445]
        uint8_t command[4] = {
			DATAIN_COMMAND,
            (uint8_t)((currentAddr >> 16) & 0xFF),
            (uint8_t)((currentAddr >> 8) & 0xFF),
            (uint8_t)(currentAddr & 0xFF)
        };

        initTransmission;
        status = HAL_SPI_Transmit(&hspi1, command, 4, SPI_DELAY);
        if (status == HAL_OK) {
            status = HAL_SPI_Transmit(&hspi1, dataPtr, sizeToWrite, SPI_DELAY);
        }
        endTransmission;

        if (status != HAL_OK) return status;

        // 4. Aguarda o Write In Progress (WIP) ser limpo [cite: 2612, 2838]
        // O tempo típico é de 2ms para 512 bytes
        while(M95P32_ReadStatus() & 0x01);

        bytesRemaining -= sizeToWrite;
        currentAddr += sizeToWrite;
        dataPtr += sizeToWrite;
    }
    return HAL_OK;
}
