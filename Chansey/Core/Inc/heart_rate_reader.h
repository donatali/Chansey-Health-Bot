#ifndef HEART_RATE_READER_H
#define HEART_RATE_READER_H

#include "max30105.h"
#include "heartRate.h"
#include "stdio.h"
#include "stm32l4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

void heart_rate_init(void);
int heart_rate_task(void);
int final_hr_result(void);

#ifdef __cplusplus
}
#endif

#endif /* HEART_RATE_READER_H */
