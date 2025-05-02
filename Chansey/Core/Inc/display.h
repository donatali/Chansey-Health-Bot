#include "stdio.h"
#include <stdint.h>
#include <string.h>
#include "stm32l4xx_hal.h"



void DisplayBegin(); // set CS high with HAL_GPIO_WritePin(GPIOF, GPIO_PIN_4, GPIO_PIN_SET); and then call this

void DrawHLine(int16_t x, int16_t y0, int16_t y1, uint16_t color);

void DrawVLine(int16_t x0, int16_t x1, int16_t y, uint16_t color);

void FillScreen(uint16_t color);

void MeasuringText();

void HeartErrorText ();

void FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

void RemoveText();

void TimeCornerText(char* time);

void DrawText(int16_t x, int16_t y, char* text, uint16_t color, uint16_t bg, uint8_t size);

void ScanText (); // SCAN state

void HeartTempText(char * name); // HEART TEMP state

void PillsText(char * type); // PILLS state

void DoneText(char * name); // DONE state

void IdleText(float rTemp, float rHumidity);

void IdleTimeText(char*time);

void ResultText(float bTemp, uint8_t hRate);

void DrawBitmap (int16_t x, int16_t y, uint16_t *bitmap, uint8_t *mask, int16_t w, int16_t h);

void DrawChanseyCorner ();

void FillScreenExceptCorner(uint16_t color);

void TempText();
