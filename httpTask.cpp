#include "global.h"
#include "config.h"
#include "camera.h"
#include "httpTask.h"


void httpTask(void* pvParameters) {
  while (1) {
    ulTaskNotifyTake(
      pdTRUE,
      portMAX_DELAY
    );
  
    String imageBase64;
    if (cameraTake(&imageBase64) != 0) {
      notifyToLed(HTTP_FAILED);
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
        if (httpCode == 200) {
          notifyToLed(HTTP_SUCCESS);
        } else {
          notifyToLed(HTTP_WARNING);
        }

        Serial.printf("HTTPS Code: %d\n", httpCode);
        String response = http.getString();
        // Serial.println(response);

        // ここから最後までbug修正
        // ptrを使った関数のデータ型を直す
        char *audioBase64 = nullptr;
        if (process(response, &audioBase64) != 0) {
          safeFree((void**)&audioBase64);
        }


        // JsonDocument doc;
        // DeserializationError error = deserializeJson(doc, response); 
        
        // if (error) {
        //   Serial.print("JSON parse failed: ");
        //   Serial.println(error.c_str());
        // }

        // size_t length = strlen(doc["data"]);

        // char *audioBase64 = (char *)ps_malloc(length + 1);
        // if (audioBase64 == nullptr) {
        //   Serial.println("PSRAM allocation failed");
        // }
        // strcpy(audioBase64, doc["data"]);
        // Serial.println("Processing Over");

        
        if (base64ToPcm(audioBase64, &pcmData) == -1) {
          safeFree((void**)&audioBase64);
          safeFree((void**)&pcmData);
        }
        safeFree((void**)&audioBase64);

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



// JSONから音声データを切り出す
int process(String response, char **audioBase64) {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, response);
  if (error) {
    Serial.print("JSON parse failed: ");
    Serial.println(error.c_str());
    return -1;
  }
  
  const char *data = doc["data"];
  if (data == nullptr) {
    Serial.println("data failed");
    return -1;
  }

  size_t length = strlen(data);

  *audioBase64 = (char *)ps_malloc(length + 1);
  if (*audioBase64 == nullptr) {
    Serial.println("PSRAM allocation failed");
    return -1;
  }

  strcpy(*audioBase64, data);

  Serial.println("Processing Over");

  return 0;
}


int base64ToPcm(char *audioBase64, char **pcmData) {
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
    &decodedLength,
    (const unsigned char*)audioBase64,
    base64Length
  );
  if (result != 0) {
    Serial.printf("Base64 decode failed: %d\n", result);
    free(*pcmData);
    return -1;
  }

  Serial.printf(
    "Base64 length: %u bytes\n",
    base64Length
  );
  Serial.printf(
    "PCM length: %u bytes\n",
    decodedLength
  );

  return decodedLength;
}


void notifyToLed(uint32_t status) {
  xTaskNotify(
    ledTaskHandle,
    status,
    eSetValueWithOverwrite
  );
}