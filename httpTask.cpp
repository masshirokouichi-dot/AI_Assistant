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
  
    String imageBase64;
    if (cameraTake(&imageBase64) != 0) {
      error();
      continue;
    }


    if (WiFi.status() == WL_CONNECTED) {
      WiFiClient client;
      // client.setInsecure();
      HTTPClient http;

      http.begin(
        client,
        "http://192.168.137.1:8080/askGemini"
      );
      http.setTimeout(120000);

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

      http.addHeader(
        "Content-Type",
        "application/json"
      );
      
      String body = R"({
        "text": "暂时不把图片装入json, 防止消耗太多token。测试语音tts和扬声器功能, 随便为tts模型说点话。",
        "image": ")" + imageBase64 + R"("
      })";

      notifyToLed(HTTP_WAITING);
      int httpCode = http.POST(body);

  
      if (httpCode > 0) {
        if (httpCode != 200) {
          http.end();
          error();
          continue;
        }

        Serial.printf("HTTPS Code: %d\n", httpCode);
        // String response = http.getString();
        // Serial.println(response);
        // char *audioBase64 = (char *)response.c_str();
        // audioBase64 += 9;
        // char *end = audioBase64;
        // while (*end != '"') 
        //   end++;
        // *end = '\0';
        // int result = base64ToPcm(audioBase64, &pcmData);
        // if (result == -1) {
        //   safeFree((void**)&pcmData);
        //   continue;
        // }

        int contentLength = http.getSize();

        if (contentLength <= 0) {
          Serial.println("Invalid audio Content-Length");
          error();
          http.end();
          continue;
        }
        safeFree((void**)&pcmData);
        pcmData = (char*)ps_malloc((size_t)contentLength);

        if (pcmData == nullptr) {
          Serial.printf("PSRAM allocation failed: %d bytes\n", contentLength);

          notifyToLed(HTTP_FAILED);
          http.end();

          portENTER_CRITICAL(&audioStateMux);
          audioState = AudioState::IDLE;
          portEXIT_CRITICAL(&audioStateMux);

          continue;
        }


        WiFiClient *stream = http.getStreamPtr();
        size_t totalRead = 0;
        unsigned long lastDataTime = millis();

        while (totalRead < (size_t)contentLength) {
          size_t availableBytes = stream->available();

          if (availableBytes > 0) {
            size_t remaining = (size_t)contentLength - totalRead;
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



        
        notifyToLed(HTTP_SUCCESS);
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


int base64ToPcm(const char *audioBase64, char **pcmData) {
  size_t base64Length = strlen(audioBase64);
  size_t pcmLength = (base64Length / 4) * 3 + 3;
  *pcmData = (char*)ps_malloc(pcmLength);
  if (*pcmData == nullptr) {
    Serial.println("PSRAM allocation failed");
    return -1;
  }

  int result = mbedtls_base64_decode(
    (unsigned char *)*pcmData,
    pcmLength,
    &pcmDataLength,
    (const unsigned char*)audioBase64,
    base64Length
  );
  if (result != 0) {
    Serial.printf("Base64 decode failed: %d\n", result);
    return -1;
  }

  Serial.printf(
    "Base64 length: %u bytes\n",
    base64Length
  );
  Serial.printf(
    "PCM length: %u bytes\n",
    pcmDataLength
  );

  return pcmDataLength;
}


// 未完成　今のところには使う必要がない
// JSONから音声データを切り出す
int process(String *response, char **audioBase64) {
  Serial.print("response length = ");
  Serial.println(response->length());
  // BasicJsonDocument<SpiRamAllocator> doc;
  // DeserializationError error = deserializeJson(doc, *response);
  // if (error) {
  //   Serial.print("JSON parse failed: ");
  //   Serial.println(error.c_str());
  //   return -1;
  // }

  *audioBase64 = strstr(response->c_str(), "\"data\"");
  *audioBase64 += 9;

  
  // const char *data = doc["data"];
  // if (data == nullptr) {
  //   Serial.println("data failed");
  //   return -1;
  // }

  // size_t length = strlen(data);

  // *audioBase64 = (char *)ps_malloc(length + 1);
  // if (*audioBase64 == nullptr) {
  //   Serial.println("PSRAM allocation failed");
  //   return -1;
  // }

  // strcpy(*audioBase64, data);

  return 0;
}

void notifyToLed(uint32_t status) {
  xTaskNotify(
    ledTaskHandle,
    status,
    eSetValueWithOverwrite
  );
}