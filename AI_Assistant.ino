#include <WiFi.h>


constexpr int buttonPin = 2;

const char* ssid = "CMCC-3fXC";
const char* password = "5hnvu3fa";

bool ledState = false;
volatile unsigned long lastInterruptTime = 0;


SemaphoreHandle_t buttonSemaphore;
TaskHandle_t buttonTaskHandle;


void IRAM_ATTR buttonledState() {
  unsigned long currentTime = millis();

  if (currentTime - lastInterruptTime > 100) {
    xSemaphoreGiveFromISR(
      buttonSemaphore,
      NULL);
  }

  lastInterruptTime = currentTime;
}


void setup() {
  Serial.begin(115200);

  pinMode(buttonPin, INPUT_PULLUP);

  attachInterrupt(buttonPin, buttonledState, FALLING);


  buttonSemaphore = xSemaphoreCreateBinary();
  xTaskCreatePinnedToCore(
    buttonTask,
    "ButtonTask",
    2048,
    NULL,
    1,
    &buttonTaskHandle,
    1
  );

  xTaskCreatePinnedToCore(
    wifiTask,
    "WiFiTask",
    4096,
    NULL,
    2,
    NULL,
    0
  );
}


void wifiTask(void* pvParameters) {
  WiFi.begin(ssid, password);

  while (1) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Reconnecting...");
      WiFi.reconnect();
    }

    Serial.printf(
      "WiFiTask stack Left: %u\n",
      uxTaskGetStackHighWaterMark(NULL)
    );

    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}

void buttonTask(void* pvParameters) {
  while (1) {
    xSemaphoreTake(
      buttonSemaphore,
      portMAX_DELAY);

    if (!ledState) {
      // turn LED on:
      rgbLedWrite(48, 0, 255, 0);
      ledState = !ledState;
    } else {
      // turn LED off:
      rgbLedWrite(48, 0, 0, 0);
      ledState = !ledState;
    }

    // Serial.printf(
    //   "ButtonTask stack Left: %u\n",
    //   uxTaskGetStackHighWaterMark(NULL)
    // );
  }
}



void loop() {
}
