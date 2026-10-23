/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_storage_if.c
  * @version        : v1.0_Cube
  * @brief          : Memory management layer.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "usbd_msc_if.h"

/* USER CODE BEGIN INCLUDE */

#include "M95P32.h"

/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
uint8_t MSC_Storage[32*1024];
/* USER CODE END PV */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief Usb device.
  * @{
  */

/** @defgroup USBD_STORAGE
  * @brief Usb mass storage device module
  * @{
  */

/** @defgroup USBD_STORAGE_Private_TypesDefinitions
  * @brief Private types.
  * @{
  */

/* USER CODE BEGIN PRIVATE_TYPES */

/* USER CODE END PRIVATE_TYPES */

/**
  * @}
  */

/** @defgroup USBD_STORAGE_Private_Defines
  * @brief Private defines.
  * @{
  */

#define STORAGE_LUN_NBR                  1
#define STORAGE_BLK_NBR                  32*1024/512
#define STORAGE_BLK_SIZ                  512

/* USER CODE BEGIN PRIVATE_DEFINES */

#define NUM_STORAGE 	   2
#define EEPROEM_NUM_BLOCK  0x2000
#define EEPROEM_BLOCK_SIZE 0x200

#define SD_TIMEOUT 1000

uint8_t SD_error = 0;

/* USER CODE END PRIVATE_DEFINES */

/**
  * @}
  */

/** @defgroup USBD_STORAGE_Private_Macros
  * @brief Private macros.
  * @{
  */

/* USER CODE BEGIN PRIVATE_MACRO */

/* USER CODE END PRIVATE_MACRO */

/**
  * @}
  */

/** @defgroup USBD_STORAGE_Private_Variables
  * @brief Private variables.
  * @{
  */

/* USER CODE BEGIN INQUIRY_DATA */
/** USB Mass storage Standard Inquiry Data. */
const int8_t STORAGE_Inquirydata[] = {/* 36 */

  /* LUN 0 */
  0x00, 0x80, 0x02, 0x02, (STANDARD_INQUIRY_DATA_LEN - 5), 0x00, 0x00, 0x00,
  'S', 'T', 'M', ' ', ' ', ' ', ' ', ' ', /* Manufacturer : 8 bytes */
  'E', 'E', 'P', 'R', 'O', 'M', ' ', ' ', /* Product      : 16 Bytes */
  ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
  '0', '.', '0' ,'1',                      /* Version      : 4 Bytes */

  /* LUN 1 */
  0x00, 0x80, 0x02, 0x02, (STANDARD_INQUIRY_DATA_LEN - 5), 0x00, 0x00, 0x00,
  'S', 'T', 'M', ' ', ' ', ' ', ' ', ' ', /* Manufacturer : 8 bytes */
  'S', 'D', ' ', 'C', 'a', 'r', 'd', ' ', /* Product      : 16 Bytes */
  ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
  '0', '.', '0' ,'1',                      /* Version      : 4 Bytes */
};
/* USER CODE END INQUIRY_DATA */

/* USER CODE BEGIN PRIVATE_VARIABLES */

/* USER CODE END PRIVATE_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_STORAGE_Exported_Variables
  * @brief Public variables.
  * @{
  */

extern USBD_HandleTypeDef hUsbDevice;

/* USER CODE BEGIN EXPORTED_VARIABLES */

extern SD_HandleTypeDef hsd;

/* USER CODE END EXPORTED_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_STORAGE_Private_FunctionPrototypes
  * @brief Private functions declaration.
  * @{
  */

static int8_t STORAGE_Init(uint8_t lun);
static int8_t STORAGE_GetCapacity(uint8_t lun, uint32_t *block_num, uint16_t *block_size);
static int8_t STORAGE_IsReady(uint8_t lun);
static int8_t STORAGE_IsWriteProtected(uint8_t lun);
static int8_t STORAGE_Read(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
static int8_t STORAGE_Write(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
static int8_t STORAGE_GetMaxLun(void);

/* USER CODE BEGIN PRIVATE_FUNCTIONS_DECLARATION */

/* USER CODE END PRIVATE_FUNCTIONS_DECLARATION */

/**
  * @}
  */

USBD_StorageTypeDef USBD_Storage_Interface_fops =
{
  STORAGE_Init,
  STORAGE_GetCapacity,
  STORAGE_IsReady,
  STORAGE_IsWriteProtected,
  STORAGE_Read,
  STORAGE_Write,
  STORAGE_GetMaxLun,
  (int8_t *)STORAGE_Inquirydata
};

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initializes over USB FS IP
  * @param  lun:
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t STORAGE_Init(uint8_t lun)
{
  if (lun == 0)
  {
    /* LUN 0: EEPROM_Init() */
    return (USBD_OK);
  }
  else if (lun == 1)
  {
    if(HAL_SD_GetCardState(&hsd) == HAL_SD_CARD_TRANSFER)
    {
      return (USBD_OK);
    }
    return (USBD_FAIL);
  }
  return (USBD_FAIL);
}

/**
  * @brief  .
  * @param  lun: .
  * @param  block_num: .
  * @param  block_size: .
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t STORAGE_GetCapacity(uint8_t lun, uint32_t *block_num, uint16_t *block_size)
{
  if (lun == 0)
  {
    *block_num  = EEPROEM_NUM_BLOCK;
    *block_size = EEPROEM_BLOCK_SIZE;
    return (USBD_OK);
  }
  else if (lun == 1)
  {
    HAL_SD_CardInfoTypeDef CardInfo;

    if (HAL_SD_GetCardInfo(&hsd, &CardInfo) == HAL_OK)
    {
      *block_num  = CardInfo.LogBlockNbr - 1; /* O Windows exige "Total de Blocos - 1" */
      *block_size = CardInfo.LogBlockSize;    /* Geralmente 512 bytes */
      return (USBD_OK);
    }
    return (USBD_FAIL);
  }
  return (USBD_FAIL);
}

/**
  * @brief  .
  * @param  lun: .
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t STORAGE_IsReady(uint8_t lun)
{
  if (lun == 0)
  {
    /* LUN 0: EEPROM */
    return (USBD_OK);
  }
  else if (lun == 1)
  {
    /* LUN 1: SD Card */
    if (HAL_SD_GetCardState(&hsd) == HAL_SD_CARD_TRANSFER)
    {
    	SD_error = 1;
        return (USBD_OK);
    }
    else
    {
    	if(SD_error == 0)
    	{
    		HAL_SD_DeInit(&hsd);
    		SD_error = 1;
    	}
		HAL_SD_Init(&hsd);
    	return (USBD_FAIL);
    }
  }
  return (USBD_FAIL);
}

/**
  * @brief  .
  * @param  lun: .
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t STORAGE_IsWriteProtected(uint8_t lun)
{
  /* USER CODE BEGIN 5 */
  return (USBD_OK);
  /* USER CODE END 5 */
}

/**
  * @brief  .
  * @param  lun: .
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t STORAGE_Read(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
  if (lun == 0)
  {
	if (M95P32_ReadData(blk_addr * STORAGE_BLK_SIZ, buf, blk_len * STORAGE_BLK_SIZ) != HAL_OK)
		return (USBD_FAIL);
    return (USBD_OK);
  }
  else if (lun == 1)
  {
    if (HAL_SD_ReadBlocks(&hsd, buf, blk_addr, blk_len, SD_TIMEOUT) == HAL_OK)
    {
      while (HAL_SD_GetCardState(&hsd) != HAL_SD_CARD_TRANSFER)
      {
      }
      return (USBD_OK);
    }
    return (USBD_FAIL);
  }
  return (USBD_FAIL);
}

/**
  * @brief  .
  * @param  lun: .
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t STORAGE_Write(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
  if (lun == 0)
  {
	if (M95P32_WriteData(blk_addr * STORAGE_BLK_SIZ, buf, blk_len * STORAGE_BLK_SIZ) != HAL_OK)
		return (USBD_FAIL);
    return (USBD_OK);
  }
  else if (lun == 1)
  {
    if (HAL_SD_WriteBlocks(&hsd, buf, blk_addr, blk_len, SD_TIMEOUT) == HAL_OK)
    {
      while (HAL_SD_GetCardState(&hsd) != HAL_SD_CARD_TRANSFER)
      {
      }
      return (USBD_OK);
    }
    return (USBD_FAIL);
  }

  return (USBD_FAIL);
}

/**
  * @brief  .
  * @param  None
  * @retval .
  */
int8_t STORAGE_GetMaxLun(void)
{
  /* USER CODE BEGIN 8 */
  return (NUM_STORAGE - 1);
  /* USER CODE END 8 */
}

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */

/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
