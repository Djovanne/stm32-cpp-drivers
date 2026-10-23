/*
 * App_main.cpp
 *
 *  Created on: Feb 23, 2026
 *      Author: giova
 *
 *      para o devido funcionamento do dispositivo, vc deve habilitar a class mass storation class do STM da lib USB_Device,
 *      e colocar nos métodos:
 *
 *      int8_t STORAGE_Read_FS(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
 *
 *      e
 *
 *      int8_t STORAGE_Write_FS(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
 *
 *      os códigos de gravação da sua memória
 */

#include "App_main.h"
#include "main.h"
#include <stdio.h>
#include <cstring>

void app_main()
{
	HAL_Delay(1000);
	printf("Estou funcionando!!!\n\r");
	while(1)
	{
		HAL_Delay(1000);
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
		printf("Ainda estou funcionando!!!\n\r");
	}
}


