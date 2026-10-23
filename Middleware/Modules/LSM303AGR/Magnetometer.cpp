/*
 * Magnetometer.cpp
 *
 *  Created on: Aug 31, 2025
 *      Author: giova
 */

#include <Magnetometer.h>
#include "math.h"

Magnetometer::Magnetometer(){
}

Magnetometer::~Magnetometer(){
}

HAL_StatusTypeDef Magnetometer::init(I2C_HandleTypeDef *hi2c1)
{
	this->hi2c1 = hi2c1;

	//confiurações em relação ao default~
	uint8_t reg_config_temp[] =
	{
		TEMP_CFG,  // endereço do TEMP_CFG_REG_A
		0xC0   // habilita sensor temperatura
	};

	HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(hi2c1, ACE_ADDRESS, reg_config_temp, sizeof(reg_config_temp), SENSOR_WAIT);

	if (status == HAL_OK)
		printf("Sensor de temperatura ativado com sucesso!\n\r");
	else
		printf("Falha na ativação do sensor de temperatura\n\r");

	///////////////////////////////////////////////////////////////

	uint8_t reg_config_acc[] =
	{
		ACC_CFG,  // endereço do TEMP_CFG_REG_A
		0x27,   // habilita sensor acelerômetro a 10 Hz
		0x00,
		0x00,
		0x88   //ativa o BLU, e a alta resolução
	};

	status = HAL_I2C_Master_Transmit(hi2c1, ACE_ADDRESS, reg_config_acc, sizeof(reg_config_acc), SENSOR_WAIT);

	if (status == HAL_OK)
		printf("Sensor acelerometro ativado com sucesso!\n\r");
	else
		printf("Falha na ativação do sensor de acelerometro\n\r");

	///////////////////////////////////////////////////////////////

	uint8_t reg_config_mag[] =
	{
		MAG_CFG, //endereço do reg_config_A
		0x80, //habilita a correção por temperatura e o modo continuo
		0x01, //habilita o filtro passa-baixa para diminuir ruidos
		0x10  //habilita o BDU
	};

	status = HAL_I2C_Master_Transmit(hi2c1, MAG_ADDRESS, reg_config_mag, sizeof(reg_config_mag), SENSOR_WAIT);

	if (status == HAL_OK)
		printf("Sensor magnetico ativado com sucesso!\n\r");
	else
		printf("Falha na ativação do sensor de magnetico\n\r");

	return status;
}

HAL_StatusTypeDef Magnetometer::temperature_data(float *data)
{
	uint8_t regAddL = TEMP_L;
	uint8_t tempData[2];

	HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(hi2c1, ACE_ADDRESS, &regAddL, 1, SENSOR_WAIT);

	if(status == HAL_OK)
		status = HAL_I2C_Master_Receive (hi2c1, ACE_ADDRESS, tempData, 2, SENSOR_WAIT);


	if(status != HAL_OK)
			printf("Falha na leitura da temperatura");

	printf("%d(e) , %d(d)\n",tempData[1], tempData[0]);

	*data = (float)((int16_t)((tempData[1] << 8) | tempData[0])) + 25.0;
	return status;
}

HAL_StatusTypeDef Magnetometer::acceleration_data(float* accField)
{
	uint8_t address = X_L_ACC; //byte menos significativo
	uint8_t axisField[6];

	HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(hi2c1, ACE_ADDRESS, &address, 1, SENSOR_WAIT);
	if(status == HAL_OK)
		status = HAL_I2C_Master_Receive (hi2c1, ACE_ADDRESS, axisField, 6, SENSOR_WAIT);

	if(status != HAL_OK)
	{
		printf("Falha na leitura do acelerometro");
		return status;
	}

	// Combina os bytes LSB e MSB para cada eixo

	int16_t data_x = (int16_t)((axisField[1] << 8) | axisField[0]);
	int16_t data_y = (int16_t)((axisField[3] << 8) | axisField[2]);
	int16_t data_z = (int16_t)((axisField[5] << 8) | axisField[4]);

	/* ----- Conversão para mili-g
	* 1. Os dados de 12 bits são justificados à esquerda no registrador de 16 bits, então fazemos um shift de 4 bits para a direita.
	* 2. A sensibilidade para HR ±2g é 1 mg/LSB.
	*/
	accField[0] = (float)(data_x >> 4) * 1.0f; //eixo X
	accField[1] = (float)(data_y >> 4) * 1.0f; //eixo Y
	accField[2] = (float)(data_z >> 4) * 1.0f; //eixo Z

	return status;
}

HAL_StatusTypeDef Magnetometer::magnetic_data(float* magField)
{
	uint8_t address = X_L_MAG; //byte menos significativo
	uint8_t axisField[6];

	HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(hi2c1, MAG_ADDRESS, &address, 1, SENSOR_WAIT);
	if(status == HAL_OK)
		status = HAL_I2C_Master_Receive (hi2c1, MAG_ADDRESS, axisField, 6, SENSOR_WAIT);

	if(status != HAL_OK)
	{
		printf("Falha na leitura magnética");
		return status;
	}

	//de acordo com o artigo AN5069 da STM, o fator 1.5 deixa a unidade em mG (miliGauss)
	magField[0] = (float)((int16_t)((axisField[1] << 8) | axisField[0])) * 1.5f - OFFSET_X; //eixo X
	magField[1] = (float)((int16_t)((axisField[3] << 8) | axisField[2])) * 1.5f - OFFSET_Y; //eixo Y
	magField[2] = (float)((int16_t)((axisField[5] << 8) | axisField[4])) * 1.5f - OFFSET_Z; //eixo Z

	return status;
}

HAL_StatusTypeDef Magnetometer::magnetic_data(float* magField, float *module)
{
	HAL_StatusTypeDef status = magnetic_data(magField);

	*module = sqrt(magField[0]*magField[0] + magField[1]*magField[1] + magField[2]*magField[2]);

	return status;
}

HAL_StatusTypeDef Magnetometer::read_offset(int16_t *offset)
{
	uint8_t reg_offset[6];
	uint8_t reg_offset_address =  0x45;

	HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(hi2c1, MAG_ADDRESS, &reg_offset_address, 1, SENSOR_WAIT);

	if(status == HAL_OK)
		status = HAL_I2C_Master_Receive (hi2c1, MAG_ADDRESS, reg_offset, 6, SENSOR_WAIT);


	if(status != HAL_OK)
	{
		printf("Falha na leitura dos offsets");
		return status;
	}

	offset[0] = (int16_t)((reg_offset[1] << 8) | reg_offset[0]); //offset X
	offset[1] = (int16_t)((reg_offset[3] << 8) | reg_offset[2]); //offset Y
	offset[2] = (int16_t)((reg_offset[5] << 8) | reg_offset[4]); //offset Z

	printf("Os valores de offset no eixos sao:\n");
	printf("offset_X = %d\n",offset[0]);
	printf("offset_Y = %d\n",offset[1]);
	printf("offset_Z = %d\n",offset[2]);

	return status;
}

HAL_StatusTypeDef Magnetometer::set_offset(int16_t *offsets)
{
	uint8_t signed_to_unsigned[7] =
	{
		REG_OFFSET_ADDRESS,           // endereço
		(uint8_t)(offsets[0] & 0xFF), // conteudo
		(uint8_t)(offsets[0] >> 8),
		(uint8_t)(offsets[1] & 0xFF),
		(uint8_t)(offsets[1] >> 8),
		(uint8_t)(offsets[2] & 0xFF),
		(uint8_t)(offsets[2] >> 8)
	};

	HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(hi2c1, MAG_ADDRESS, signed_to_unsigned, sizeof(signed_to_unsigned), SENSOR_WAIT);

	if(status != HAL_OK)
		printf("erro no envio do offset");
	else
		printf("offset configurado com sucesso\n");
		
	return status;
}
