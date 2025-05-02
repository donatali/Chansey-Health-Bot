#include "bodytemp.h"

float getBodyTemp(I2C_HandleTypeDef *hi2c1){
	char buffer[2] = {0};
	float temperature = 0.0;
	float total = 0.0;
	int i = 0;
	while(i < 30){
		HAL_I2C_Mem_Read(hi2c1, 0xB4, 0x07, 1, (uint8_t*)buffer, 2, 100);
		int raw = (((buffer[1] << 8) | buffer[0]));
		temperature = (raw*0.02 - 273.15)*(9.0/5.0) + 32.0 + 2.2; //conversion
		//if(temperature >= 80){
			i++;
			total += temperature;
		//}
		//temperature may need some calibration
	}

	  return total/30;
}

uint8_t temp_ready(I2C_HandleTypeDef *hi2c1){
	if(HAL_I2C_IsDeviceReady(hi2c1, 0xB5, 1, 100) == HAL_OK){
		 return 1;
	}
	return 0;
}
