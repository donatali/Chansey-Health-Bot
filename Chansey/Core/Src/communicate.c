#include "communicate.h"

void sendToApp(UART_HandleTypeDef *huart1, float *user_stats){
	uint8_t buffer[sizeof(float)*4];
	memcpy(buffer, user_stats, sizeof(buffer));
	HAL_UART_Transmit(huart1, buffer, sizeof(buffer), 100);
}
