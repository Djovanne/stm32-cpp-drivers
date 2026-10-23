/*
 * Storage_app.h
 *
 *  Created on: Mar 29, 2026
 *      Author: giova
 *
 *      - Não se esqueça que esse não é o arquivo que realmente implementa, ele só desacopla do gerado pelo MX.
 *
 *      - Para funcionar, substitua no App da USB "usbd_msc_if.c", retornando a função desse arquivo.
 *
 *      Não se esqueça de também mudar o InquirityData do original gerado pelo MX para as quantidades que queira de LUN
 *      como por exemplo:
 */
// const int8_t STORAGE_Inquirydata[] = {/* 36 */
//  /* LUN 0 */
//  0x00, 0x80, 0x02, 0x02, (STANDARD_INQUIRY_DATA_LEN - 5), 0x00, 0x00, 0x00,
//  'S', 'T', 'M', ' ', ' ', ' ', ' ', ' ', /* Manufacturer : 8 bytes */
//  'E', 'E', 'P', 'R', 'O', 'M', ' ', ' ', /* Product      : 16 Bytes */
//  ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
//  '0', '.', '0' ,'1',                      /* Version      : 4 Bytes */
//  /* LUN 1 */
//  0x00, 0x80, 0x02, 0x02, (STANDARD_INQUIRY_DATA_LEN - 5), 0x00, 0x00, 0x00,
//  'S', 'T', 'M', ' ', ' ', ' ', ' ', ' ', /* Manufacturer : 8 bytes */
//  'S', 'D', ' ', 'C', 'a', 'r', 'd', ' ', /* Product      : 16 Bytes */
//  ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
//  '0', '.', '0' ,'1',                      /* Version      : 4 Bytes */
//};


#ifndef USB_STORAGE_APP_H_
#define USB_STORAGE_APP_H_

#include "main.h"
#include "usbd_def.h"
#include "W25Q128.h"

////////////// Defines ///////////////////////
#define SD_TIMEOUT 10000
#define LUN_NUMBER 2 //número de dispositivos/ Flash não utilizada

#define FLASH_NUM_BLOCK  0x8000
#define FLASH_BLOCK_SIZE 0x200

////////////// Extern Handle /////////////
extern SD_HandleTypeDef hsd;
extern W25Q128_HandleTypeDef flash;
extern uint8_t SD_error;

int8_t USB_Storage_Init(uint8_t lun);
int8_t USB_Storage_GetCapacity(uint8_t lun, uint32_t *block_num, uint16_t *block_size);
int8_t USB_Storage_IsReady(uint8_t lun);
int8_t USB_IsWriteProtected(uint8_t lun);
int8_t USB_Storage_Read(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
int8_t USB_Storage_Write(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
int8_t USB_STORAGE_GetMaxLun();

#endif /* USB_STORAGE_APP_H_ */

//const int8_t STORAGE_Inquirydata[] = {/* 36 */
//
//  /* LUN 0 */
//  0x00, 0x80, 0x02, 0x02, (STANDARD_INQUIRY_DATA_LEN - 5), 0x00, 0x00, 0x00,
//  'G', 'I', 'O', 'V', 'A', 'N', 'N', 'I', /* Manufacturer : 8 bytes */
//  'S', 'D', ' ', 'C', 'a', 'r', 'd', ' ', /* Product      : 16 Bytes */
//  ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
//  '1', '.', '0' ,'0'                      /* Version      : 4 Bytes */
//};
