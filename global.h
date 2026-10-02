#ifndef GLOBAL_H
#define GLOBAL_H

#include <Arduino.h>

extern TaskHandle_t ledTaskHandle;
extern TaskHandle_t wifiTaskHandle;
extern TaskHandle_t httpTaskHandle;
extern TaskHandle_t stackCheckHandle;
extern TaskHandle_t playerTaskHandle;
extern TaskHandle_t microTaskHandle;

extern volatile unsigned long lastInterruptTime;
extern const unsigned long debounceInterval;

extern char *pcmData;
extern size_t pcmDataLength;


extern void safeFree(void **ptr);

enum HttpStatus: uint32_t{
  HTTP_WAITING = 0,
  HTTP_SUCCESS = 1,
  HTTP_WARNING = 2,
  HTTP_FAILED = 3
};
enum class AudioState : uint8_t {
  IDLE,
  HTTP,
  RECORDING,
  PLAYING
};
extern AudioState audioState;
extern portMUX_TYPE audioStateMux;
#endif

// enum class AIModel {
//   HTTPBIN,
//   GEMINI,
//   DEEPSEEK,
//   PC
// };

// struct AIConfig {
//   const char* url;
//   const char* apiKey;
// };
