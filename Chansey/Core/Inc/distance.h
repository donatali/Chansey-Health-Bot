#include "stm32l4xx_hal.h"
#include "../../Drivers/VL53L0X/core/inc/vl53l0x_api.h"

#define VL_W_A 0x52
#define VL_R_A 0x53

void TogglePin();
void SensorInit(  uint32_t *refSpadCount, uint8_t *isApertureSpads, uint8_t *VhvSettings, uint8_t *PhaseCal);
uint16_t distance_in_range(I2C_HandleTypeDef *hi2c1);
