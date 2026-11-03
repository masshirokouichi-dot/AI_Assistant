#include "ledTask.h"
#include "config.h"
#include "global.h"


void ledTask(void* pvParameters) {
  bool recording = false;
  uint32_t recordingStartMs = 0;
  TickType_t waitTicks = portMAX_DELAY;

  while (1) {
    uint32_t status;

    if (xTaskNotifyWait(
          0,
          0,
          &status,
          waitTicks
        ) == pdTRUE) {
      recording = false;
      waitTicks = portMAX_DELAY;

      switch (status) {
        case LED_RECORDING:
          recording = true;
          recordingStartMs = millis();
          waitTicks = pdMS_TO_TICKS(20);
          break;

        case HTTP_WAITING:
          rgbLedWrite(RGBLedPin, 0, 0, 255);  // 青
          break;

        case HTTP_SUCCESS:
          rgbLedWrite(RGBLedPin, 0, 255, 0);  // 緑
          waitTicks = pdMS_TO_TICKS(1000);
          break;

        case HTTP_WARNING:
          rgbLedWrite(RGBLedPin, 255, 255, 0);  // 黄
          waitTicks = pdMS_TO_TICKS(3000);
          break;

        default:
          rgbLedWrite(RGBLedPin, 255, 0, 0);  // 赤
          waitTicks = pdMS_TO_TICKS(3000);
          break;
      }
    }
    else if (!recording) {
      // 表示時間が過ぎたら消灯する。
      rgbLedWrite(RGBLedPin, 0, 0, 0);
      waitTicks = portMAX_DELAY;
    }

    if (recording) {
      // 0%から100%まで1秒かけて明るくし、次の1秒で0%に戻す。
      const uint32_t phase =
          (millis() - recordingStartMs) % 2000;
      const uint32_t ramp =
          phase <= 1000 ? phase : 2000 - phase;
      const uint8_t brightness =
          (255 * ramp) / 1000;

      rgbLedWrite(RGBLedPin, brightness, 0, brightness);
    }
  }
}
