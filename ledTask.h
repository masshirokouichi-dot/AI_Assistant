#ifndef LED_TASK_H
#define LED_TASK_H

#include <Arduino.h>

void IRAM_ATTR ledButton();
void ledTask(void* pvParameters);


#endif