/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "bodytemp.h"
#include "max30105.h"
#include "heart_rate_reader.h"
#include "heartRate.h"
#include "distance.h"
#include "rfid.h"
#include "temp_humid.h"
#include "stdio.h"
#include "motors.h"
#include "communicate.h"
#include "string.h"
#include "../../Drivers/VL53L0X/core/inc/vl53l0x_api.h"
#include "display.h"



/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef hlpuart1;
UART_HandleTypeDef huart1;

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi3;

TIM_HandleTypeDef htim4;

/* USER CODE BEGIN PV */
enum States {
	IDLE,
	SCAN,
	HEART,
	TEMP,
	RESULT,
	PILLS,
	DONE
};

enum Users {
	DEFAULT,
	ABRAHAM, //1
	ALEC,    //2
	ANNA,    //3
	AVA,     //4
	JAMES,   //5
	JOHN,    //6
	JOSEPH,  //7
	JUNYI,   //8
	KRIS,    //9
	MATT,    //10
	NATALIE  //11
};

const char* UsersArr[] = {
    "",
	"ABRAHAM", //1
	"ALEC",    //2
	"ANNA",    //3
	"AVA",     //4
	"JAMES",   //5
	"JOHN",    //6
	"JOSEPH",  //7
	"JUNYI",   //8
	"KRIS",    //9
	"MATT",    //10
	"NATALIE"  //11
};

VL53L0X_RangingMeasurementData_t RangingData;
VL53L0X_Dev_t vl53l0x_c;
VL53L0X_DEV Dev = &vl53l0x_c;
volatile uint8_t restock = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_LPUART1_UART_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_SPI1_Init(void);
static void MX_SPI3_Init(void);
static void MX_TIM4_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
volatile uint16_t hour = 6;
volatile uint16_t minute = 9;
volatile uint8_t second = 0;
volatile uint32_t last_minute_press = 0;
volatile uint32_t last_hour_press = 0;
char timeStr[16];

float old_buffer[2] = {00.00,00.00};

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_LPUART1_UART_Init();
  MX_USART1_UART_Init();
  MX_SPI1_Init();
  MX_SPI3_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */

  //Start timer
  HAL_TIM_Base_Start_IT(&htim4);


  enum States Chansey = IDLE;
  //IDLE STATE VARIABLES
  float buffer[2] = {0};
  uint8_t RFID_bool = 0;
  uint8_t distance_bool = 0;
  uint8_t counter = 0; //to not read temperature every single time we perform loop

  //RFID VARIABLES
  uint8_t str[MAX_LEN] = {0};
  uint8_t serNum[5] = {0};
  uint8_t R[16] = {0};
  uint8_t TrainerNum = DEFAULT;


  //HEART VARIABLES
   int heart_rate;

  //BODY TEMPERATURE VARIABLES
  float body_temperature;

  //PILL VARIABLES
  uint8_t pill_dispensed;

  // COMMUNICATION VARIABLES
	int sent = 0;


  //DISTANCE SENSOR VARIABLES
	uint32_t refSpadCount;
	uint8_t isApertureSpads;
	uint8_t VhvSettings;
	uint8_t PhaseCal;
	  Dev->I2cHandle = &hi2c1;
	  Dev->I2cDevAddr = 0x52;

	  // INITIALIZE DISPLAY
	  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_4, GPIO_PIN_SET); // CS HIGH
	  DisplayBegin();
	  FillScreen(0xFFFF);
	  DrawChanseyCorner();


	  // INITIALIZE HR
	  heart_rate_init(); // ** may have to do everytime heart is called

	  // DISPLAY VARIABLES
	  char* Msg ;
	  char* rfidName;

	  uint32_t timeOut;  // time in ms since startup
	  uint8_t  scanBool = 0;
	  uint8_t  heartBool = 0;
	  uint8_t  tempBool = 0;
	  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, 0);
	  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, 0);
	  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_6, 0);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	  //Chansey = TEMP;
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  	 HAL_Delay(10);
	  	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, 0);

	  	 printf("Chansey State: %d\r\n", Chansey);
	  	printf("Time: %02u:%02u:%02u\r\n", hour, minute, second);

	  	//sprintf(timeStr, "%02u:%02u:%02u", hour, minute, second);
		if (hour > 12) {
			//pm
			sprintf(timeStr, "%02u:%02u PM", hour-12, minute);
		}
		else {
			// am
			sprintf(timeStr, "%02u:%02u AM", hour, minute);

		}

		TimeCornerText(timeStr);

		 switch(Chansey) {
		 	 case IDLE:
		 		//turn off the speaker
		 	  	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, 0);
		 	  	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, 0);
		 	  	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_6, 0);

		 		 rfidName = "";
		 		 Msg = "";
			 	sent = 0;

		 		 RFID_bool = 0;
		 		 distance_bool = 0;
		 		 //temperature and humidity logic

		 		 memcpy(old_buffer, buffer, 2);
		 		//used to not read temperature/humidity all the time if not sensing
		 		 if(counter == 0){
		 			// get time variable

		 			if(temp_humid(buffer, &hi2c1)){
		 					printf("Temperature & Humidity: %.2f, %.2f\r\n", buffer[0], buffer[1]);
		 					IdleText(buffer[0], buffer[1]); // <-- Add this here
		 				}
		 				else{
		 					printf("Temp & Humidity not read: %.2f, %.2f\r\n", old_buffer[0], old_buffer[1]);
		 					IdleText(old_buffer[0], old_buffer[1]);
		 				}
		 				HAL_Delay(10);
		 		 }
		 		 //distance sensor logic
		 		  uint8_t buf[10] = {0xC0};
		 		  HAL_I2C_Master_Transmit(&hi2c1, VL_W_A, &buf[0], 1, 1000);
		 		  HAL_I2C_Master_Receive(&hi2c1, VL_R_A, &buf[2], 1, 1000);
		 		  SensorInit(  &refSpadCount,    &isApertureSpads,   & VhvSettings,   & PhaseCal);

		 		  VL53L0X_PerformSingleRangingMeasurement(Dev, &RangingData);
		 		  //printf("status: %u\n\r", RangingData.RangeStatus);


		 		  if (RangingData.RangeStatus==0) {
		 			  printf("Measured distance in mm: %u\n\r", RangingData.RangeMilliMeter);

		 			  if (RangingData.RangeMilliMeter == 0) {
		 				  SensorInit( &refSpadCount,    &isApertureSpads,    &VhvSettings,   & PhaseCal);
		 			  }
		 			  else{
		 				  if(RangingData.RangeMilliMeter < 200){
		 					  distance_bool = 1;
		 				  }
		 			  }
		 		  }


		 		 //RFID logic
		 		  uint8_t status = getSerNum(str);
		 		  memcpy(serNum, str, 5);

		 		 if(status == MI_OK){
		 			 RFID_bool = 1;
		 			 printf("here\n");
		 		 }

		 		 /*NEXT STATE LOGIC : if distance threshold met, move to SCAN, if RFID is scanned override and go to HEART.
		 		 (in cases where it recognizes both RFID and a person, it will not move to a redundant state)
		 		 */

		 		 if(distance_bool){
		 			HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, 0);
		 			HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, 1);
		 			 Chansey = SCAN;
				 		FillScreenExceptCorner(0xFFFF);

		 			 printf("To scan\r\n");
		 		 }
		 		 if(RFID_bool){
		 			HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, 0);
		 			HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, 1);
		 			uint16_t trainerID = returnTrainer(serNum);  // returns 0, 1, etc. to correspond to trainer num
		 			TrainerNum = trainerID;
		 			printf("%d %d %d %d %d\n\r", serNum[0], serNum[1], serNum[2], serNum[3], serNum[4]);
		 					 			printf("TRAINER ID: %d\n\r", trainerID);
		 					 			rfidName = UsersArr[trainerID];
		 					 			printf("%s\n\r", rfidName);
		 			 Chansey = HEART;
				 		FillScreenExceptCorner(0xFFFF);

		 			 printf("To heart\r\n");
		 		 }

		 		 //increase counter
		 		 counter++;
		 		 //reset counter if it goes over 255 range
		 		 if(counter > 30){
		 			 counter = 0;
		 		 }

		 		 break;
		 	 case SCAN:



		 		 if(scanBool == 0){
		 			scanBool = 1;
		 			timeOut = HAL_GetTick();
		 		 }else{
		 			uint32_t now = HAL_GetTick();
		 			 if(  (now - timeOut) > 30000){
		 				scanBool = 0;
		 				Chansey = IDLE;
		 				counter = 0;
		 				FillScreenExceptCorner(0xFFFF);
		 				break;
		 			 }
		 		 }



		 		 ScanText (); // SCAN state



	 			// rfid
		 		 uint8_t status1 = getSerNum(str);
		 		 memcpy(serNum, str, 5);

//		 		 //********************************************************************** REMOVE, FOR DEBUGGING PURPOSES
//		 		 Chansey = HEART;
//		 		 break;

		 		 if(status1 == MI_OK ){
		 			 RFID_bool = 1;
		 		 }

		 		 if(RFID_bool == 1){
		 			 Chansey = HEART;

		 			 scanBool = 0;

					FillScreenExceptCorner(0xFFFF);

		 			 //***//

		 			uint16_t trainerID = returnTrainer(serNum);  // returns 0, 1, etc. to correspond to trainer num
		 			TrainerNum = trainerID;
		 			printf("TRAINER ID: %d\n\r", trainerID);
		 			rfidName = UsersArr[trainerID];
		 			printf("%s\n\r", rfidName);
		 		 }

		 		 break;

		 	case HEART:

		 		printf("Inside heart state\r\n");


		 		if(heartBool == 0){
		 			heartBool = 1;
					timeOut = HAL_GetTick();
					HeartTempText(rfidName);

				 }else{
					uint32_t now = HAL_GetTick();
					 if(  (now - timeOut) > 30000){
						heartBool = 0;
						Chansey = IDLE;
						counter = 0;
						FillScreenExceptCorner(0xFFFF);
						break;
					 }
				 }

//				int h = heart_rate_task();
//
//				if(h != 0){
//				   printf("in hr\r\n");
//				   FillScreenExceptCorner(0xFFFF);
//				   MeasuringText();
					heart_rate = final_hr_result();
					HAL_GPIO_WritePin(GPIOE, GPIO_PIN_6, 0);
					if(heart_rate <= 40 || heart_rate >= 120){
						heart_rate = 0;

					}

					printf("final  result is = %d\r\n", heart_rate);

					heartBool = 0;
					Chansey = TEMP;
					FillScreenExceptCorner(0xFFFF);




		 		//get heart rate
		 		/*
		 		 * if heart rate > threshold then store and make heart_bool = 1
		 		 */




		 		break;
		 	case TEMP:



		 		if(tempBool == 0){
		 			tempBool = 1;
					timeOut = HAL_GetTick();
					TempText();
					HAL_Delay(5000);

				 }else{
					uint32_t now = HAL_GetTick();
					 if(  (now - timeOut) > 30000){
						 tempBool = 0;
						Chansey = IDLE;
						counter = 0;
						FillScreenExceptCorner(0xFFFF);
						break;
					 }
				 }


				if(temp_ready(&hi2c1)){
					HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, 1);
					body_temperature = getBodyTemp(&hi2c1);
					HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, 0);



					if (body_temperature <= 85 || body_temperature >= 110) {
						body_temperature = 0;
					}


					printf("body_temperature: %.2f\r\n", body_temperature);


					tempBool = 0;
					Chansey = RESULT;
			 		FillScreenExceptCorner(0xFFFF);

				}
				HAL_Delay(100);
		 		break;

		 	case RESULT:
		 		//set pill_dispenser for multivitamin; //set the variable
		 		//read rfid
		 		//block 4 == average heart rate
		 		//if body_temperature > 100.4
		 		//set pill dispenser for fever medication
		 		//if heart_rate is not within +- 10 beats of users average then -> "Abnormal heart rate reading, hospital visit recommended"
		 		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, 0);
		 		pill_dispensed = 1;
		 		if(body_temperature > 100.4){
		 			pill_dispensed = 0;
		 		}
		 		// display time

		 		if (body_temperature == 0) {
		 			TempErrorText();
		 		}
		 		if (heart_rate == 0) {
		 			HeartErrorText();
		 		}

		 		ResultText(body_temperature, heart_rate);

		 		HAL_Delay(8000);

		 		Chansey = PILLS;
		 		FillScreenExceptCorner(0xFFFF);


		 		break;
		 	case PILLS:
		 		//read pill_dispenser variable
		 		//if plll_dispenser == 1 //
		 		//release multivitamin -> TURN LEFT MOTOR

		 		// display time
		 		if(pill_dispensed == 1){
		 			printf("Dispensing Multivitamin\r\n");

		 			Msg = "Multivitamin";
		 			PillsText(Msg);
			 	 	HAL_GPIO_EXTI_Callback(GPIO_PIN_13);
		 		}

		 		else{ //release fever meds
		 			Msg = "Fever";
					PillsText(Msg);
		 			HAL_GPIO_EXTI_Callback(GPIO_PIN_12);
		 		}

				Chansey = DONE;
		 		FillScreenExceptCorner(0xFFFF);

		 		break;
		 	case DONE:

		 		DoneText(rfidName); // DONE state



		 		if (sent == 0) {
		 			float data[4] = {(float)TrainerNum, (float)heart_rate, body_temperature, (float)pill_dispensed};
		 			sendToApp(&huart1, data);
		 			sent = 1;
		 		}


		 		HAL_Delay(8000);

		 		counter = 0;

		 		// if card is there, remove. otherwise, go to idle.
		 		if(getSerNum(str) != MI_OK){
		 			Chansey = IDLE;
			 		FillScreenExceptCorner(0xFFFF);

		 		}

		 		break;
		 }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_11;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x10805D88;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief LPUART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_LPUART1_UART_Init(void)
{

  /* USER CODE BEGIN LPUART1_Init 0 */

  /* USER CODE END LPUART1_Init 0 */

  /* USER CODE BEGIN LPUART1_Init 1 */

  /* USER CODE END LPUART1_Init 1 */
  hlpuart1.Instance = LPUART1;
  hlpuart1.Init.BaudRate = 115200;
  hlpuart1.Init.WordLength = UART_WORDLENGTH_8B;
  hlpuart1.Init.StopBits = UART_STOPBITS_1;
  hlpuart1.Init.Parity = UART_PARITY_NONE;
  hlpuart1.Init.Mode = UART_MODE_TX_RX;
  hlpuart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  hlpuart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  hlpuart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  hlpuart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  hlpuart1.FifoMode = UART_FIFOMODE_DISABLE;
  if (HAL_UART_Init(&hlpuart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&hlpuart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&hlpuart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&hlpuart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LPUART1_Init 2 */

  /* USER CODE END LPUART1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief SPI3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI3_Init(void)
{

  /* USER CODE BEGIN SPI3_Init 0 */

  /* USER CODE END SPI3_Init 0 */

  /* USER CODE BEGIN SPI3_Init 1 */

  /* USER CODE END SPI3_Init 1 */
  /* SPI3 parameter configuration*/
  hspi3.Instance = SPI3;
  hspi3.Init.Mode = SPI_MODE_MASTER;
  hspi3.Init.Direction = SPI_DIRECTION_2LINES;
  hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi3.Init.NSS = SPI_NSS_SOFT;
  hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi3.Init.CRCPolynomial = 7;
  hspi3.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi3.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI3_Init 2 */

  /* USER CODE END SPI3_Init 2 */

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 47999;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 999;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  HAL_PWREx_EnableVddIO2();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2|GPIO_PIN_3|playAudio_Pin|pokeInterrupt_Pin
                          |vibrator_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_7|GPIO_PIN_8
                          |GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4|GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_1|GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10|GPIO_PIN_11, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, GPIO_PIN_RESET);

  /*Configure GPIO pins : PE2 PE3 playAudio_Pin pokeInterrupt_Pin
                           vibrator_Pin */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3|playAudio_Pin|pokeInterrupt_Pin
                          |vibrator_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PF0 PF1 PF2 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : PF4 PF5 PF7 PF8
                           PF9 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_7|GPIO_PIN_8
                          |GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : PC0 PC1 PC2 PC3
                           PC4 PC5 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG_ADC_CONTROL;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PA1 PA3 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG_ADC_CONTROL;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PA4 PA8 PA9 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PB1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG_ADC_CONTROL;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB2 PB6 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PG1 PG5 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : PE7 PE8 PE9 PE10
                           PE11 PE12 PE13 */
  GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10
                          |GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : PE14 PE15 */
  GPIO_InitStruct.Pin = GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF3_TIM1_COMP1;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : PB10 */
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB12 PB13 PB15 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF13_SAI2;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PB14 */
  GPIO_InitStruct.Pin = GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF14_TIM15;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PD8 PD9 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : PC6 */
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF13_SAI2;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PC7 */
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PC8 PC9 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PC10 PC11 */
  GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PD0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : PD2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_SDMMC1;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : PD3 PD4 PD5 PD6 */
  GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : PG12 */
  GPIO_InitStruct.Pin = GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
PUTCHAR_PROTOTYPE
{
  HAL_UART_Transmit(&hlpuart1, (uint8_t *)&ch, 1, 0xFFFF);
  return ch;
}




/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
