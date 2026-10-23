/*
 * I2C_Scan.c
 *
 *  Created on: Out 17, 2025
 *      Author: giova
 */

#include "I2C_Scan.h"

#define MAX_I2C_DEVICES 10

#ifdef HAL_I2C_MODULE_ENABLED

void I2C_SCAN(I2C_HandleTypeDef *hi2c1)
{
	uint8_t address_found[MAX_I2C_DEVICES];
    uint8_t devices_found = 0;

    for (uint8_t address = 0x03; address < 0x78; address++)
    {
        if (HAL_I2C_IsDeviceReady(hi2c1, address << 1, 1, 10) == HAL_OK)
        {
        	if (devices_found < sizeof(address_found))
        	    address_found[devices_found] = address << 1;

            devices_found++;
        }
    }

    if (devices_found == 0)
    	printf("Nenhum dispositivo encontrado no barramento I2C.\n\r");

    else
    {
    	printf("Foi encontrado %u dispositivo(s) nesse barramento I2C, com endereco(s):\n\r",devices_found);
    	uint8_t print_count = (devices_found > MAX_I2C_DEVICES) ? MAX_I2C_DEVICES : devices_found;
    	for (int i = 0; i < print_count; i++)
    	    printf("8-bit: 0x%02X | Real (7-bit): 0x%02X\n\r", address_found[i], address_found[i] >> 1);
    }
}

#endif

/////////////////////////////// Fast I2C Bus /////////////////////////////////////

#ifdef HAL_FMPI2C_MODULE_ENABLED

void FMPI2C_SCAN(FMPI2C_HandleTypeDef *hfmpi2c1)
{
    uint8_t address_found[10];
    uint8_t devices_found = 0;

    for (uint8_t address = 0x03; address < 0x78; address++)
    {
        if (HAL_FMPI2C_IsDeviceReady(hfmpi2c1, address << 1, 1, 10) == HAL_OK)
        {
        	if (devices_found < sizeof(address_found))
        	    address_found[devices_found] = address << 1;

            devices_found++;
        }
    }

    if (devices_found == 0)
        printf("Nenhum dispositivo encontrado no barramento FMPI2C.\n\r");
    else
    {
        printf("Foi encontrado %u dispositivo(s) no barramento FMPI2C, com endereco(s):\n\r", devices_found);
        uint8_t print_count = (devices_found > MAX_I2C_DEVICES) ? MAX_I2C_DEVICES : devices_found;
        for (int i = 0; i < print_count; i++)
            printf("8-bit: 0x%02X | Real (7-bit): 0x%02X\n\r", address_found[i], address_found[i] >> 1);
    }
}

#endif
