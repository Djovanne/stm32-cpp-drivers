/*
 * W25Qxx.h
 *
 * Created on: Mar 30, 2026
 * Author: giova
 */

#ifndef W25Qxx_H_
#define W25Qxx_H_

#include "main.h"
#include <stdbool.h>
#include "pinName.h"

#ifdef __cplusplus
extern "C" {
#endif

// Comandos do datasheet W25Qxx
#define W25Q_CMD_WRITE_ENABLE     0x06
#define W25Q_CMD_READ_STATUS_1    0x05
#define W25Q_CMD_READ_DATA        0x03
#define W25Q_CMD_PAGE_PROGRAM     0x02
#define W25Q_CMD_SECTOR_ERASE     0x20
#define W25Q_CMD_CHIP_ERASE       0xC7
#define W25Q_CMD_JEDEC_ID         0x9F

// Máquina de estados do chip físico
typedef enum
{
    W25Q_STATE_IDLE = 0,
    W25Q_STATE_BUSY_MEM,   // flash ocupada (Write/Erase)
    W25Q_STATE_ERROR
} W25Qxx_State;

typedef struct
{
    SPI_HandleTypeDef* spiHandle;
    PinName CS_Pin;
    PinName WP_Pin;

    volatile W25Qxx_State state;
} W25Qxx_HandleTypeDef;

// ==========================================
// Inicialização e Controle de Linhas
// ==========================================

void W25Qxx_Init(W25Qxx_HandleTypeDef* dev);
void W25Qxx_Select(W25Qxx_HandleTypeDef* dev);
void W25Qxx_Deselect(W25Qxx_HandleTypeDef* dev);

// ==========================================
// Operações da Memória
// ==========================================

uint32_t W25Qxx_ReadID(W25Qxx_HandleTypeDef* dev);
HAL_StatusTypeDef W25Qxx_ReadData(W25Qxx_HandleTypeDef* dev, uint32_t address, uint8_t* data, uint32_t size);
HAL_StatusTypeDef W25Qxx_WritePage(W25Qxx_HandleTypeDef* dev, uint32_t address, const uint8_t* data, uint32_t size);
HAL_StatusTypeDef W25Qxx_EraseSector(W25Qxx_HandleTypeDef* dev, uint32_t address);

// ==========================================
// Consulta de Estados
// ==========================================

bool W25Qxx_IsBusy(W25Qxx_HandleTypeDef* dev);
W25Qxx_State W25Qxx_GetState(const W25Qxx_HandleTypeDef* dev);

#ifdef __cplusplus
}
#endif

#endif /* W25Qxx_H_ */
