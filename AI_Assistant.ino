#include "wifiTask.h"
#include "ledTask.h"
#include "config.h"
#include "global.h"
#include "httpTask.h"
#include "stackCheckTask.h"
#include "ledButton.h"
#include "camera.h"

#include <ArduinoJson.h>


void setup() {
  Serial.begin(115200);

  cameraInit();

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
    ledTask,
    "ledTask",
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
    6144,
    NULL,
    2,
    &httpTaskHandle,
    1
  );
}


void loop() {}
