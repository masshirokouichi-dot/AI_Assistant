#include "global.h"

TaskHandle_t ledTaskHandle;
TaskHandle_t wifiTaskHandle;
TaskHandle_t httpTaskHandle;
TaskHandle_t stackCheckHandle;

volatile unsigned long lastInterruptTime = 0;