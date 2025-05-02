/**
 * @file heartRate.h
 * @brief Optical Heart Rate Detection (PBA Algorithm) for STM32
 *
 * Converted from the SparkFun MAX3010x library to pure C for STM32 HAL
 * Original by Nathan Seidle, SparkFun Electronics (October 2nd, 2016)
 * 
 * Given a series of IR samples from the MAX30105 we discern when a heart beat is occurring
 *
 * Let's have a brief chat about what this code does. We're going to try to detect
 * heart-rate optically. This is tricky and prone to give false readings. We really don't
 * want to get anyone hurt so use this code only as an example of how to process optical
 * data. Build fun stuff with our MAX30105 breakout board but don't use it for actual
 * medical diagnosis.
 *
 * Excellent background on optical heart rate detection:
 * http://www.ti.com/lit/an/slaa655/slaa655.pdf
 *
 * Good reading:
 * http://www.techforfuture.nl/fjc_documents/mitrabaratchi-measuringheartratewithopticalsensor.pdf
 * https://fruct.org/publications/fruct13/files/Lau.pdf
 *
 * This is an implementation of Maxim's PBA (Penpheral Beat Amplitude) algorithm. It's been 
 * converted to work with STM32 microcontrollers.
 */

/* Copyright (C) 2016 Maxim Integrated Products, Inc., All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
 * OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of Maxim Integrated
 * Products, Inc. shall not be used except as stated in the Maxim Integrated
 * Products, Inc. Branding Policy.
 *
 * The mere transfer of this software does not imply any licenses
 * of trade secrets, proprietary technology, copyrights, patents,
 * trademarks, maskwork rights, or any other form of intellectual
 * property whatsoever. Maxim Integrated Products, Inc. retains all
 * ownership rights.
 */

#ifndef HEART_RATE_H
#define HEART_RATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "stm32l4xx_hal.h"
#include "display.h"

/**
 * @brief Check if a heartbeat occurred in the IR sample
 * 
 * @param sample IR sample to analyze
 * @return true if a heartbeat was detected
 * @return false if no heartbeat was detected
 */
bool checkForBeat(int32_t sample);

/**
 * @brief Average DC estimator
 * 
 * @param p Pointer to the previous DC value
 * @param x New sample
 * @return int16_t New DC value
 */
int16_t averageDCEstimator(int32_t *p, uint16_t x);

/**
 * @brief Low-pass FIR filter
 * 
 * @param din Input sample
 * @return int16_t Filtered output
 */
int16_t lowPassFIRFilter(int16_t din);

/**
 * @brief Optimized 16-bit multiplication
 * 
 * @param x First operand
 * @param y Second operand
 * @return int32_t Product of x and y
 */
int32_t mul16(int16_t x, int16_t y);

#ifdef __cplusplus
}
#endif

#endif /* HEART_RATE_H */
