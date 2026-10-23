/*
 * ILPS28.h
 *
 *  Created on: Apr 1, 2026
 *      Author: giova
 */

#ifndef ILPS28_H
#define ILPS28_H

#include "main.h"
#ifdef HAL_I2C_MODULE_ENABLED

#define REG_WHO_AM_I   0x0F
#define REG_CTRL_REG1  0x10
#define REG_CTRL_REG2  0x11
#define REG_CTRL_REG3  0x12
#define REG_STATUS     0x27
#define REG_PRESS_XL   0x28
#define REG_TEMP_L     0x2B

#define ILPS28_ID      0xB4

class ILPS28
{
private:
    I2C_HandleTypeDef* hi2c;
    uint16_t i2c_addr;

    bool writeReg(uint8_t reg, uint8_t value);
    bool readRegs(uint8_t reg, uint8_t* buffer, uint16_t len);

public:

    ILPS28(I2C_HandleTypeDef* i2c_handle, uint16_t address = 0xB8);

    bool init();

    float getPressure();

    float getTemperature();
};

#endif

#endif // ILPS28_H
