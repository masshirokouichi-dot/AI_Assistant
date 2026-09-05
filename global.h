#ifndef GLOBAL_H
#define GLOBAL_H

#include <Arduino.h>

extern TaskHandle_t ledTaskHandle;
extern TaskHandle_t wifiTaskHandle;
extern TaskHandle_t httpTaskHandle;
extern TaskHandle_t stackCheckHandle;
extern TaskHandle_t playerTaskHandle;

extern volatile unsigned long lastInterruptTime;

extern char *pcmData;
extern size_t decodedLength;


extern void safeFree(void **ptr);

enum HttpStatus: uint32_t{
  HTTP_WAITING = 0,
  HTTP_SUCCESS = 1,
  HTTP_WARNING = 2,
  HTTP_FAILED = 3
};
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
