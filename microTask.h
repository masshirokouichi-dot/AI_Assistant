#ifndef MICRO_TASK_H
#define MICRO_TASK_H

#include <Arduino.h>
#include <ESP_I2S.h>

void microInit();
void microTask(void* pvParameters);

#endif 