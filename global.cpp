#include "global.h"

TaskHandle_t ledTaskHandle;
TaskHandle_t wifiTaskHandle;
TaskHandle_t httpTaskHandle;
TaskHandle_t stackCheckHandle;
TaskHandle_t playerTaskHandle;

volatile unsigned long lastInterruptTime = 0;
const unsigned long debounceInterval = 500;

char *pcmData = nullptr;
size_t pcmDataLength = 0;

void safeFree(void **ptr) {
  free(*ptr);
  *ptr = nullptr;
}

// bool taskState(AudioState state, AudioState thisState) {
//   portENTER_CRITICAL(&audioStateMux);
//   if (audioState == AudioState::IDLE) {
//     state = thisState;

//   }
//   portEXIT_CRITICAL(&audioStateMux);
//   return false;
// }