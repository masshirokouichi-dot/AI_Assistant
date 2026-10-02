#ifndef STACK_CHECK_TASK_H
#define STACK_CHECK_TASK_H

#include <Arduino.h>
#include <esp_heap_caps.h>

void stackCheckButton();
void stackCheckTask(void* pvParameters);

#endif