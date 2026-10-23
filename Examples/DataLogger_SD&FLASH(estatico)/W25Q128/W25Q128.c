/*
 * W25Q128.cpp
 *
 *  Created on: Mar 30, 2026
 *      Author: giova
 */

#include "W25Q128.h"

// ==========================================
// FUNÇÕES PRIVADAS (Static)
// ==========================================

static void W25Q128_WriteEnable(W25Q128_HandleTypeDef* dev)
{
    uint8_t cmd = W25Q_CMD_WRITE_ENABLE;
    W25Q128_Select(dev);
    HAL_SPI_Transmit(dev->spiHandle, &cmd, 1, 10);
    W25Q128_Deselect(dev);
}

static uint8_t W25Q128_ReadStatusReg(W25Q128_HandleTypeDef* dev)
{
    uint8_t cmd = W25Q_CMD_READ_STATUS_1;
    uint8_t status = 0;

    W25Q128_Select(dev);
    HAL_SPI_Transmit(dev->spiHandle, &cmd, 1, 10);
    HAL_SPI_Receive(dev->spiHandle, &status, 1, 10);
    W25Q128_Deselect(dev);

    return status;
}

// ==========================================
// CONTROLE BÁSICO E ESTADOS
// ==========================================

void W25Q128_Init(W25Q128_HandleTypeDef* dev)
{
    dev->state = W25Q_STATE_IDLE;
    W25Q128_Deselect(dev);

    HAL_GPIO_WritePin(GET_PORT(dev->WP_Pin), GET_PIN(dev->WP_Pin), GPIO_PIN_SET);
}

void W25Q128_Select(W25Q128_HandleTypeDef* dev)
{
    HAL_GPIO_WritePin(GET_PORT(dev->CS_Pin), GET_PIN(dev->CS_Pin), GPIO_PIN_RESET);
}

void W25Q128_Deselect(W25Q128_HandleTypeDef* dev)
{
    HAL_GPIO_WritePin(GET_PORT(dev->CS_Pin), GET_PIN(dev->CS_Pin), GPIO_PIN_SET);
}

bool W25Q128_IsBusy(W25Q128_HandleTypeDef* dev)
{
    // O barramento SPI está ocupado pelo DMA?
    if (dev->state == W25Q_STATE_BUSY_SPI_TX || dev->state == W25Q_STATE_BUSY_SPI_RX) return true;

    // O barramento está livre, mas o chip está apagando/gravando internamente?
    if (dev->state == W25Q_STATE_BUSY_MEM)
    {
        uint8_t status = W25Q128_ReadStatusReg(dev);
        if ((status & 0x01) == 0x00) {
            dev->state = W25Q_STATE_IDLE;
            return false;
        }
        return true;
    }

    return false;
}

// ==========================================
// OPERAÇÕES DA MEMÓRIA
// ==========================================

uint32_t W25Q128_ReadID(W25Q128_HandleTypeDef* dev)
{
    if (W25Q128_IsBusy(dev)) return 0;

    uint8_t cmd = W25Q_CMD_JEDEC_ID;
    uint8_t id[3] = {0};

    W25Q128_Select(dev);
    HAL_SPI_Transmit(dev->spiHandle, &cmd, 1, 10);
    HAL_SPI_Receive(dev->spiHandle, id, 3, 10);
    W25Q128_Deselect(dev);

    return (id[0] << 16) | (id[1] << 8) | id[2];
}

HAL_StatusTypeDef W25Q128_ReadData(W25Q128_HandleTypeDef* dev, uint32_t address, uint8_t* data, uint32_t size)
{
    if (W25Q128_IsBusy(dev)) return HAL_BUSY;

    uint8_t cmd[4];
    cmd[0] = W25Q_CMD_READ_DATA;
    cmd[1] = (address >> 16) & 0xFF;
    cmd[2] = (address >> 8) & 0xFF;
    cmd[3] = address & 0xFF;

    W25Q128_Select(dev);


    HAL_SPI_Transmit(dev->spiHandle, cmd, 4, 10);

    dev->state = W25Q_STATE_BUSY_SPI_RX;
    return HAL_SPI_Receive_DMA(dev->spiHandle, data, size);
}

HAL_StatusTypeDef W25Q128_WritePage(W25Q128_HandleTypeDef* dev, uint32_t address, uint8_t* data, uint32_t size)
{
    if (W25Q128_IsBusy(dev)) return HAL_BUSY;
    W25Q128_WriteEnable(dev);
    uint8_t cmd[4];
    cmd[0] = W25Q_CMD_PAGE_PROGRAM;
    cmd[1] = (address >> 16) & 0xFF;
    cmd[2] = (address >> 8) & 0xFF;
    cmd[3] = address & 0xFF;

    W25Q128_Select(dev);
    // Envia o cabeçalho
    HAL_SPI_Transmit(dev->spiHandle, cmd, 4, 10);

    dev->state = W25Q_STATE_BUSY_SPI_TX;
    return HAL_SPI_Transmit_DMA(dev->spiHandle, data, size);

    // NOTA: O pino CS continua LOW
    // Ele será levantado pela interrupção (Callback) quando o DMA terminar.
}

HAL_StatusTypeDef W25Q128_EraseSector(W25Q128_HandleTypeDef* dev, uint32_t address)
{
    if (W25Q128_IsBusy(dev)) return HAL_BUSY;

    W25Q128_WriteEnable(dev);

    uint8_t cmd[4];
    cmd[0] = W25Q_CMD_SECTOR_ERASE;
    cmd[1] = (address >> 16) & 0xFF;
    cmd[2] = (address >> 8) & 0xFF;
    cmd[3] = address & 0xFF;

    W25Q128_Select(dev);
    HAL_SPI_Transmit(dev->spiHandle, cmd, 4, 10);
    W25Q128_Deselect(dev); // Levantar o CS inicia a deleção na memória

    // CPU está livre, mas o chip vai ficar ocupado por ~45ms apagando o setor.
    dev->state = W25Q_STATE_BUSY_MEM;
    return HAL_OK;
}

// ==========================================
// CALLBACK DO DMA (Chamado pela Interrupção)
// ==========================================

void W25Q128_DMA_CpltCallback(W25Q128_HandleTypeDef* dev)
{
    if (dev->state == W25Q_STATE_BUSY_SPI_TX)
    {
        W25Q128_Deselect(dev);
        dev->state = W25Q_STATE_BUSY_MEM;
    }
    else if (dev->state == W25Q_STATE_BUSY_SPI_RX)
    {
        W25Q128_Deselect(dev);
        dev->state = W25Q_STATE_IDLE;
    }
}
