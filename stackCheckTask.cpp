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
      "playerTask stack left: %u\n", 
      uxTaskGetStackHighWaterMark(playerTaskHandle)
    );


    Serial.printf(
        "PSRAM total: %u\n",
        ESP.getPsramSize()
    );
    Serial.printf(
        "PSRAM free: %u\n",
        ESP.getFreePsram()
    );
    Serial.printf(
        "PSRAM minimum free: %u\n",
        heap_caps_get_minimum_free_size(MALLOC_CAP_SPIRAM)
    );
    Serial.printf("Free heap size: %u\n", ESP.getHeapSize());
    Serial.printf("Free min size: %u\n", ESP.getMinFreeHeap());
    Serial.printf("Free heap: %u\n", ESP.getFreeHeap());
    Serial.printf("Free PSRAM: %u\n", ESP.getFreePsram());
    Serial.printf(
        "Largest block: %u\n",
        heap_caps_get_largest_free_block(MALLOC_CAP_8BIT)
    );
    

    
    Serial.printf(
      "stackCheck stack left: %u\n", 
      uxTaskGetStackHighWaterMark(NULL)
    );

  }
}