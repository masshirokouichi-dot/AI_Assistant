#include "global.h"
#include "config.h"
#include "camera.h"
#include "httpTask.h"


static bool notifyNext = true;

void IRAM_ATTR startButton() {
  unsigned long currentTime = millis();

  if (currentTime - lastInterruptTime > debounceInterval) {
    BaseType_t higherPriorityTaskWoken = pdFALSE;

    TaskHandle_t target = notifyNext
      ? httpTaskHandle
      : microTaskHandle;

    vTaskNotifyGiveFromISR(
      target,
      &higherPriorityTaskWoken
    );

    notifyNext = !notifyNext;


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
      notifyNext = true;
      error();
      continue;
    }
    Serial.printf(
      "Photo: %dx%d, %d bytes\n",
      image->width,
      image->height,
      image->len
    );
    
    xTaskNotifyGive(microTaskHandle);
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    if (WiFi.status() == WL_CONNECTED) {
      WiFiClient client;
      // client.setInsecure();
      HTTPClient http;

      http.begin(
        client,
        "http://192.168.137.1:8080/askGemini"
      );
      http.setTimeout(60000);

      notifyToLed(HTTP_WAITING);
      
      const String boundary = "Esp32Boundary7e43a9";
      String imageHeader =
          "--" + boundary + "\r\n"
          "Content-Disposition: form-data; name=\"image\"; "
          "filename=\"photo.jpg\"\r\n"
          "Content-Type: image/jpeg\r\n\r\n";
      String audioHeader =
          "\r\n--" + boundary + "\r\n"
          "Content-Disposition: form-data; name=\"audio\"; "
          "filename=\"recording.pcm\"\r\n"
          "Content-Type: application/octet-stream\r\n\r\n";
      String ending = "\r\n--" + boundary + "--\r\n";
      size_t length =
          imageHeader.length() + image->len +
          audioHeader.length() + pcmDataLength +
          ending.length();

      uint8_t* body = (uint8_t*)ps_malloc(length);
      if (!body) {
        Serial.println("Upload buffer allocation failed");
        continue;
      }

      size_t offset = 0;
      // 将每一段依次复制到请求体中
      auto append = [&](const void* data, size_t size) {
          memcpy(body + offset, data, size);
          offset += size;
      };

      append(imageHeader.c_str(), imageHeader.length());
      append(image->buf, image->len);
      append(audioHeader.c_str(), audioHeader.length());
      append(pcmData, pcmDataLength);
      append(ending.c_str(), ending.length());

      http.addHeader(
          "Content-Type",
          "multipart/form-data; boundary=" + boundary
      );

      int httpCode = http.POST(body, length);

      free(body);
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
        if (httpCode > 0) {
          Serial.printf("HTTP status: %d\n", httpCode);
          Serial.println(http.getString());
        }
        else {
          Serial.printf(
            "Request failed: %s\n",
            http.errorToString(httpCode).c_str()
          );
        }
        error();
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