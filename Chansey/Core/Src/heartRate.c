/**
 * @file heartRate.c
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
 * This is an implementation of Maxim's PBA (Penpheral Beat Amplitude) algorithm.
 * It's been converted to work with STM32 microcontrollers.
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

#include "heartRate.h"
#include <string.h>
#include "stm32l4xx_hal.h"

/**
 * @brief Structure to hold heart rate detection state
 */
typedef struct {
    int16_t IR_AC_Max;
    int16_t IR_AC_Min;
    int16_t IR_AC_Signal_Current;
    int16_t IR_AC_Signal_Previous;
    int16_t IR_AC_Signal_min;
    int16_t IR_AC_Signal_max;
    int16_t IR_Average_Estimated;
    int16_t positiveEdge;
    int16_t negativeEdge;
    int32_t ir_avg_reg;
    int16_t cbuf[32];
    uint8_t offset;
} heart_rate_state_t;

/* FIR filter coefficients */
static const uint16_t FIRCoeffs[12] = {172, 321, 579, 927, 1360, 1858, 2390, 2916, 3391, 3768, 4012, 4096};

/* Global state for heart rate detection */
static heart_rate_state_t hrState;

/**
 * @brief Initialize the heart rate detection
 * 
 * This function initializes the internal state for heart rate detection.
 * Must be called before using checkForBeat.
 */
void heartRate_init(void)
{
    /* Initialize state variables with default values */
    hrState.IR_AC_Max = 20;
    hrState.IR_AC_Min = -20;
    hrState.IR_AC_Signal_Current = 0;
    hrState.IR_AC_Signal_Previous = 0;
    hrState.IR_AC_Signal_min = 0;
    hrState.IR_AC_Signal_max = 0;
    hrState.IR_Average_Estimated = 0;
    hrState.positiveEdge = 0;
    hrState.negativeEdge = 0;
    hrState.ir_avg_reg = 0;
    hrState.offset = 0;
    
    /* Clear circular buffer */
    memset(hrState.cbuf, 0, sizeof(hrState.cbuf));
}

/**
 * @brief Check for heartbeat in IR sample
 * 
 * Heart Rate Monitor functions takes a sample value
 * Returns true if a beat is detected.
 * A running average of four samples is recommended for display.
 * 
 * @param sample IR sample to analyze
 * @return true if a heartbeat was detected
 * @return false if no heartbeat was detected
 */
bool checkForBeat(int32_t sample)
{
    bool beatDetected = false;

    /* Save current state */
    hrState.IR_AC_Signal_Previous = hrState.IR_AC_Signal_Current;
  
    /* Process next data sample */
    hrState.IR_Average_Estimated = averageDCEstimator(&hrState.ir_avg_reg, sample);
    hrState.IR_AC_Signal_Current = lowPassFIRFilter(sample - hrState.IR_Average_Estimated);

    /* Detect positive zero crossing (rising edge) */
    if ((hrState.IR_AC_Signal_Previous < 0) && (hrState.IR_AC_Signal_Current >= 0))
    {
        hrState.IR_AC_Max = hrState.IR_AC_Signal_max; /* Adjust our AC max and min */
        hrState.IR_AC_Min = hrState.IR_AC_Signal_min;

        hrState.positiveEdge = 1;
        hrState.negativeEdge = 0;
        hrState.IR_AC_Signal_max = 0;

        /* Check if we detected a heart beat */
        if ((hrState.IR_AC_Max - hrState.IR_AC_Min) > 20 && (hrState.IR_AC_Max - hrState.IR_AC_Min) < 1000)
        {
            /* Heart beat detected! */
            beatDetected = true;
        }
    }

    /* Detect negative zero crossing (falling edge) */
    if ((hrState.IR_AC_Signal_Previous > 0) && (hrState.IR_AC_Signal_Current <= 0))
    {
        hrState.positiveEdge = 0;
        hrState.negativeEdge = 1;
        hrState.IR_AC_Signal_min = 0;
    }

    /* Find Maximum value in positive cycle */
    if (hrState.positiveEdge && (hrState.IR_AC_Signal_Current > hrState.IR_AC_Signal_Previous))
    {
        hrState.IR_AC_Signal_max = hrState.IR_AC_Signal_Current;
    }

    /* Find Minimum value in negative cycle */
    if (hrState.negativeEdge && (hrState.IR_AC_Signal_Current < hrState.IR_AC_Signal_Previous))
    {
        hrState.IR_AC_Signal_min = hrState.IR_AC_Signal_Current;
    }
  
    return beatDetected;
}

/**
 * @brief Average DC Estimator
 * 
 * Estimates the DC component of the signal
 * 
 * @param p Pointer to the previous DC value
 * @param x New sample
 * @return int16_t New DC value
 */
int16_t averageDCEstimator(int32_t *p, uint16_t x)
{
    *p += ((((long)x << 15) - *p) >> 4);
    return (*p >> 15);
}

/**
 * @brief Low Pass FIR Filter
 * 
 * Applies a low-pass FIR filter to the input signal
 * 
 * @param din Input sample
 * @return int16_t Filtered output
 */
int16_t lowPassFIRFilter(int16_t din)
{  
    hrState.cbuf[hrState.offset] = din;

    int32_t z = mul16(FIRCoeffs[11], hrState.cbuf[(hrState.offset - 11) & 0x1F]);
  
    for (uint8_t i = 0; i < 11; i++)
    {
        z += mul16(FIRCoeffs[i], hrState.cbuf[(hrState.offset - i) & 0x1F] + 
                  hrState.cbuf[(hrState.offset - 22 + i) & 0x1F]);
    }

    hrState.offset++;
    hrState.offset %= 32; /* Wrap condition */

    return (z >> 15);
}

/**
 * @brief Integer multiplier
 * 
 * Performs 16-bit integer multiplication
 * 
 * @param x First operand
 * @param y Second operand
 * @return int32_t Product of x and y
 */
int32_t mul16(int16_t x, int16_t y)
{
    return ((int32_t)x * (int32_t)y);
}
