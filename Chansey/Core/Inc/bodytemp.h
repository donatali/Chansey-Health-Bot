#include "stm32l4xx_hal.h"

#define temperatureAddress 0x5A
float getBodyTemp(I2C_HandleTypeDef *hi2c1);
uint8_t temp_ready(I2C_HandleTypeDef *hi2c1);
