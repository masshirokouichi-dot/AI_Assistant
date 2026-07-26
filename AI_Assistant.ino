#include <WiFi.h>
#include <HTTPClient.h>


constexpr int stackCheckButtonPin = 1;
constexpr int ledButtonPin = 2;
constexpr int RGBLedPin = 48;

const char* ssid = "CMCC-3fXC";
const char* password = "5hnvu3fa";

bool ledState = false;
volatile unsigned long lastInterruptTime = 0;


TaskHandle_t ledTaskHandle;
TaskHandle_t wifiTaskHandle;
TaskHandle_t httpTaskHandle;
TaskHandle_t stackCheckHandle;



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


void setup() {
  Serial.begin(115200);

  pinMode(ledButtonPin, INPUT_PULLUP);
  pinMode(stackCheckButtonPin, INPUT_PULLUP);

  attachInterrupt(ledButtonPin, ledButton, FALLING);
  attachInterrupt(stackCheckButtonPin, stackCheckButton, FALLING);


  xTaskCreatePinnedToCore(
    stackCheckTask,
    "StackCheckTask",
    3072,
    NULL,
    1,
    &stackCheckHandle,
    1
  );

  xTaskCreatePinnedToCore(
    buttonTask,
    "ButtonTask",
    2048,
    NULL,
    1,
    &ledTaskHandle,
    1
  );

  xTaskCreatePinnedToCore(
    wifiTask,
    "WiFiTask",
    4096,
    NULL,
    3,
    &wifiTaskHandle,
    0
  );

  xTaskCreatePinnedToCore(
    httpTask,
    "HttpTask",
    4096,
    NULL,
    2,
    &httpTaskHandle,
    1
  );
}


void stackCheckTask(void* pvParameters) {
  while(1) {
    ulTaskNotifyTake(
      pdTRUE,
      portMAX_DELAY
    );
    
    Serial.printf(
      "buttonTask stack left: %u\n", 
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

void buttonTask(void* pvParameters) {
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



void loop() {}
