#include "ledTask.h"
#include "config.h"
#include "global.h"


bool ledState = false;


void IRAM_ATTR ledButton() {
  unsigned long currentTime = millis();

  if (currentTime - lastInterruptTime > 100) {
    BaseType_t higherPriorityTaskWoken = pdFALSE;

    vTaskNotifyGiveFromISR(
      ledTaskHandle,
      &higherPriorityTaskWoken
    );

    portYIELD_FROM_ISR(higherPriorityTaskWoken);
  }

  lastInterruptTime = currentTime;
}

void ledTask(void* pvParameters) {
  while (1) {
    ulTaskNotifyTake(
      pdTRUE,
      portMAX_DELAY
    );

    if (!ledState) {
      // turn LED on:
      rgbLedWrite(RGBLedPin, 0, 255, 0);
      ledState = !ledState;
    } else {
      // turn LED off:
      rgbLedWrite(RGBLedPin, 0, 0, 0);
      ledState = !ledState;
    }

    xTaskNotifyGive(httpTaskHandle);
    // taskYIELD();


  }
}