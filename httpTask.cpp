#include "global.h"
#include "config.h"
#include "camera.h"
#include "httpTask.h"

void IRAM_ATTR startButton() {
  unsigned long currentTime = millis();

  if (currentTime - lastInterruptTime > debounceInterval) {
    BaseType_t higherPriorityTaskWoken = pdFALSE;

    vTaskNotifyGiveFromISR(
      httpTaskHandle,
      &higherPriorityTaskWoken
    );

    portYIELD_FROM_ISR(higherPriorityTaskWoken);
  }

  lastInterruptTime = currentTime;
}


void httpTask(void* pvParameters) {
  while (1) {
    ulTaskNotifyTake(
      pdTRUE,
      portMAX_DELAY
    );
    bool accepted = false;
    portENTER_CRITICAL(&audioStateMux);
    if (audioState == AudioState::IDLE) {
      audioState = AudioState::HTTP;
      accepted = true;
    }
    portEXIT_CRITICAL(&audioStateMux);
    if (!accepted) {
      continue;
    }
  
    camera_fb_t *image = esp_camera_fb_get();
    if (image == nullptr) {
      Serial.println("Camera capture failed");
      error();
      continue;
    }
    Serial.printf(
      "Photo: %dx%d, %d bytes\n",
      image->width,
      image->height,
      image->len
    );

    if (WiFi.status() == WL_CONNECTED) {
      WiFiClient client;
      // client.setInsecure();
      HTTPClient http;

      http.begin(
        client,
        "http://192.168.137.1:8080/askGemini"
      );
      http.setTimeout(60000);

    // switch(model) {
    //   case AIModel::GEMINI: 
    //     // http.addHeader(
    //     //   "x-goog-api-key",
    //     //   getAIConfig(model).apiKey
    //     // );
    //     break;
    //   case AIModel::DEEPSEEK:
    //     http.addHeader(
    //       "Authorization",
    //       String("Bearer ") + getAIConfig(model).apiKey
    //     );
    //     break;
    //   default: break;
    // }

      http.addHeader("Content-Type", "image/jpeg");
      
      // String body = R"({
      //   "text": "暂时不把图片装入json, 防止消耗太多token。测试语音tts和扬声器功能, 随便为tts模型说点话。",
      //   "image": ")" + imageBase64 + R"("
      // })";

      notifyToLed(HTTP_WAITING);
      int httpCode = http.POST(image->buf, image->len);

      esp_camera_fb_return(image);
      image = nullptr;
  
      if (httpCode == 200) {
        Serial.printf("HTTPS Code: %d\n", httpCode);
        size_t contentLength = (size_t)http.getSize();

        if (contentLength <= 0) {
          Serial.println("Invalid audio Content-Length");
          error();
          http.end();
          continue;
        }
        safeFree((void**)&pcmData);
        pcmData = (char*)ps_malloc(contentLength);

        if (pcmData == nullptr) {
          Serial.printf("PSRAM allocation failed: %d bytes\n", contentLength);

          error();
          http.end();

          portENTER_CRITICAL(&audioStateMux);
          audioState = AudioState::IDLE;
          portEXIT_CRITICAL(&audioStateMux);

          continue;
        }


        WiFiClient *stream = http.getStreamPtr();
        size_t totalRead = 0;
        unsigned long lastDataTime = millis();

        while (totalRead < contentLength) {
          size_t availableBytes = stream->available();

          if (availableBytes > 0) {
            size_t remaining = contentLength - totalRead;
            size_t readSize = min(availableBytes, remaining);

            int bytesRead = stream->readBytes(
              (uint8_t*)pcmData + totalRead,
              readSize
            );

            if (bytesRead > 0) {
              totalRead += (size_t)bytesRead;
              lastDataTime = millis();
            }
          }
          else {
            // 服务器断开且已经没有数据
            if (!stream->connected()) {
              break;
            }

            // 防止网络异常时永久卡住
            if (millis() - lastDataTime > 10000) {
              Serial.println("Audio receive timeout");
              break;
            }

            vTaskDelay(pdMS_TO_TICKS(1));
          }
        }

        if (totalRead != contentLength) {
          Serial.printf(
            "Incomplete audio: received %u / %d bytes\n",
            (unsigned int)totalRead,
            contentLength
          );

          safeFree((void**)&pcmData);
          pcmDataLength = 0;
          http.end();
          error();
          continue;
        }

        pcmDataLength = totalRead;

        Serial.printf(
          "PCM received: %u bytes\n",
          (unsigned int)pcmDataLength
        );
        
        notifyToLed(HTTP_SUCCESS);
        portENTER_CRITICAL(&audioStateMux);
        audioState = AudioState::PLAYING;
        portEXIT_CRITICAL(&audioStateMux);
        xTaskNotifyGive(playerTaskHandle);
      } 
      else {
        notifyToLed(HTTP_FAILED);
        Serial.printf("Failed: %s\n", http.errorToString(httpCode).c_str());
      }

      http.end();
    }
  }
}

void error() {
  notifyToLed(HTTP_FAILED);
  portENTER_CRITICAL(&audioStateMux);
  audioState = AudioState::IDLE;
  portEXIT_CRITICAL(&audioStateMux);
}


void notifyToLed(uint32_t status) {
  xTaskNotify(
    ledTaskHandle,
    status,
    eSetValueWithOverwrite
  );
}