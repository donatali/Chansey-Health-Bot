#include "stm32l4xx_hal.h"



#define SENSOR_I2C_ADDRESS 0x44  // I2C address of the SHT3x-DIS sensor

// Convert raw temperature and humidity values into physical quantities
void convert_data(uint16_t raw_temp, uint16_t raw_humidity, float *temperature, float *humidity);

// Function to read sensor data (temperature and humidity) in Single Shot Mode
HAL_StatusTypeDef read_sensor_data(I2C_HandleTypeDef *hi2c, uint16_t command, uint16_t *temperature, uint16_t *humidity);

uint8_t temp_humid(float *buffer, I2C_HandleTypeDef *hi2c1);
