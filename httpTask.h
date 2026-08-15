#ifndef HTTP_TASK_H
#define HTTP_TASK_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WifiClientSecure.h>
#include <esp_camera.h>
#include <base64.h>

void httpTask(void* pvParameters);
void notifyToLed(uint32_t status);
// const AIConfig& getAIConfig(AIModel Model);

#endif 