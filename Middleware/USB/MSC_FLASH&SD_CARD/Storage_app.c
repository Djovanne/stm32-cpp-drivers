/*
 * Storage_app.c
 *
 * Created on: Mar 29, 2026
 * Author: giova
 */

#include "Storage_app.h"
#include "bsp_sd_card.h"

////////////// Private Variables /////////////
uint8_t SD_error = 0;

///////////////////////////// Inicialização das memórias //////////////////////////////////

int8_t USB_Storage_Init(uint8_t lun)
{
	return USBD_OK;
}

///////////////////////////// Capacidade das memórias //////////////////////////////////

int8_t USB_Storage_GetCapacity(uint8_t lun, uint32_t *block_num, uint16_t *block_size)
{
	//return USBD_FAIL;
	if (lun == 0)
	{
		*block_num = FLASH_NUM_BLOCK;
		*block_size = FLASH_BLOCK_SIZE;
		return USBD_OK;

	}
	else if (lun == 1)
	{
		HAL_SD_CardInfoTypeDef CardInfo;

		if (HAL_SD_GetCardInfo(&hsd, &CardInfo) != HAL_OK) return USBD_FAIL;

		*block_num  = CardInfo.LogBlockNbr - 1;
		*block_size = CardInfo.LogBlockSize;
		return USBD_OK;

	}
    return USBD_FAIL;
}

///////////////////////////// Is Ready das memórias //////////////////////////////////

int8_t USB_Storage_IsReady(uint8_t lun)
{
	if(lun == 0)
	{
		return USBD_OK;
	}
	else if (lun == 1)
	{
		if(BSP_SD_CheckAndInit() == HAL_OK) return USBD_OK;
	}
	return USBD_FAIL;
}

///////////////////////////// Memória protegida //////////////////////////////////

int8_t USB_IsWriteProtected(uint8_t lun)
{
	if (lun == 1)
	        if (BSP_SD_GetStatus() == SD_ABSENT) return 1;
	return USBD_OK;
}

///////////////////////////// Leitura das memórias //////////////////////////////////

int8_t USB_Storage_Read(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
	if(lun == 0)
	{
		uint32_t address = blk_addr * FLASH_BLOCK_SIZE;
		uint32_t size = blk_len * FLASH_BLOCK_SIZE;

		W25Q128_ReadData(&flash, address, buf, size);

		return USBD_OK;
	}
	if(lun == 1)
	{
	    if (HAL_SD_ReadBlocks(&hsd, buf, blk_addr, blk_len, SD_TIMEOUT) != HAL_OK) return USBD_FAIL;

	    uint32_t timeStart = HAL_GetTick();
		while (HAL_SD_GetCardState(&hsd) != HAL_SD_CARD_TRANSFER)
		{
			if(HAL_GetTick() - timeStart >= SD_TIMEOUT) return USBD_FAIL;
		}
		return USBD_OK;

    }
    return USBD_FAIL;
}

///////////////////////////// Escrita das memórias //////////////////////////////////

int8_t USB_Storage_Write(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
	if(lun == 0)
	{
		static uint8_t sector_cache[4096];

		for (uint16_t bloco = 0; bloco < blk_len; bloco++)
		{
			uint32_t endereco_absoluto = (blk_addr + bloco) * 512;
			uint32_t inicio_do_setor = endereco_absoluto & 0xFFFFF000;
			uint32_t offset_interno = endereco_absoluto % 4096;

			W25Q128_ReadData(&flash, inicio_do_setor, sector_cache, 4096);
			memcpy(&sector_cache[offset_interno], buf + (bloco * 512), 512);
			W25Q128_EraseSector(&flash, inicio_do_setor);

		for (uint32_t pagina = 0; pagina < 4096; pagina += 256)
		{
			W25Q128_WritePage(&flash, inicio_do_setor + pagina, &sector_cache[pagina], 256);
		}
	}

	}
	else if(lun == 1)
	{
	    if (HAL_SD_WriteBlocks(&hsd, buf, blk_addr, blk_len, SD_TIMEOUT) != HAL_OK) return USBD_FAIL;

	    uint32_t timeStart = HAL_GetTick();
		while (HAL_SD_GetCardState(&hsd) != HAL_SD_CARD_TRANSFER)
		{
			if(HAL_GetTick() - timeStart >= SD_TIMEOUT) return USBD_FAIL;
		}
		return USBD_OK;
	}
    return USBD_FAIL;
}

int8_t USB_STORAGE_GetMaxLun()
{
  return (LUN_NUMBER - 1);
}
