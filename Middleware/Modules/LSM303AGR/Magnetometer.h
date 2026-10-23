/*
 * magnetometer.h
 *
 *  Created on: Aug 31, 2025
 *      Author: giova
 */

#ifndef LSM303AGR_MAGNETOMETER_H_
#define LSM303AGR_MAGNETOMETER_H_

#include "main.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

extern I2C_HandleTypeDef hi2c1; //barramento que o magnetômetro está conectado
extern UART_HandleTypeDef huart2;

#define TEMP_CFG 0x1F  // endereço do TEMP_CFG_REG_A
#define MAG_CFG 0x60   // primeiro endereço MAG_CFG
#define ACC_CFG 0x20   // primeiro endereço ACC_CFG

// endereço dado magnético

#define X_L_MAG 0x68
#define X_H_MAG 0x69
#define Y_L_MAG 0x6A
#define Y_H_MAG 0x6B
#define Z_L_MAG 0x6C
#define Z_H_MAG 0x6D

// endereço dado acelerômetro

#define X_L_ACC 0x28
#define X_H_ACC 0x29
#define Y_L_ACC 0x2A
#define Y_H_ACC 0x2B
#define Z_L_ACC 0x2C
#define Z_H_ACC 0x2D

// endereço dado temperatura

#define TEMP_L 0x0C
#define TEMP_H 0x0D

// offset medido experimentalmente
#define OFFSET_X -38
#define OFFSET_Y 159
#define OFFSET_Z 256

#define MAG_ADDRESS        0x3C    //endereço do magnetometro
#define ACE_ADDRESS        0x32    //endereço do acelerometro

#define SENSOR_WAIT        100     //tempo de espera de resposta do sensor
#define REG_OFFSET_ADDRESS 0x45    //endereço do reg de offset

class Magnetometer
{		
	public:
		Magnetometer();
		~Magnetometer();

		HAL_StatusTypeDef init(I2C_HandleTypeDef *hi2c1);
		HAL_StatusTypeDef temperature_data(float* data);

		HAL_StatusTypeDef acceleration_data(float* accField);

		HAL_StatusTypeDef magnetic_data(float *magfield);
		HAL_StatusTypeDef magnetic_data(float *magfield, float *module);

		HAL_StatusTypeDef read_offset(int16_t *offset);
		HAL_StatusTypeDef set_offset(int16_t *offset);
		
	private:
		I2C_HandleTypeDef *hi2c1;
};

#endif /* LSM303AGR_MAGNETOMETER_H_ */
