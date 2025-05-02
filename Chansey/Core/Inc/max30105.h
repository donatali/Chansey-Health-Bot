/**
 * @file max30105.h
 * @brief C library for MAX30105 heart rate sensor for STM32
 * 
 * Converted from the SparkFun MAX3010x library to pure C for STM32 HAL
 * Original library by Peter Jansen and Nathan Seidle (SparkFun)
 * This adaptation maintains BSD license terms of the original
 */

#ifndef MAX30105_H
#define MAX30105_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32l4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

#define MAX30105_ADDRESS          0x57 // 7-bit I2C address of the MAX30105
#define I2C_TIMEOUT_MS            100

// The number of samples to keep in the sense buffer
#define MAX30105_STORAGE_SIZE     32

typedef struct {
    uint32_t red[MAX30105_STORAGE_SIZE];
    uint32_t IR[MAX30105_STORAGE_SIZE];
    uint32_t green[MAX30105_STORAGE_SIZE];
    uint8_t head;
    uint8_t tail;
} max30105_fifo_t;

typedef struct {
    I2C_HandleTypeDef* i2c_handle;
    uint8_t _i2caddr;
    uint8_t revisionID;
    uint8_t activeLEDs; // Gets set during setup. Allows check() to calculate how many bytes to read from FIFO
    max30105_fifo_t sense;
} max30105_t;

// Status Registers
#define MAX30105_INTSTAT1         0x00
#define MAX30105_INTSTAT2         0x01
#define MAX30105_INTENABLE1       0x02
#define MAX30105_INTENABLE2       0x03

// FIFO Registers
#define MAX30105_FIFOWRITEPTR     0x04
#define MAX30105_FIFOOVERFLOW     0x05
#define MAX30105_FIFOREADPTR      0x06
#define MAX30105_FIFODATA         0x07

// Configuration Registers
#define MAX30105_FIFOCONFIG       0x08
#define MAX30105_MODECONFIG       0x09
#define MAX30105_PARTICLECONFIG   0x0A  // Referred to as SPO2 config in datasheet
#define MAX30105_LED1_PULSEAMP    0x0C
#define MAX30105_LED2_PULSEAMP    0x0D
#define MAX30105_LED3_PULSEAMP    0x0E
#define MAX30105_LED_PROX_AMP     0x10
#define MAX30105_MULTILEDCONFIG1  0x11
#define MAX30105_MULTILEDCONFIG2  0x12

// Die Temperature Registers
#define MAX30105_DIETEMPINT       0x1F
#define MAX30105_DIETEMPFRAC      0x20
#define MAX30105_DIETEMPCONFIG    0x21

// Proximity Function Registers
#define MAX30105_PROXINTTHRESH    0x30

// Part ID Registers
#define MAX30105_REVISIONID       0xFE
#define MAX30105_PARTID           0xFF

// MAX30105 Commands
// Interrupt configuration
#define MAX30105_INT_A_FULL_MASK      ((uint8_t)~0x80)
#define MAX30105_INT_A_FULL_ENABLE    0x80
#define MAX30105_INT_A_FULL_DISABLE   0x00

#define MAX30105_INT_DATA_RDY_MASK    ((uint8_t)~0x40)
#define MAX30105_INT_DATA_RDY_ENABLE  0x40
#define MAX30105_INT_DATA_RDY_DISABLE 0x00

#define MAX30105_INT_ALC_OVF_MASK     ((uint8_t)~0x20)
#define MAX30105_INT_ALC_OVF_ENABLE   0x20
#define MAX30105_INT_ALC_OVF_DISABLE  0x00

#define MAX30105_INT_PROX_INT_MASK    ((uint8_t)~0x10)
#define MAX30105_INT_PROX_INT_ENABLE  0x10
#define MAX30105_INT_PROX_INT_DISABLE 0x00

#define MAX30105_INT_DIE_TEMP_RDY_MASK    ((uint8_t)~0x02)
#define MAX30105_INT_DIE_TEMP_RDY_ENABLE  0x02
#define MAX30105_INT_DIE_TEMP_RDY_DISABLE 0x00

// Sample averaging config
#define MAX30105_SAMPLEAVG_MASK   ((uint8_t)~0xE0)
#define MAX30105_SAMPLEAVG_1      0x00
#define MAX30105_SAMPLEAVG_2      0x20
#define MAX30105_SAMPLEAVG_4      0x40
#define MAX30105_SAMPLEAVG_8      0x60
#define MAX30105_SAMPLEAVG_16     0x80
#define MAX30105_SAMPLEAVG_32     0xA0

// FIFO rollover config
#define MAX30105_ROLLOVER_MASK    0xEF
#define MAX30105_ROLLOVER_ENABLE  0x10
#define MAX30105_ROLLOVER_DISABLE 0x00

// FIFO almost full config
#define MAX30105_A_FULL_MASK      0xF0

// Mode configuration commands
#define MAX30105_SHUTDOWN_MASK    0x7F
#define MAX30105_SHUTDOWN         0x80
#define MAX30105_WAKEUP           0x00

#define MAX30105_RESET_MASK       0xBF
#define MAX30105_RESET            0x40

#define MAX30105_MODE_MASK        0xF8
#define MAX30105_MODE_REDONLY     0x02
#define MAX30105_MODE_REDIRONLY   0x03
#define MAX30105_MODE_MULTILED    0x07

// Particle sensing configuration commands
#define MAX30105_ADCRANGE_MASK    0x9F
#define MAX30105_ADCRANGE_2048    0x00
#define MAX30105_ADCRANGE_4096    0x20
#define MAX30105_ADCRANGE_8192    0x40
#define MAX30105_ADCRANGE_16384   0x60

#define MAX30105_SAMPLERATE_MASK  0xE3
#define MAX30105_SAMPLERATE_50    0x00
#define MAX30105_SAMPLERATE_100   0x04
#define MAX30105_SAMPLERATE_200   0x08
#define MAX30105_SAMPLERATE_400   0x0C
#define MAX30105_SAMPLERATE_800   0x10
#define MAX30105_SAMPLERATE_1000  0x14
#define MAX30105_SAMPLERATE_1600  0x18
#define MAX30105_SAMPLERATE_3200  0x1C

#define MAX30105_PULSEWIDTH_MASK  0xFC
#define MAX30105_PULSEWIDTH_69    0x00
#define MAX30105_PULSEWIDTH_118   0x01
#define MAX30105_PULSEWIDTH_215   0x02
#define MAX30105_PULSEWIDTH_411   0x03

// Multi-LED Mode configuration
#define MAX30105_SLOT1_MASK       0xF8
#define MAX30105_SLOT2_MASK       0x8F
#define MAX30105_SLOT3_MASK       0xF8
#define MAX30105_SLOT4_MASK       0x8F

#define SLOT_NONE                 0x00
#define SLOT_RED_LED              0x01
#define SLOT_IR_LED               0x02
#define SLOT_GREEN_LED            0x03
#define SLOT_NONE_PILOT           0x04
#define SLOT_RED_PILOT            0x05
#define SLOT_IR_PILOT             0x06
#define SLOT_GREEN_PILOT          0x07

#define MAX_30105_EXPECTEDPARTID  0x15

/* Function Prototypes */

// Initialization and configuration
bool max30105_init(max30105_t* device, I2C_HandleTypeDef* i2c_handle);
void max30105_setup(max30105_t* device, uint8_t powerLevel, uint8_t sampleAverage, uint8_t ledMode, 
                  int sampleRate, int pulseWidth, int adcRange);
void max30105_softReset(max30105_t* device);
void max30105_shutDown(max30105_t* device);
void max30105_wakeUp(max30105_t* device);

// Interrupt configuration
uint8_t max30105_getINT1(max30105_t* device);
uint8_t max30105_getINT2(max30105_t* device);
void max30105_enableAFULL(max30105_t* device);
void max30105_disableAFULL(max30105_t* device);
void max30105_enableDATARDY(max30105_t* device);
void max30105_disableDATARDY(max30105_t* device);
void max30105_enableALCOVF(max30105_t* device);
void max30105_disableALCOVF(max30105_t* device);
void max30105_enablePROXINT(max30105_t* device);
void max30105_disablePROXINT(max30105_t* device);
void max30105_enableDIETEMPRDY(max30105_t* device);
void max30105_disableDIETEMPRDY(max30105_t* device);

// LED configuration
void max30105_setLEDMode(max30105_t* device, uint8_t mode);
void max30105_setPulseAmplitudeRed(max30105_t* device, uint8_t amplitude);
void max30105_setPulseAmplitudeIR(max30105_t* device, uint8_t amplitude);
void max30105_setPulseAmplitudeGreen(max30105_t* device, uint8_t amplitude);
void max30105_setPulseAmplitudeProximity(max30105_t* device, uint8_t amplitude);
void max30105_enableSlot(max30105_t* device, uint8_t slotNumber, uint8_t device_type);
void max30105_disableSlots(max30105_t* device);

// Sensor configuration
void max30105_setADCRange(max30105_t* device, uint8_t adcRange);
void max30105_setSampleRate(max30105_t* device, uint8_t sampleRate);
void max30105_setPulseWidth(max30105_t* device, uint8_t pulseWidth);
void max30105_setProximityThreshold(max30105_t* device, uint8_t threshMSB);

// FIFO configuration and operations
void max30105_setFIFOAverage(max30105_t* device, uint8_t numberOfSamples);
void max30105_enableFIFORollover(max30105_t* device);
void max30105_disableFIFORollover(max30105_t* device);
void max30105_setFIFOAlmostFull(max30105_t* device, uint8_t numberOfSamples);
void max30105_clearFIFO(max30105_t* device);
uint8_t max30105_getWritePointer(max30105_t* device);
uint8_t max30105_getReadPointer(max30105_t* device);

// Data retrieval
uint16_t max30105_check(max30105_t* device);
bool max30105_safeCheck(max30105_t* device, uint8_t maxTimeToCheck);
uint8_t max30105_available(max30105_t* device);
void max30105_nextSample(max30105_t* device);
uint32_t max30105_getRed(max30105_t* device);
uint32_t max30105_getIR(max30105_t* device);
uint32_t max30105_getGreen(max30105_t* device);
uint32_t max30105_getFIFORed(max30105_t* device);
uint32_t max30105_getFIFOIR(max30105_t* device);
uint32_t max30105_getFIFOGreen(max30105_t* device);

// Temperature functions
float max30105_readTemperature(max30105_t* device);
float max30105_readTemperatureF(max30105_t* device);

// Device info and identification 
uint8_t max30105_readPartID(max30105_t* device);
void max30105_readRevisionID(max30105_t* device);
uint8_t max30105_getRevisionID(max30105_t* device);

// Low-level I2C communication
void max30105_bitMask(max30105_t* device, uint8_t reg, uint8_t mask, uint8_t thing);
uint8_t max30105_readRegister8(max30105_t* device, uint8_t reg);
void max30105_writeRegister8(max30105_t* device, uint8_t reg, uint8_t value);
bool max30105_readMultipleBytes(max30105_t* device, uint8_t reg, uint8_t* buffer, uint8_t length);

#ifdef __cplusplus
}
#endif

#endif /* MAX30105_H */
