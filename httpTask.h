#ifndef HTTP_TASK_H
#define HTTP_TASK_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WifiClientSecure.h>
#include <esp_camera.h>
#include <base64.h>
#include <ArduinoJson.h>
#include "mbedtls/base64.h"

void httpTask(void* pvParameters);

void notifyToLed(uint32_t status);
int process(String response, char **audioBase64);
int base64ToPcm(char *audioBase64, char **pcmData);
void safeFree(void **ptr);
// const AIConfig& getAIConfig(AIModel Model);

#endif 