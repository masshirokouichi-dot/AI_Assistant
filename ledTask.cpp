#include "ledTask.h"
#include "config.h"
#include "global.h"


// bool ledState = false;
uint32_t status;

void ledTask(void* pvParameters) {
  while (1) {
    xTaskNotifyWait(
      0,
      0,
      &status,
      portMAX_DELAY
    );

    switch (status) {
      case HTTP_WAITING:
        rgbLedWrite(RGBLedPin, 0, 0, 255);  // blue
        break;
      case HTTP_SUCCESS:
        rgbLedWrite(RGBLedPin, 0, 255, 0);  // green
        vTaskDelay(pdMS_TO_TICKS(1000));
        rgbLedWrite(RGBLedPin, 0, 0, 0);
        break;
      case HTTP_WARNING:
        rgbLedWrite(RGBLedPin, 255, 255, 0);  // yellow
        vTaskDelay(pdMS_TO_TICKS(3000));
        rgbLedWrite(RGBLedPin, 0, 0, 0);
        break;
      default:
        rgbLedWrite(RGBLedPin, 255, 0, 0);  // red
        vTaskDelay(pdMS_TO_TICKS(3000));
        rgbLedWrite(RGBLedPin, 0, 0, 0);
        break;
    }
  }
}