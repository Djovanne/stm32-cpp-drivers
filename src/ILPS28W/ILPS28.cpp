/*
 * ILPS28.cpp
 *
 *  Created on: Apr 1, 2026
 *      Author: giova
 */

#include "ILPS28.h"

// Construtor
ILPS28::ILPS28(I2C_HandleTypeDef* i2c_handle, uint16_t address)
{
    this->hi2c = i2c_handle;
    this->i2c_addr = address;
}

bool ILPS28::writeReg(uint8_t reg, uint8_t value)
{
    HAL_StatusTypeDef status = HAL_I2C_Mem_Write(hi2c, i2c_addr, reg, I2C_MEMADD_SIZE_8BIT, &value, 1, 100);
    return (status == HAL_OK);
}

bool ILPS28::readRegs(uint8_t reg, uint8_t* buffer, uint16_t len)
{
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(hi2c, i2c_addr, reg, I2C_MEMADD_SIZE_8BIT, buffer, len, 100);
    return (status == HAL_OK);
}

bool ILPS28::init()
{
    uint8_t who_am_i = 0;

    if (!readRegs(REG_WHO_AM_I, &who_am_i, 1))
    {
        return false;
    }

    if (who_am_i != ILPS28_ID)
    {
        return false;
    }

    if (!writeReg(REG_CTRL_REG1, 0x1A)) return false;
    if (!writeReg(REG_CTRL_REG2, 0x08)) return false;
    if (!writeReg(REG_CTRL_REG3, 0x01)) return false;

    return true;
}

float ILPS28::getPressure()
{
    uint8_t status_reg = 0;

    if (!readRegs(REG_STATUS, &status_reg, 1)) {
        return 0.0f;
    }

    if (status_reg & 0x01)
    {
        uint8_t raw_data[3] = {0, 0, 0};

        if (readRegs(REG_PRESS_XL, raw_data, 3))
        {

            int32_t press_raw = (int32_t)((raw_data[2] << 16) | (raw_data[1] << 8) | raw_data[0]);

            if (press_raw & 0x00800000)
            {
                press_raw |= 0xFF000000;
            }

            return (float)press_raw / 4096.0f;
        }
    }

    return 0.0f;
}

float ILPS28::getTemperature()
{
    uint8_t status_reg = 0;

    if (!readRegs(REG_STATUS, &status_reg, 1))
    {
        return -273.15f;
    }

    if (status_reg & 0x02)
    {
        uint8_t raw_data[2] = {0, 0};

        if (readRegs(REG_TEMP_L, raw_data, 2))
        {

            int16_t temp_raw = (int16_t)((raw_data[1] << 8) | raw_data[0]);

            return (float)temp_raw / 100.0f;
        }
    }

    return -273.15f;
}
