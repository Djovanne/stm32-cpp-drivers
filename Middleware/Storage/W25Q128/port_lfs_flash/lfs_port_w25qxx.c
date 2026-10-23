/*
 * lfs_port_W25Qxx.c
 *
 *  Created on: Jul 20, 2026
 *      Author: giova
 */

#include "lfs_port_w25qxx.h"
#include "W25Qxx.h"
#include "main.h"

static uint8_t lfs_read_buf[256];
static uint8_t lfs_prog_buf[256];
static uint32_t lfs_lookahead_buf[256 / 4];

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void stmlfs_hal_setupConfig(struct lfs_config *c, W25Qxx_HandleTypeDef *dev, uint32_t block_count)
{
    memset(c, 0, sizeof(*c));

    // Vincula o contexto do driver
    c->context = dev;

    // Funções de HAL definidas
    c->read  = stmlfs_hal_read;
    c->prog  = stmlfs_hal_prog;
    c->erase = stmlfs_hal_erase;
    c->sync  = stmlfs_hal_sync;

    // Buffers estáticos
    c->read_buffer      = lfs_read_buf;
	c->prog_buffer      = lfs_prog_buf;
	c->lookahead_buffer = lfs_lookahead_buf;

    // Configuração do chip W25Qxx (em Bytes)
    c->read_size      = 256;
    c->prog_size      = 256;
    c->block_size     = 4096; // Tamanho do Setor
    c->block_count    = block_count;
    c->cache_size     = 256;
    c->lookahead_size = 256;
    c->block_cycles   = 500;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

int stmlfs_hal_init(struct lfs_config *c, W25Qxx_HandleTypeDef *dev)
{
	if (dev == NULL || c == NULL) return LFS_ERR_INVAL;

	W25Qxx_Init(dev);
	HAL_Delay(5);

	uint32_t jedec_id = W25Qxx_ReadID(dev);

	/* O ID do jedec da Winbond segue a seguinte lógica:
	 *
	 * 1 Byte: 0xEF (ID do frabricante Winbond)
	 * 2 Byte: 0x40 (Diz que a interface é SPI)
	 * 3 Byte: 0x18 (é a capacidade em potencia de 2^N, no caso 0x18 = 0d24 -> 2^24 Byte = 16 MBytes
	 */

	uint8_t mf_id    = (jedec_id >> 16) & 0xFF;
	uint8_t mem_type = (jedec_id >> 8)  & 0xFF;
	uint8_t cap_code = jedec_id  & 0xFF;

	// ID de verificacao
	if (mf_id != MANUFACTORY_ID || mem_type != MEM_TYPE_ID) return LFS_ERR_IO;

	// Intervalo possivel de memória
	if (cap_code < 0x15 || cap_code > 0x18) return LFS_ERR_IO;

	/* Subtrai 12 para converter direto para numero de blocos
	 *
	 * (/4096 bytes = log2(4096) = >> 12) para dividir pelo tamanho de cada bloco
	 */

	uint32_t block_count = (1UL << (cap_code - 12));

	stmlfs_hal_setupConfig(c, dev, block_count);

	return LFS_ERR_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

int stmlfs_hal_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void* buffer, lfs_size_t size)
{
	W25Qxx_HandleTypeDef* dev = (W25Qxx_HandleTypeDef*)c->context;

	while(W25Qxx_IsBusy(dev));

	HAL_StatusTypeDef status = W25Qxx_ReadData(dev, (block * c->block_size) + off, (uint8_t*)buffer, size);

	if (status != HAL_OK) return LFS_ERR_IO;

	return LFS_ERR_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

int stmlfs_hal_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void* buffer, lfs_size_t size)
{
	W25Qxx_HandleTypeDef* dev = (W25Qxx_HandleTypeDef*)c->context;

	while(W25Qxx_IsBusy(dev));

	HAL_StatusTypeDef status = W25Qxx_WritePage(dev, (block * c->block_size) + off, (uint8_t*)buffer, size);

	if (status != HAL_OK) return LFS_ERR_IO;

	while (W25Qxx_IsBusy(dev));

	return LFS_ERR_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

int stmlfs_hal_erase(const struct lfs_config *c, lfs_block_t block)
{
	W25Qxx_HandleTypeDef* dev = (W25Qxx_HandleTypeDef*)c->context;

	while(W25Qxx_IsBusy(dev));

	HAL_StatusTypeDef status = W25Qxx_EraseSector(dev, block * c->block_size);

	if (status != HAL_OK) return LFS_ERR_IO;

	return LFS_ERR_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

int stmlfs_hal_sync(const struct lfs_config *c)
{
	W25Qxx_HandleTypeDef* dev = (W25Qxx_HandleTypeDef*)c->context;

	while (W25Qxx_IsBusy(dev));
	return LFS_ERR_OK;
}
