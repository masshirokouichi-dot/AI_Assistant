#include "global.h"

TaskHandle_t ledTaskHandle;
TaskHandle_t wifiTaskHandle;
TaskHandle_t httpTaskHandle;
TaskHandle_t stackCheckHandle;
TaskHandle_t playerTaskHandle;

volatile unsigned long lastInterruptTime = 0;

char *pcmData = nullptr;
size_t decodedLength = 0;

void safeFree(void **ptr) {
  free(*ptr);
  *ptr = nullptr;
}