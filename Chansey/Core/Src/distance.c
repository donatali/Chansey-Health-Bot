#include "distance.h"
//
extern VL53L0X_RangingMeasurementData_t RangingData;
extern VL53L0X_Dev_t vl53l0x_c;
extern VL53L0X_DEV Dev;

void TogglePin(){
	  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_5, GPIO_PIN_RESET);
	  HAL_Delay(100);
	  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_5, GPIO_PIN_SET);
	  HAL_Delay(100);
}
void SensorInit(  uint32_t *refSpadCount, uint8_t *isApertureSpads, uint8_t *VhvSettings, uint8_t *PhaseCal){
	  VL53L0X_WaitDeviceBooted(Dev);
	  VL53L0X_DataInit(Dev);
	  VL53L0X_StaticInit(Dev);
	  VL53L0X_PerformRefSpadManagement(Dev, VhvSettings, PhaseCal);
	  VL53L0X_PerformRefCalibration(Dev, refSpadCount, isApertureSpads);
	  VL53L0X_SetDeviceMode(Dev, VL53L0X_DEVICEMODE_SINGLE_RANGING);
}

uint16_t distance_in_range(I2C_HandleTypeDef *hi2c1){


	Dev->I2cHandle = &hi2c1;
	Dev->I2cDevAddr = 0x52;

	uint8_t buf[10] = {0xC0};
	HAL_I2C_Master_Transmit(hi2c1, VL_W_A, &buf[0], 1, 1000);
	HAL_I2C_Master_Receive(hi2c1, VL_R_A, &buf[2], 1, 1000);



	 VL53L0X_PerformSingleRangingMeasurement(Dev, &RangingData);
	 //printf("status: %u\n\r", RangingData.RangeStatus);

	 uint8_t distance_measured = 0;
	 if (RangingData.RangeStatus==0) {
		 return RangingData.RangeMilliMeter;
		 //printf("Measured distance in mm: %u\n\r", RangingData.RangeMilliMeter);

		 if (RangingData.RangeMilliMeter == 0) {
		 // TogglePin();
//			 SensorInit(refSpadCount, isApertureSpads,   VhvSettings,    PhaseCal);
//			 VL53L0X_PerformSingleRangingMeasurement(Dev, &RangingData);
		 }

	}
	 return 1000;

}
