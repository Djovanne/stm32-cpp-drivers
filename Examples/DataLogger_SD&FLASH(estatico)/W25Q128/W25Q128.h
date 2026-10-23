/*
 * W25Q128.h
 *
 * Created on: Mar 30, 2026
 * Author: giova
 */

#ifndef W25Q128_H_
#define W25Q128_H_

#include "main.h"
#include "GlobalDefines.h" // Inclui a sua struct PinName
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Comandos do datasheet
#define W25Q_CMD_WRITE_ENABLE    0x06
#define W25Q_CMD_READ_STATUS_1   0x05
#define W25Q_CMD_READ_DATA       0x03
#define W25Q_CMD_PAGE_PROGRAM    0x02
#define W25Q_CMD_SECTOR_ERASE    0x20
#define W25Q_CMD_CHIP_ERASE      0xC7
#define W25Q_CMD_JEDEC_ID        0x9F

// Máquina de estados

typedef enum
{
    W25Q_STATE_IDLE = 0,
    W25Q_STATE_BUSY_SPI_TX,      // DMA transmitindo dados
    W25Q_STATE_BUSY_SPI_RX,      // DMA recebendo dados
	W25Q_STATE_BUSY_MEM,   // SPI livre
    W25Q_STATE_ERROR
} W25Q128_State;


typedef struct
{
    SPI_HandleTypeDef* spiHandle;
    PinName CS_Pin;
    PinName WP_Pin;
    volatile W25Q128_State state; // Volatile pois será alterado na interrupção
} W25Q128_HandleTypeDef;

// --- Protótipos das Funções ---

// ==========================================
// Inicialização
// ==========================================

void W25Q128_Init(W25Q128_HandleTypeDef* dev);
void W25Q128_Select(W25Q128_HandleTypeDef* dev);
void W25Q128_Deselect(W25Q128_HandleTypeDef* dev);

// ==========================================
// Funções de Operação da Memória (Assíncrona)
// ==========================================

uint32_t W25Q128_ReadID(W25Q128_HandleTypeDef* dev);
HAL_StatusTypeDef W25Q128_ReadData(W25Q128_HandleTypeDef* dev, uint32_t address, uint8_t* data, uint32_t size);
HAL_StatusTypeDef W25Q128_WritePage(W25Q128_HandleTypeDef* dev, uint32_t address, uint8_t* data, uint32_t size);
HAL_StatusTypeDef W25Q128_EraseSector(W25Q128_HandleTypeDef* dev, uint32_t address);

// ==========================================
// Funções de estado
// ==========================================

bool W25Q128_IsBusy(W25Q128_HandleTypeDef* dev);
void W25Q128_DMA_CpltCallback(W25Q128_HandleTypeDef* dev);

#ifdef __cplusplus
}
#endif

#endif /* W25Q128_H_ */
