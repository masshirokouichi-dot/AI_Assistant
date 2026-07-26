#ifndef GLOBAL_H
#define GLOBAL_H

#include <Arduino.h>

extern TaskHandle_t ledTaskHandle;
extern TaskHandle_t wifiTaskHandle;
extern TaskHandle_t httpTaskHandle;
extern TaskHandle_t stackCheckHandle;

extern volatile unsigned long lastInterruptTime;

#endif