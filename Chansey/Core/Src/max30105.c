/**
 * @file max30105.c
 * @brief C library implementation for MAX30105 heart rate sensor for STM32
 * 
 * Converted from the SparkFun MAX3010x library to pure C for STM32 HAL
 * Original library by Peter Jansen and Nathan Seidle (SparkFun)
 * This adaptation maintains BSD license terms of the original
 */

#include "max30105.h"
#include <string.h>
#include "stm32l4xx_hal.h"

// Forward declarations for helper functions
void max30105_bitMask(max30105_t* device, uint8_t reg, uint8_t mask, uint8_t value);
uint8_t max30105_readRegister8(max30105_t* device, uint8_t reg);
void max30105_writeRegister8(max30105_t* device, uint8_t reg, uint8_t value);

/**
 * @brief Initialize the MAX30105 sensor
 * 
 * @param device Pointer to device structure
 * @param i2c_handle Pointer to I2C handle
 * @return true if initialization successful
 * @return false if initialization failed
 */
bool max30105_init(max30105_t* device, I2C_HandleTypeDef* i2c_handle) {
    // Initialize sensor data structure
    if (device == NULL || i2c_handle == NULL) {
        return false;
    }
    
    // Setup the device structure
    device->i2c_handle = i2c_handle;
    device->_i2caddr = MAX30105_ADDRESS;
    device->activeLEDs = 0;
    device->revisionID = 0;
    
    // Initialize FIFO structure
    memset(&device->sense, 0, sizeof(max30105_fifo_t));
    
    // Check if the device ID is correct
    uint8_t partID = max30105_readPartID(device);
    if (partID != MAX_30105_EXPECTEDPARTID) {
        return false;
    }
    
    // Read revision ID
    max30105_readRevisionID(device);
    
    return true;
}

/**
 * @brief Read the interrupt status register 1
 * 
 * @param device Pointer to device structure
 * @return uint8_t Interrupt status
 */
uint8_t max30105_getINT1(max30105_t* device) {
    return max30105_readRegister8(device, MAX30105_INTSTAT1);
}

/**
 * @brief Read the interrupt status register 2
 * 
 * @param device Pointer to device structure
 * @return uint8_t Interrupt status
 */
uint8_t max30105_getINT2(max30105_t* device) {
    return max30105_readRegister8(device, MAX30105_INTSTAT2);
}

/**
 * @brief Enable almost full interrupt
 * 
 * @param device Pointer to device structure
 */
void max30105_enableAFULL(max30105_t* device) {
    max30105_bitMask(device, MAX30105_INTENABLE1, MAX30105_INT_A_FULL_MASK, MAX30105_INT_A_FULL_ENABLE);
}

/**
 * @brief Disable almost full interrupt
 * 
 * @param device Pointer to device structure
 */
void max30105_disableAFULL(max30105_t* device) {
    max30105_bitMask(device, MAX30105_INTENABLE1, MAX30105_INT_A_FULL_MASK, MAX30105_INT_A_FULL_DISABLE);
}

/**
 * @brief Enable data ready interrupt
 * 
 * @param device Pointer to device structure
 */
void max30105_enableDATARDY(max30105_t* device) {
    max30105_bitMask(device, MAX30105_INTENABLE1, MAX30105_INT_DATA_RDY_MASK, MAX30105_INT_DATA_RDY_ENABLE);
}

/**
 * @brief Disable data ready interrupt
 * 
 * @param device Pointer to device structure
 */
void max30105_disableDATARDY(max30105_t* device) {
    max30105_bitMask(device, MAX30105_INTENABLE1, MAX30105_INT_DATA_RDY_MASK, MAX30105_INT_DATA_RDY_DISABLE);
}

/**
 * @brief Enable ambient light cancellation overflow interrupt
 * 
 * @param device Pointer to device structure
 */
void max30105_enableALCOVF(max30105_t* device) {
    max30105_bitMask(device, MAX30105_INTENABLE1, MAX30105_INT_ALC_OVF_MASK, MAX30105_INT_ALC_OVF_ENABLE);
}
/**
 * @brief Disable ambient light cancellation overflow interrupt
 * 
 * @param device Pointer to device structure
 */
void max30105_disableALCOVF(max30105_t* device) {
    max30105_bitMask(device, MAX30105_INTENABLE1, MAX30105_INT_ALC_OVF_MASK, MAX30105_INT_ALC_OVF_DISABLE);
}

/**
 * @brief Enable proximity interrupt
 * 
 * @param device Pointer to device structure
 */
void max30105_enablePROXINT(max30105_t* device) {
    max30105_bitMask(device, MAX30105_INTENABLE1, MAX30105_INT_PROX_INT_MASK, MAX30105_INT_PROX_INT_ENABLE);
}

/**
 * @brief Disable proximity interrupt
 * 
 * @param device Pointer to device structure
 */
void max30105_disablePROXINT(max30105_t* device) {
    max30105_bitMask(device, MAX30105_INTENABLE1, MAX30105_INT_PROX_INT_MASK, MAX30105_INT_PROX_INT_DISABLE);
}

/**
 * @brief Enable die temperature ready interrupt
 * 
 * @param device Pointer to device structure
 */
void max30105_enableDIETEMPRDY(max30105_t* device) {
    max30105_bitMask(device, MAX30105_INTENABLE2, MAX30105_INT_DIE_TEMP_RDY_MASK, MAX30105_INT_DIE_TEMP_RDY_ENABLE);
}

/**
 * @brief Disable die temperature ready interrupt
 * 
 * @param device Pointer to device structure
 */
void max30105_disableDIETEMPRDY(max30105_t* device) {
    max30105_bitMask(device, MAX30105_INTENABLE2, MAX30105_INT_DIE_TEMP_RDY_MASK, MAX30105_INT_DIE_TEMP_RDY_DISABLE);
}

/**
 * @brief Soft reset the sensor
 * 
 * @param device Pointer to device structure
 */
void max30105_softReset(max30105_t* device) {
    max30105_bitMask(device, MAX30105_MODECONFIG, MAX30105_RESET_MASK, MAX30105_RESET);

    // Poll for bit to clear, reset is then complete
    // Timeout after 100ms
    uint32_t startTime = HAL_GetTick();
    while (HAL_GetTick() - startTime < 100) {
        uint8_t response = max30105_readRegister8(device, MAX30105_MODECONFIG);
        if ((response & MAX30105_RESET) == 0) break; // Reset complete
        HAL_Delay(1); // Avoid overloading I2C bus
    }
}

/**
 * @brief Put device into shutdown mode
 * 
 * @param device Pointer to device structure
 */
void max30105_shutDown(max30105_t* device) {
    // Put IC into low power mode (datasheet pg. 19)
    // During shutdown the IC will continue to respond to I2C commands but will
    // not update with or take new readings (such as temperature)
    max30105_bitMask(device, MAX30105_MODECONFIG, MAX30105_SHUTDOWN_MASK, MAX30105_SHUTDOWN);
}

/**
 * @brief Wake up the device from shutdown mode
 * 
 * @param device Pointer to device structure
 */
void max30105_wakeUp(max30105_t* device) {
    // Pull IC out of low power mode (datasheet pg. 19)
    max30105_bitMask(device, MAX30105_MODECONFIG, MAX30105_SHUTDOWN_MASK, MAX30105_WAKEUP);
}

/**
 * @brief Set the LED mode
 * 
 * @param device Pointer to device structure
 * @param mode LED mode (MAX30105_MODE_REDONLY, MAX30105_MODE_REDIRONLY, or MAX30105_MODE_MULTILED)
 */
void max30105_setLEDMode(max30105_t* device, uint8_t mode) {
    // Set which LEDs are used for sampling -- Red only, RED+IR only, or custom.
    // See datasheet, page 19
    max30105_bitMask(device, MAX30105_MODECONFIG, MAX30105_MODE_MASK, mode);
}

/**
 * @brief Set ADC range
 * 
 * @param device Pointer to device structure
 * @param adcRange ADC range (MAX30105_ADCRANGE_2048, _4096, _8192, _16384)
 */
void max30105_setADCRange(max30105_t* device, uint8_t adcRange) {
    // adcRange: one of MAX30105_ADCRANGE_2048, _4096, _8192, _16384
    max30105_bitMask(device, MAX30105_PARTICLECONFIG, MAX30105_ADCRANGE_MASK, adcRange);
}

/**
 * @brief Set sample rate
 * 
 * @param device Pointer to device structure
 * @param sampleRate Sample rate (MAX30105_SAMPLERATE_50, _100, _200, etc.)
 */
void max30105_setSampleRate(max30105_t* device, uint8_t sampleRate) {
    // sampleRate: one of MAX30105_SAMPLERATE_50, _100, _200, _400, _800, _1000, _1600, _3200
    max30105_bitMask(device, MAX30105_PARTICLECONFIG, MAX30105_SAMPLERATE_MASK, sampleRate);
}

/**
 * @brief Set pulse width
 * 
 * @param device Pointer to device structure
 * @param pulseWidth Pulse width (MAX30105_PULSEWIDTH_69, _118, _215, _411)
 */
void max30105_setPulseWidth(max30105_t* device, uint8_t pulseWidth) {
    // pulseWidth: one of MAX30105_PULSEWIDTH_69, _188, _215, _411
    max30105_bitMask(device, MAX30105_PARTICLECONFIG, MAX30105_PULSEWIDTH_MASK, pulseWidth);
}

/**
 * @brief Set pulse amplitude for red LED
 * 
 * @param device Pointer to device structure
 * @param amplitude Amplitude (0x00 = 0mA, 0x7F = 25.4mA, 0xFF = 50mA (typical))
 */
void max30105_setPulseAmplitudeRed(max30105_t* device, uint8_t amplitude) {
    max30105_writeRegister8(device, MAX30105_LED1_PULSEAMP, amplitude);
}

/**
 * @brief Set pulse amplitude for IR LED
 * 
 * @param device Pointer to device structure
 * @param amplitude Amplitude (0x00 = 0mA, 0x7F = 25.4mA, 0xFF = 50mA (typical))
 */
void max30105_setPulseAmplitudeIR(max30105_t* device, uint8_t amplitude) {
    max30105_writeRegister8(device, MAX30105_LED2_PULSEAMP, amplitude);
}

/**
 * @brief Set pulse amplitude for green LED
 * 
 * @param device Pointer to device structure
 * @param amplitude Amplitude (0x00 = 0mA, 0x7F = 25.4mA, 0xFF = 50mA (typical))
 */
void max30105_setPulseAmplitudeGreen(max30105_t* device, uint8_t amplitude) {
    max30105_writeRegister8(device, MAX30105_LED3_PULSEAMP, amplitude);
}

/**
 * @brief Set pulse amplitude for proximity mode
 * 
 * @param device Pointer to device structure
 * @param amplitude Amplitude (0x00 = 0mA, 0x7F = 25.4mA, 0xFF = 50mA (typical))
 */
void max30105_setPulseAmplitudeProximity(max30105_t* device, uint8_t amplitude) {
    max30105_writeRegister8(device, MAX30105_LED_PROX_AMP, amplitude);
}

/**
 * @brief Set proximity threshold
 * 
 * @param device Pointer to device structure
 * @param threshMSB Threshold MSB (8 most significant bits of the ADC count)
 */
void max30105_setProximityThreshold(max30105_t* device, uint8_t threshMSB) {
    // Set the IR ADC count that will trigger the beginning of particle-sensing mode.
    // The threshMSB signifies only the 8 most significant-bits of the ADC count.
    // See datasheet, page 24.
    max30105_writeRegister8(device, MAX30105_PROXINTTHRESH, threshMSB);
}

/**
 * @brief Enable a slot for multi-LED mode
 * 
 * @param device Pointer to device structure
 * @param slotNumber Slot number (1-4)
 * @param device_type Device type (SLOT_RED_LED, SLOT_IR_LED, etc.)
 */
void max30105_enableSlot(max30105_t* device, uint8_t slotNumber, uint8_t device_type) {
    switch (slotNumber) {
        case (1):
            max30105_bitMask(device, MAX30105_MULTILEDCONFIG1, MAX30105_SLOT1_MASK, device_type);
            break;
        case (2):
            max30105_bitMask(device, MAX30105_MULTILEDCONFIG1, MAX30105_SLOT2_MASK, device_type << 4);
            break;
        case (3):
            max30105_bitMask(device, MAX30105_MULTILEDCONFIG2, MAX30105_SLOT3_MASK, device_type);
            break;
        case (4):
            max30105_bitMask(device, MAX30105_MULTILEDCONFIG2, MAX30105_SLOT4_MASK, device_type << 4);
            break;
        default:
            // Invalid slot number
            break;
    }
}

/**
 * @brief Disable all slots
 * 
 * @param device Pointer to device structure
 */
void max30105_disableSlots(max30105_t* device) {
    max30105_writeRegister8(device, MAX30105_MULTILEDCONFIG1, 0);
    max30105_writeRegister8(device, MAX30105_MULTILEDCONFIG2, 0);
}

/**
 * FIFO Configuration
 */

/**
 * @brief Set FIFO averaging
 * 
 * @param device Pointer to device structure
 * @param numberOfSamples Number of samples to average (MAX30105_SAMPLEAVG_x)
 */
void max30105_setFIFOAverage(max30105_t* device, uint8_t numberOfSamples) {
    max30105_bitMask(device, MAX30105_FIFOCONFIG, MAX30105_SAMPLEAVG_MASK, numberOfSamples);
}

/**
 * @brief Clear FIFO
 * 
 * @param device Pointer to device structure
 */
void max30105_clearFIFO(max30105_t* device) {
    max30105_writeRegister8(device, MAX30105_FIFOWRITEPTR, 0);
    max30105_writeRegister8(device, MAX30105_FIFOOVERFLOW, 0);
    max30105_writeRegister8(device, MAX30105_FIFOREADPTR, 0);
}

/**
 * @brief Enable FIFO rollover
 * 
 * @param device Pointer to device structure
 */
void max30105_enableFIFORollover(max30105_t* device) {
    max30105_bitMask(device, MAX30105_FIFOCONFIG, MAX30105_ROLLOVER_MASK, MAX30105_ROLLOVER_ENABLE);
}

/**
 * @brief Disable FIFO rollover
 * 
 * @param device Pointer to device structure
 */
void max30105_disableFIFORollover(max30105_t* device) {
    max30105_bitMask(device, MAX30105_FIFOCONFIG, MAX30105_ROLLOVER_MASK, MAX30105_ROLLOVER_DISABLE);
}

/**
 * @brief Set FIFO almost full value
 * 
 * @param device Pointer to device structure
 * @param numberOfSamples Number of samples (0x00 is 32 samples, 0x0F is 17 samples)
 */
void max30105_setFIFOAlmostFull(max30105_t* device, uint8_t numberOfSamples) {
    max30105_bitMask(device, MAX30105_FIFOCONFIG, MAX30105_A_FULL_MASK, numberOfSamples);
}
/**
 * @brief Get FIFO write pointer
 * 
 * @param device Pointer to device structure
 * @return uint8_t Write pointer value
 */
uint8_t max30105_getWritePointer(max30105_t* device) {
    return max30105_readRegister8(device, MAX30105_FIFOWRITEPTR);
}

/**
 * @brief Get FIFO read pointer
 * 
 * @param device Pointer to device structure
 * @return uint8_t Read pointer value
 */
uint8_t max30105_getReadPointer(max30105_t* device) {
    return max30105_readRegister8(device, MAX30105_FIFOREADPTR);
}

/**
 * @brief Read temperature from the sensor
 * 
 * @param device Pointer to device structure
 * @return float Temperature in Celsius
 */
float max30105_readTemperature(max30105_t* device) {
    // DIE_TEMP_RDY interrupt must be enabled
    
    // Step 1: Config die temperature register to take 1 temperature sample
    max30105_writeRegister8(device, MAX30105_DIETEMPCONFIG, 0x01);

    // Poll for bit to clear, reading is then complete
    // Timeout after 100ms
    uint32_t startTime = HAL_GetTick();
    while (HAL_GetTick() - startTime < 100) {
        // Check to see if DIE_TEMP_RDY interrupt is set
        uint8_t response = max30105_readRegister8(device, MAX30105_INTSTAT2);
        if ((response & MAX30105_INT_DIE_TEMP_RDY_ENABLE) > 0) break; // We're done!
        HAL_Delay(1); // Don't overload the I2C bus
    }
    
    // If we timed out, return error value
    if (HAL_GetTick() - startTime >= 100) return -999.0f;

    // Step 2: Read die temperature registers
    int8_t tempInt = (int8_t)max30105_readRegister8(device, MAX30105_DIETEMPINT);
    uint8_t tempFrac = max30105_readRegister8(device, MAX30105_DIETEMPFRAC); // Also clears the DIE_TEMP_RDY interrupt

    // Step 3: Calculate temperature (datasheet pg. 23)
    return (float)tempInt + ((float)tempFrac * 0.0625f);
}

/**
 * @brief Read temperature from the sensor in Fahrenheit
 * 
 * @param device Pointer to device structure 
 * @return float Temperature in Fahrenheit
 */
float max30105_readTemperatureF(max30105_t* device) {
    float temp = max30105_readTemperature(device);

    if (temp != -999.0f) {
        temp = temp * 1.8f + 32.0f;
    }

    return temp;
}

/**
 * @brief Set the PROX_INT_THRESHOLD value
 * 
 * @param device Pointer to device structure
 * @param val Threshold value
 */
void max30105_setPROXINTTHRESH(max30105_t* device, uint8_t val) {
    max30105_writeRegister8(device, MAX30105_PROXINTTHRESH, val);
}

/**
 * Device ID and Revision
 */

/**
 * @brief Read part ID register
 * 
 * @param device Pointer to device structure
 * @return uint8_t Part ID value (should be 0x15)
 */
uint8_t max30105_readPartID(max30105_t* device) {
    return max30105_readRegister8(device, MAX30105_PARTID);
}

/**
 * @brief Read and store the revision ID
 * 
 * @param device Pointer to device structure
 */
void max30105_readRevisionID(max30105_t* device) {
    device->revisionID = max30105_readRegister8(device, MAX30105_REVISIONID);
}

/**
 * @brief Get the revision ID
 * 
 * @param device Pointer to device structure
 * @return uint8_t Revision ID value
 */
uint8_t max30105_getRevisionID(max30105_t* device) {
    return device->revisionID;
}

/**
 * @brief Setup the sensor with default configuration
 * 
 * @param device Pointer to device structure
 * @param powerLevel Power level for LED
 * @param sampleAverage Number of samples to average
 * @param ledMode LED mode (1 = Red only, 2 = Red+IR, 3 = Red+IR+Green)
 * @param sampleRate Sample rate in Hz
 * @param pulseWidth Pulse width in μs
 * @param adcRange ADC range in nA
 */
void max30105_setup(max30105_t* device, uint8_t powerLevel, uint8_t sampleAverage, 
                   uint8_t ledMode, int sampleRate, int pulseWidth, int adcRange) {
    max30105_softReset(device); // Reset all configuration, threshold, and data registers to POR values

    // FIFO Configuration
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // The chip will average multiple samples of same type together if you wish
    if (sampleAverage == 1) max30105_setFIFOAverage(device, MAX30105_SAMPLEAVG_1); // No averaging per FIFO record
    else if (sampleAverage == 2) max30105_setFIFOAverage(device, MAX30105_SAMPLEAVG_2);
    else if (sampleAverage == 4) max30105_setFIFOAverage(device, MAX30105_SAMPLEAVG_4);
    else if (sampleAverage == 8) max30105_setFIFOAverage(device, MAX30105_SAMPLEAVG_8);
    else if (sampleAverage == 16) max30105_setFIFOAverage(device, MAX30105_SAMPLEAVG_16);
    else if (sampleAverage == 32) max30105_setFIFOAverage(device, MAX30105_SAMPLEAVG_32);
    else max30105_setFIFOAverage(device, MAX30105_SAMPLEAVG_4);

    // setFIFOAlmostFull(2); // Set to 30 samples to trigger an 'Almost Full' interrupt
    max30105_enableFIFORollover(device); // Allow FIFO to wrap/roll over
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    // Mode Configuration
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    if (ledMode == 3) max30105_setLEDMode(device, MAX30105_MODE_MULTILED); // Watch all three LED channels
    else if (ledMode == 2) max30105_setLEDMode(device, MAX30105_MODE_REDIRONLY); // Red and IR
    else max30105_setLEDMode(device, MAX30105_MODE_REDONLY); // Red only
    device->activeLEDs = ledMode; // Used to control how many bytes to read from FIFO buffer
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    // Particle Sensing Configuration
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    if (adcRange < 4096) max30105_setADCRange(device, MAX30105_ADCRANGE_2048); // 7.81pA per LSB
    else if (adcRange < 8192) max30105_setADCRange(device, MAX30105_ADCRANGE_4096); // 15.63pA per LSB
    else if (adcRange < 16384) max30105_setADCRange(device, MAX30105_ADCRANGE_8192); // 31.25pA per LSB
    else if (adcRange == 16384) max30105_setADCRange(device, MAX30105_ADCRANGE_16384); // 62.5pA per LSB
    else max30105_setADCRange(device, MAX30105_ADCRANGE_2048);

    if (sampleRate < 100) max30105_setSampleRate(device, MAX30105_SAMPLERATE_50); // Take 50 samples per second
    else if (sampleRate < 200) max30105_setSampleRate(device, MAX30105_SAMPLERATE_100);
    else if (sampleRate < 400) max30105_setSampleRate(device, MAX30105_SAMPLERATE_200);
    else if (sampleRate < 800) max30105_setSampleRate(device, MAX30105_SAMPLERATE_400);
    else if (sampleRate < 1000) max30105_setSampleRate(device, MAX30105_SAMPLERATE_800);
    else if (sampleRate < 1600) max30105_setSampleRate(device, MAX30105_SAMPLERATE_1000);
    else if (sampleRate < 3200) max30105_setSampleRate(device, MAX30105_SAMPLERATE_1600);
    else if (sampleRate == 3200) max30105_setSampleRate(device, MAX30105_SAMPLERATE_3200);
    else max30105_setSampleRate(device, MAX30105_SAMPLERATE_50);

    // The longer the pulse width the longer range of detection you'll have
    // At 69us and 0.4mA it's about 2 inches
    // At 411us and 0.4mA it's about 6 inches
    if (pulseWidth < 118) max30105_setPulseWidth(device, MAX30105_PULSEWIDTH_69); // 15 bit resolution
    else if (pulseWidth < 215) max30105_setPulseWidth(device, MAX30105_PULSEWIDTH_118); // 16 bit resolution
    else if (pulseWidth < 411) max30105_setPulseWidth(device, MAX30105_PULSEWIDTH_215); // 17 bit resolution
    else if (pulseWidth == 411) max30105_setPulseWidth(device, MAX30105_PULSEWIDTH_411); // 18 bit resolution
    else max30105_setPulseWidth(device, MAX30105_PULSEWIDTH_69);
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    // LED Pulse Amplitude Configuration
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // Default is 0x1F which gets us 6.4mA
    // powerLevel = 0x02, 0.4mA - Presence detection of ~4 inch
    // powerLevel = 0x1F, 6.4mA - Presence detection of ~8 inch
    // powerLevel = 0x7F, 25.4mA - Presence detection of ~8 inch
    // powerLevel = 0xFF, 50.0mA - Presence detection of ~12 inch

    max30105_setPulseAmplitudeRed(device, powerLevel);
    max30105_setPulseAmplitudeIR(device, powerLevel);
    max30105_setPulseAmplitudeGreen(device, powerLevel);
    max30105_setPulseAmplitudeProximity(device, powerLevel);
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    // Multi-LED Mode Configuration, Enable the reading of the three LEDs
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    max30105_enableSlot(device, 1, SLOT_RED_LED);
    if (ledMode > 1) max30105_enableSlot(device, 2, SLOT_IR_LED);
    if (ledMode > 2) max30105_enableSlot(device, 3, SLOT_GREEN_LED);
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    max30105_clearFIFO(device); // Reset the FIFO before we begin checking the sensor
}

/**
 * Data Collection
 */

/**
 * @brief Get number of available samples in FIFO buffer
 * 
 * @param device Pointer to device structure
 * @return uint8_t Number of samples
 */
uint8_t max30105_available(max30105_t* device)
{
    int8_t numberOfSamples = device->sense.head - device->sense.tail;
    if (numberOfSamples < 0) numberOfSamples += MAX30105_STORAGE_SIZE;

    return (uint8_t)numberOfSamples;
}

/**
 * @brief Get the most recent red value
 * 
 * @param device Pointer to device structure
 * @return uint32_t Red sensor value or 0 if no new data
 */
uint32_t max30105_getRed(max30105_t* device)
{
    // Check the sensor for new data for 250ms
    if (max30105_safeCheck(device, 250))
        return device->sense.red[device->sense.head];
    else
        return 0; // Sensor failed to find new data
}

/**
 * @brief Get the most recent IR value
 * 
 * @param device Pointer to device structure
 * @return uint32_t IR sensor value or 0 if no new data
 */
uint32_t max30105_getIR(max30105_t* device)
{
    // Check the sensor for new data for 250ms
    if (max30105_safeCheck(device, 250))
        return device->sense.IR[device->sense.head];
    else
        return 0; // Sensor failed to find new data
}

/**
 * @brief Get the most recent green value
 * 
 * @param device Pointer to device structure
 * @return uint32_t Green sensor value or 0 if no new data
 */
uint32_t max30105_getGreen(max30105_t* device)
{
    // Check the sensor for new data for 250ms
    if (max30105_safeCheck(device, 250))
        return device->sense.green[device->sense.head];
    else
        return 0; // Sensor failed to find new data
}

/**
 * @brief Get the next red value in the FIFO
 * 
 * @param device Pointer to device structure
 * @return uint32_t Red value at the tail position
 */
uint32_t max30105_getFIFORed(max30105_t* device)
{
    return device->sense.red[device->sense.tail];
}

/**
 * @brief Get the next IR value in the FIFO
 * 
 * @param device Pointer to device structure
 * @return uint32_t IR value at the tail position
 */
uint32_t max30105_getFIFOIR(max30105_t* device)
{
    return device->sense.IR[device->sense.tail];
}

/**
 * @brief Get the next green value in the FIFO
 * 
 * @param device Pointer to device structure
 * @return uint32_t Green value at the tail position
 */
uint32_t max30105_getFIFOGreen(max30105_t* device)
{
    return device->sense.green[device->sense.tail];
}

/**
 * @brief Advance the tail pointer to the next sample
 * 
 * @param device Pointer to device structure
 */
void max30105_nextSample(max30105_t* device)
{
    if (max30105_available(device)) // Only advance the tail if new data is available
    {
        device->sense.tail++;
        device->sense.tail %= MAX30105_STORAGE_SIZE; // Wrap condition
    }
}
/**
 * @brief Poll the sensor for new data
 * 
 * Call regularly. If new data is available, it updates the head and tail in the device structure.
 * 
 * @param device Pointer to device structure
 * @return uint16_t Number of new samples obtained
 */
uint16_t max30105_check(max30105_t* device)
{
    // Read register FIFO_DATA in (3-byte * number of active LED) chunks
    // Until FIFO_RD_PTR = FIFO_WR_PTR

    uint8_t readPointer = max30105_getReadPointer(device);
    uint8_t writePointer = max30105_getWritePointer(device);

    int numberOfSamples = 0;

    // Do we have new data?
    if (readPointer != writePointer)
    {
        // Calculate the number of readings we need to get from sensor
        numberOfSamples = writePointer - readPointer;
        if (numberOfSamples < 0) numberOfSamples += 32; // Wrap condition

        // Calculate bytes to read (each value is 3 bytes)
        int bytesLeftToRead = numberOfSamples * device->activeLEDs * 3;

        // Get ready to read a burst of data from the FIFO register
        // Using STM32 HAL for I2C communication
        uint8_t reg = MAX30105_FIFODATA;
        HAL_StatusTypeDef status;
        
        // Send register address to read from
        status = HAL_I2C_Master_Transmit(device->i2c_handle, device->_i2caddr << 1, &reg, 1, I2C_TIMEOUT_MS);
        if (status != HAL_OK) {
            return 0; // Error in I2C transmission
        }

        // We may need to read as many as 288 bytes
        // Read in chunks no larger than 32 bytes at a time (common I2C buffer size)
        while (bytesLeftToRead > 0)
        {
            int toGet = bytesLeftToRead;
            if (toGet > 32) {
                // If toGet is 32 this is bad because we read 6 bytes (Red+IR * 3 = 6) at a time
                // 32 % 6 = 2 left over. We don't want to request 32 bytes, we want to request 30.
                // 32 % 9 (Red+IR+GREEN) = 5 left over. We want to request 27.
                toGet = 32 - (32 % (device->activeLEDs * 3)); // Trim to be a multiple of the samples we need to read
            }

            bytesLeftToRead -= toGet;

            // Request toGet number of bytes from sensor
            uint8_t buffer[32]; // Maximum buffer size of 32 bytes
            status = HAL_I2C_Master_Receive(device->i2c_handle, device->_i2caddr << 1, buffer, toGet, I2C_TIMEOUT_MS);
            if (status != HAL_OK) {
                return 0; // Error in I2C reception
            }

            // Process the data received
            for (int i = 0; i < toGet; i += (device->activeLEDs * 3)) {
                device->sense.head++; // Advance the head of the storage struct
                device->sense.head %= MAX30105_STORAGE_SIZE; // Wrap condition

                uint32_t tempLong;

                // Burst read three bytes - RED
                tempLong = ((uint32_t)buffer[i] << 16) | ((uint32_t)buffer[i+1] << 8) | buffer[i+2];
                tempLong &= 0x3FFFF; // Zero out all but 18 bits
                device->sense.red[device->sense.head] = tempLong; // Store this reading into the sense array

                if (device->activeLEDs > 1) {
                    // Burst read three more bytes - IR
                    tempLong = ((uint32_t)buffer[i+3] << 16) | ((uint32_t)buffer[i+4] << 8) | buffer[i+5];
                    tempLong &= 0x3FFFF; // Zero out all but 18 bits
                    device->sense.IR[device->sense.head] = tempLong;
                }

                if (device->activeLEDs > 2) {
                    // Burst read three more bytes - Green
                    tempLong = ((uint32_t)buffer[i+6] << 16) | ((uint32_t)buffer[i+7] << 8) | buffer[i+8];
                    tempLong &= 0x3FFFF; // Zero out all but 18 bits
                    device->sense.green[device->sense.head] = tempLong;
                }
            }
        } // End while (bytesLeftToRead > 0)
    } // End readPtr != writePtr

    return (uint16_t)numberOfSamples; // Let the world know how much new data we found
}

/**
 * @brief Check for new data with timeout
 * 
 * @param device Pointer to device structure
 * @param maxTimeToCheck Maximum time to wait in milliseconds
 * @return true New data was found
 * @return false No new data found within timeout
 */
bool max30105_safeCheck(max30105_t* device, uint8_t maxTimeToCheck)
{
    uint32_t markTime = HAL_GetTick();
    
    while (1) {
        if (HAL_GetTick() - markTime > maxTimeToCheck) return false;

        if (max30105_check(device) > 0) { // We found new data!
            return true;
        }

        HAL_Delay(1);
    }
}

/**
 * @brief Modify specific bits in a register
 * 
 * @param device Pointer to device structure
 * @param reg Register address
 * @param mask Bit mask
 * @param value Value to set
 */
void max30105_bitMask(max30105_t* device, uint8_t reg, uint8_t mask, uint8_t value)
{
    // Grab current register context
    uint8_t originalContents = max30105_readRegister8(device, reg);

    // Zero-out the portions of the register we're interested in
    originalContents = originalContents & mask;

    // Change contents
    max30105_writeRegister8(device, reg, originalContents | value);
}

/**
 * Low-level I2C Communication
 */

/**
 * @brief Read a single byte from a register
 * 
 * @param device Pointer to device structure
 * @param reg Register address
 * @return uint8_t Register value
 */
uint8_t max30105_readRegister8(max30105_t* device, uint8_t reg)
{
    uint8_t value;
    HAL_StatusTypeDef status;
    
    // Send register address
    status = HAL_I2C_Master_Transmit(device->i2c_handle, device->_i2caddr << 1, &reg, 1, I2C_TIMEOUT_MS);
    if (status != HAL_OK) {
        return 0; // I2C error
    }
    
    // Read register value
    status = HAL_I2C_Master_Receive(device->i2c_handle, device->_i2caddr << 1, &value, 1, I2C_TIMEOUT_MS);
    if (status != HAL_OK) {
        return 0; // I2C error
    }
    
    return value;
}

/**
 * @brief Write a single byte to a register
 * 
 * @param device Pointer to device structure
 * @param reg Register address
 * @param value Value to write
 */
void max30105_writeRegister8(max30105_t* device, uint8_t reg, uint8_t value)
{
    uint8_t buffer[2];
    buffer[0] = reg;
    buffer[1] = value;
    
    HAL_StatusTypeDef status;
    status = HAL_I2C_Master_Transmit(device->i2c_handle, device->_i2caddr << 1, buffer, 2, I2C_TIMEOUT_MS);
    
    // Error handling - in a production environment, you might want to handle this error
    // more gracefully, such as setting an error flag or implementing retries
    if (status != HAL_OK) {
        // Handle I2C error - could add custom error handling here
        // For example: device->errorFlag = true;
    }
}
