#include "max30105.h"
#include "heartRate.h"
#include "stdio.h"
#include "stm32l4xx_hal.h"

extern I2C_HandleTypeDef hi2c1;
max30105_t particleSensor;

#define RATE_SIZE 4 //Increase this for more averaging. 4 is good.
uint8_t rates[RATE_SIZE]; //Array of heart rates
uint8_t rateSpot = 0;
uint32_t lastBeat = 0; //Time at which the last beat occurred

float beatsPerMinute;
int beatAvg;

void heart_rate_init(void)
{
  //Serial.begin(115200);
  printf("Initializing...\r\n");

  // Initialize sensor
  if (!max30105_init(&particleSensor, &hi2c1))  
  {
    printf("MAX30105 was not found. Please check wiring/power. ");
//    while (1);
  }
  printf("Place your index finger on the sensor with steady pressure.\r\n");

  max30105_setup(&particleSensor, 0x1F, 4, 2, 400, 411, 4096);
  max30105_setPulseAmplitudeRed(&particleSensor, 0x0A);
  max30105_setPulseAmplitudeGreen(&particleSensor, 0);
}

int heart_rate_task(void)
{

  uint32_t irValue = max30105_getIR(&particleSensor);
  if (checkForBeat(irValue) == true)
  {
    //We sensed a beat!
    uint32_t delta = HAL_GetTick() - lastBeat;
    lastBeat = HAL_GetTick();

    beatsPerMinute = 60 / ((float)delta / 1000.0f);

    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      rates[rateSpot++] = (uint8_t)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (uint8_t x = 0 ; x < RATE_SIZE ; x++){
          beatAvg += rates[x];
      }
      beatAvg /= RATE_SIZE;
    }
  }

  //printf("IR=%ld", irValue);
  //printf(", BPM=%.1f", beatsPerMinute);
  //printf(", Avg BPM=%d", beatAvg);
  return beatAvg;

  //if (irValue < 50000){
	  //printf("No finger?");
  //}

    //printf("\r\n");

}

int final_hr_result(void){
	  int done = 0;
	  int counter = 0;
	  int buffer[100] = {0};
	  uint32_t start_time;
	  int real_hr;
	while (done!= 2)
	  {

		   int hr = heart_rate_task();
		   if(hr != 0 && done == 0){
			   start_time = HAL_GetTick();
			   done = 1;
			   FillScreenExceptCorner(0xFFFF);

			   MeasuringText();
			   HAL_GPIO_WritePin(GPIOE, GPIO_PIN_6, 1);
		   }
//		   if(hr != 0 && ((HAL_GetTick() - start_time) % 1000 == 0)){
//			   HAL_GPIO_WritePin(GPIOE, GPIO_PIN_6, 1);
//			   HAL_Delay(30);
//			   HAL_GPIO_WritePin(GPIOE, GPIO_PIN_6, 0);
//			   HAL_Delay(70);
//		   }

		   /*if(hr == 0 && done == 1){
		   start_time = HAL_GetTick();
		   memset(buffer, 0, sizeof(buffer));  // Zero all bytes
		   }*/



		   if((HAL_GetTick() - start_time >= 9000) && done == 1){

			   printf("final heart result is = %d\r\n", hr);
			   if(counter < 100 && hr != 0){
				   buffer[counter] = hr;
				   counter++;
			   }

			   if(counter == 100){
				   printf("[");
				   double reciprocal_sum = 0.0;
				   double normal_sum = 0.0;
				   for(int i = 0; i < 100; i++){
					   printf("%d, ", buffer[i]);
					   reciprocal_sum += 1.0/buffer[i];
					   normal_sum += buffer[i];
				   }
				   printf("]\n");
				   real_hr = (int)(100/reciprocal_sum);
				   int really_hr = (int)(normal_sum/100);
				   int combo = (int)((real_hr + really_hr)/2);
				   printf("Heart rate is (harmonic): %d\n", real_hr);
				   printf("Heart rate is (arithmetic): %d\n", really_hr);

				   printf("Heart rate is (combo?): %d\n", combo);

				   done = 2;
				   //HAL_GPIO_WritePin(GPIOE, GPIO_PIN_6, 0);

			   }

		   }
	  }
	return real_hr;

}


