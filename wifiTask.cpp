#include "wifiTask.h"
#include "config.h"

void wifiTask(void* pvParameters) {
  Serial.println("Connecting WiFi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    vTaskDelay(pdMS_TO_TICKS(1000));
  }

  Serial.println();
  Serial.println("Connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());


  while (1) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Reconnecting...");
      WiFi.reconnect();
    }

    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}
