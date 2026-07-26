#include "httpTask.h"
#include "global.h"

void httpTask(void* pvParameters) {
  while (1) {
    ulTaskNotifyTake(
      pdTRUE,
      portMAX_DELAY
    );

    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      
      http.begin("http://httpbin.org/post");

      http.addHeader(
        "Content-Type",
        "application/json"
      );
      String body = R"({
        "name": "ESP32",
        "number": "S3"
      })";

      // int httpCode = http.GET();
      int httpCode = http.POST(body);

      if (httpCode > 0) {
        Serial.printf("HTTP Code: %d\n", httpCode);

        String response = http.getString();

        Serial.println(response);
      } else {
        Serial.printf("Failed: %s\n", http.errorToString(httpCode).c_str());
      }

      http.end();
    }
  }
}

