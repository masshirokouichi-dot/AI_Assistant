#include "stackCheckTask.h"
#include "global.h"

void IRAM_ATTR stackCheckButton() {
  unsigned long currentTime = millis();

  if (currentTime - lastInterruptTime > 100) {
    BaseType_t higherPriorityTaskWoken = pdFALSE;

    vTaskNotifyGiveFromISR(
      stackCheckHandle,
      &higherPriorityTaskWoken
    );

    portYIELD_FROM_ISR(higherPriorityTaskWoken);
  }

  lastInterruptTime = currentTime;
}

void stackCheckTask(void* pvParameters) {
  while(1) {
    ulTaskNotifyTake(
      pdTRUE,
      portMAX_DELAY
    );
    
    Serial.printf(
      "ledTask stack left: %u\n", 
      uxTaskGetStackHighWaterMark(ledTaskHandle)
    );
    Serial.printf(
      "wifiTask stack left: %u\n", 
      uxTaskGetStackHighWaterMark(wifiTaskHandle)
    );
    Serial.printf(
      "httpTask stack left: %u\n", 
      uxTaskGetStackHighWaterMark(httpTaskHandle)
    );

    Serial.printf(
      "stackCheck stack left: %u\n", 
      uxTaskGetStackHighWaterMark(NULL)
    );
  }
}