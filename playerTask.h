#ifndef PLAYER_TASK_H
#define PLAYER_TASK_H

#include <ESP_I2S.h>
#include <Arduino.h>


void playerInit();
void playerButton();
void playerTask(void* pvParameters);
float getVolume();

#endif