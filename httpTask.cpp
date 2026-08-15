#include "global.h"
#include "config.h"
#include "httpTask.h"

// constexpr AIConfig aiConfig[] = {
//   {
//     "https: //httpbin.org/post",
//     ""
//   },
//   {
//     "https://generativelanguage.googleapis.com/v1beta/models/gemini-3.5-flash:generateContent",
//     ""
//   },
//   {
//     "https://api.deepseek.com/chat/completions",
//     ""
//   },
//   {
//     "http://192.168.1.30:8080/askGemini",
//     ""
//   }
// };


void httpTask(void* pvParameters) {
  while (1) {
    ulTaskNotifyTake(
      pdTRUE,
      portMAX_DELAY
    );

    camera_fb_t * fb = esp_camera_fb_get();

    if (fb == nullptr) {
      Serial.println("Camera capture failed");
      notifyToLed(HTTP_FAILED);
      continue;
    }
    Serial.printf(
      "Photo: %dx%d, %d bytes\n",
      fb->width,
      fb->height,
      fb->len
    );

    String imageBase64 = base64::encode(fb->buf, fb->len);
    esp_camera_fb_return(fb);


    if (WiFi.status() == WL_CONNECTED) {
      WiFiClient client;
      // client.setInsecure();
      HTTPClient http;

      

      http.begin(
        client,
        "http://192.168.1.30:8080/askGemini"
      );

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
        "contents": [
          {
            "text": "测试: 1.能否看得到那段日语写的回答要求? 2.能否看到图片? 分析一下图片。",
            "image": ")" + imageBase64 + R"("
          }
        ]
      })";



      notifyToLed(HTTP_WAITING);
      int httpCode = http.POST(body);
      // int httpCode = http.GET();
  

      if (httpCode > 0) {
        if (httpCode == 200) {
          notifyToLed(HTTP_SUCCESS);
        } else {
          notifyToLed(HTTP_WARNING);
        }
          Serial.printf("HTTPS Code: %d\n", httpCode);

          String response = http.getString();

          Serial.println(response);

      } else {
        notifyToLed(HTTP_FAILED);
        Serial.printf("Failed: %s\n", http.errorToString(httpCode).c_str());
      }

      http.end();
    }
  }
}

void notifyToLed(uint32_t status) {
  xTaskNotify(
    ledTaskHandle,
    status,
    eSetValueWithOverwrite
  );
}

// const AIConfig& getAIConfig(AIModel model) {
//   return aiConfig[static_cast<int>(model)];
// }
