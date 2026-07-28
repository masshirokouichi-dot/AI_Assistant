#include "httpTask.h"
#include "global.h"


void httpTask(void* pvParameters) {
  while (1) {
    ulTaskNotifyTake(
      pdTRUE,
      portMAX_DELAY
    );

    if (WiFi.status() == WL_CONNECTED) {
      WiFiClientSecure client;
      client.setInsecure();
      HTTPClient http;
      http.begin(
        client,
        "https://httpbin.org/post"
      );

      http.addHeader(
        "Content-Type",
        "application/json"
      );
      String body = R"({
        "name": "ESP32",
        "chip": "S3"
      })";


      notifyToLed(HTTP_WAITING);
      // int httpCode = http.GET();
      int httpCode = http.POST(body);
      

      if (httpCode > 0) {
        if (httpCode == 200) {
          notifyToLed(HTTP_SUCCESS);
        } else {
          notifyToLed(HTTP_WARNING);
        }
          Serial.printf("HTTPS Code: %d\n", httpCode);

          String response = http.getString();

          Serial.println(response);

      } else {
        notifyToLed(HTTP_FAILED);
        Serial.printf("Failed: %s\n", http.errorToString(httpCode).c_str());
      }

      http.end();
    }
  }
}

void notifyToLed(uint32_t status) {
  xTaskNotify(
    ledTaskHandle,
    status,
    eSetValueWithOverwrite
  );
}

