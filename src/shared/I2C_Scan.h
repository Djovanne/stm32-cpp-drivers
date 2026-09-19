/*
 * I2C_Scan.h
 *
 *  Created on: Aug 31, 2025
 *      Author: giova
 */

#ifndef UTILITY_I2C_SCAN_H_
#define UTILITY_I2C_SCAN_H_

#include "main.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/////////////////////////////// I2C Bus /////////////////////////////////////

#ifdef HAL_I2C_MODULE_ENABLED

void I2C_SCAN(I2C_HandleTypeDef *hi2c1);

#endif

/////////////////////////////// Fast I2C Bus /////////////////////////////////////

#ifdef HAL_FMPI2C_MODULE_ENABLED

void FMPI2C_SCAN(FMPI2C_HandleTypeDef *hfmpi2c1);

#endif

#ifdef __cplusplus
}
#endif

#endif /* UTILITY_I2C_SCAN_H_ */


