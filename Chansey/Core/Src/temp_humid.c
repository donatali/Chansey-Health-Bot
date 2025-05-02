#include "temp_humid.h"
// Convert raw temperature and humidity values into physical quantities
void convert_data(uint16_t raw_temp, uint16_t raw_humidity, float *temperature, float *humidity) {
    // Temperature conversion (in °C)
    *temperature = -49 + 315 * (float)raw_temp / ((1 << 16) - 1);

    // Humidity conversion (% RH)
    *humidity = 100 * (float)raw_humidity / ((1 << 16) - 1);
}

// Function to read sensor data (temperature and humidity) in Single Shot Mode
HAL_StatusTypeDef read_sensor_data(I2C_HandleTypeDef *hi2c, uint16_t command, uint16_t *temperature, uint16_t *humidity) {

	HAL_StatusTypeDef ret;
	uint8_t data[6];  // Buffer to store 6 data bytes (temperature, humidity, CRC)

    // Send the measurement command (16-bit)
    uint8_t command_data[2] = {command >> 8, command & 0xFF};
    ret = HAL_I2C_Master_Transmit(hi2c, SENSOR_I2C_ADDRESS << 1, command_data, 2, HAL_MAX_DELAY);
    if (ret != HAL_OK){
    	//strcpy( (char*)command_data , "ERROR Tx\r\n");
    	//printf( "ERROR Tx \r\n" );
    }

    else{
        // Wait for the measurement to complete
        HAL_Delay(100);  // Adjust delay based on measurement time for your command
        // Start read transfer and get all 6 bytes
           ret = HAL_I2C_Master_Receive(hi2c, SENSOR_I2C_ADDRESS << 1, data, 6, HAL_MAX_DELAY);

           if (ret != HAL_OK){
        	   //strcpy( (char*)data , "ERROR Rx\r\n");
        	   //printf("ERROR Rx \r\n");
           }

           else{
        	   // Extract temperature and humidity while skipping CRC bytes
			   *temperature = (data[0] << 8) | data[1];  // First 2 bytes are temperature
			   *humidity = (data[3] << 8) | data[4];     // Skip 1 CRC byte, use next 2 bytes for humidity

           }

    }

    // Ignore CRC, no need to process it, just discard the next 2 bytes (CRC for temp and humidity)
    // No need to read additional CRC bytes if you don't need them

    return ret;  // Successful read without processing CRC
}

//runs the temphumidity sensor and returns 1 if succeeds
uint8_t temp_humid(float *buffer, I2C_HandleTypeDef *hi2c1){
	uint16_t temperature_raw, humidity_raw;
	float temperature, humidity;


	if (read_sensor_data(hi2c1, 0x2400, &temperature_raw, &humidity_raw) == HAL_OK) {
	  // Convert the raw values into physical units
	  convert_data(temperature_raw, humidity_raw, &temperature, &humidity);

	  buffer[0] = temperature;
	  buffer[1] = humidity;

	  // Print out the results (or send them to a display)
//	  printf("Temperature: %.2f°F \r\n", temperature);
//	  printf("Humidity: %.2f%% RH \r\n", humidity);
	  return 1;
	} else {
	  // Handle read error
		return 0;
	}


}
