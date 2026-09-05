#include "wifiTask.h"
#include "ledTask.h"
#include "config.h"
#include "global.h"
#include "httpTask.h"
#include "stackCheckTask.h"
#include "camera.h"
#include "playerTask.h"

#include <ArduinoJson.h>


void setup() {
  Serial.begin(115200);

  cameraInit();
  playerInit();

  pinMode(httpButtonPin, INPUT_PULLUP);
  pinMode(stackCheckButtonPin, INPUT_PULLUP);
  pinMode(playerButtonPin, INPUT_PULLUP);

  attachInterrupt(httpButtonPin, httpButton, FALLING);
  attachInterrupt(stackCheckButtonPin, stackCheckButton, FALLING);
  attachInterrupt(playerButtonPin, playerButton, FALLING);

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
    5120,
    NULL,
    2,
    &httpTaskHandle,
    1
  );

  xTaskCreatePinnedToCore(
    playerTask,
    "PlayerTask",
    7168,
    NULL,
    2,
    &playerTaskHandle,
    1
  );
}


void loop() {}
