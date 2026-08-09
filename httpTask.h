#ifndef HTTP_TASK_H
#define HTTP_TASK_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WifiClientSecure.h>

void httpTask(void* pvParameters);
void notifyToLed(uint32_t status);
const AIConfig& getAIConfig(AIModel Model);

#endif 