/*
 * W25Qxx.c
 *
 * Created on: Mar 30, 2026
 * Author: giova
 */

#include "W25Qxx.h"
#include "stdbool.h"

// ==========================================
// FUNÇÕES PRIVADAS (Static)
// ==========================================

static void W25Qxx_WriteEnable(W25Qxx_HandleTypeDef* dev)
{
    uint8_t cmd = W25Q_CMD_WRITE_ENABLE;
    W25Qxx_Select(dev);
    HAL_SPI_Transmit(dev->spiHandle, &cmd, 1, 50);
    W25Qxx_Deselect(dev);
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//

static uint8_t W25Qxx_ReadStatusReg(W25Qxx_HandleTypeDef* dev)
{
    uint8_t cmd = W25Q_CMD_READ_STATUS_1;
    uint8_t status = 0;

    W25Qxx_Select(dev);
    HAL_SPI_Transmit(dev->spiHandle, &cmd, 1, 50);
    HAL_SPI_Receive(dev->spiHandle, &status, 1, 50);
    W25Qxx_Deselect(dev);

    return status;
}

// ==========================================
// CONTROLE BÁSICO E ESTADOS
// ==========================================

void W25Qxx_Init(W25Qxx_HandleTypeDef* dev)
{
    dev->state = W25Q_STATE_IDLE;
    W25Qxx_Deselect(dev);

    if (dev->WP_Pin.port != NULL) {
        HAL_GPIO_WritePin(dev->WP_Pin.port, dev->WP_Pin.pin, GPIO_PIN_SET);
    }
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//

void W25Qxx_Select(W25Qxx_HandleTypeDef* dev)
{
    HAL_GPIO_WritePin(dev->CS_Pin.port, dev->CS_Pin.pin, GPIO_PIN_RESET);
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//

void W25Qxx_Deselect(W25Qxx_HandleTypeDef* dev)
{
    HAL_GPIO_WritePin(dev->CS_Pin.port, dev->CS_Pin.pin, GPIO_PIN_SET);
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//

bool W25Qxx_IsBusy(W25Qxx_HandleTypeDef* dev)
{
    // Se está em operação interna de gravação ou deleção
    if (dev->state == W25Q_STATE_BUSY_MEM)
    {
        uint8_t status = W25Qxx_ReadStatusReg(dev);

        if ((status & 0x01) == 0x00)
        {
            dev->state = W25Q_STATE_IDLE;
            return false;
        }
        return true;
    }
    return false;
}

W25Qxx_State W25Qxx_GetState(const W25Qxx_HandleTypeDef* dev)
{
    return dev->state;
}

// ==========================================
// OPERAÇÕES DA MEMÓRIA
// ==========================================

uint32_t W25Qxx_ReadID(W25Qxx_HandleTypeDef* dev)
{
    if (W25Qxx_IsBusy(dev)) return 0;

    uint8_t cmd = W25Q_CMD_JEDEC_ID;
    uint8_t id[3] = {0};

    W25Qxx_Select(dev);
    HAL_SPI_Transmit(dev->spiHandle, &cmd, 1, 50);
    HAL_SPI_Receive(dev->spiHandle, id, 3, 50);
    W25Qxx_Deselect(dev);

    return ((uint32_t)id[0] << 16) | ((uint32_t)id[1] << 8) | id[2];
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//

HAL_StatusTypeDef W25Qxx_ReadData(W25Qxx_HandleTypeDef* dev, uint32_t address, uint8_t* data, uint32_t size)
{
    if (W25Qxx_IsBusy(dev)) return HAL_BUSY;

    uint8_t cmd[4];
    cmd[0] = W25Q_CMD_READ_DATA;
    cmd[1] = (uint8_t)((address >> 16) & 0xFF);
    cmd[2] = (uint8_t)((address >> 8) & 0xFF);
    cmd[3] = (uint8_t)(address & 0xFF);

    W25Qxx_Select(dev);
    HAL_SPI_Transmit(dev->spiHandle, cmd, 4, 50);
    HAL_StatusTypeDef status = HAL_SPI_Receive(dev->spiHandle, data, size, 500);
    W25Qxx_Deselect(dev);

    return status;
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//

HAL_StatusTypeDef W25Qxx_WritePage(W25Qxx_HandleTypeDef* dev, uint32_t address, const uint8_t* data, uint32_t size)
{
    if (W25Qxx_IsBusy(dev)) return HAL_BUSY;

    W25Qxx_WriteEnable(dev);

    uint8_t cmd[4];
    cmd[0] = W25Q_CMD_PAGE_PROGRAM;
    cmd[1] = (uint8_t)((address >> 16) & 0xFF);
    cmd[2] = (uint8_t)((address >> 8) & 0xFF);
    cmd[3] = (uint8_t)(address & 0xFF);

    W25Qxx_Select(dev);
    HAL_SPI_Transmit(dev->spiHandle, cmd, 4, 50);
    HAL_StatusTypeDef status = HAL_SPI_Transmit(dev->spiHandle, (uint8_t*)data, size, 500);
    W25Qxx_Deselect(dev);

    if (status == HAL_OK) dev->state = W25Q_STATE_BUSY_MEM;

    else dev->state = W25Q_STATE_ERROR;

    return status;
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//

HAL_StatusTypeDef W25Qxx_EraseSector(W25Qxx_HandleTypeDef* dev, uint32_t address)
{
    if (W25Qxx_IsBusy(dev)) return HAL_BUSY;

    W25Qxx_WriteEnable(dev);

    uint8_t cmd[4];
    cmd[0] = W25Q_CMD_SECTOR_ERASE;
    cmd[1] = (uint8_t)((address >> 16) & 0xFF);
    cmd[2] = (uint8_t)((address >> 8) & 0xFF);
    cmd[3] = (uint8_t)(address & 0xFF);

    W25Qxx_Select(dev);
    HAL_StatusTypeDef status = HAL_SPI_Transmit(dev->spiHandle, cmd, 4, 50);
    W25Qxx_Deselect(dev);

    if (status == HAL_OK) dev->state = W25Q_STATE_BUSY_MEM; // Apagamento de setor leva tipicamente 45ms a 400ms

    else dev->state = W25Q_STATE_ERROR;

    return status;
}
