#ifndef STACK_CHECK_TASK_H
#define STACK_CHECK_TASK_H

#include <Arduino.h>

void IRAM_ATTR stackCheckButton();
void stackCheckTask(void* pvParameters);

#endif