#include "playerTask.h"
#include "config.h"

const int SAMPLE_RATE = 24000;
const size_t CHUNK_SIZE = 4096;

I2SClass I2S;


void playerInit() {
  I2S.setPins(BCLK, LRC, DIN);

  if (!I2S.begin(
    I2S_MODE_STD,
    SAMPLE_RATE,
    I2S_DATA_BIT_WIDTH_16BIT,
    I2S_SLOT_MODE_MONO
  )) {
    Serial.println("I2S init failed");
  }

  Serial.println("I2S started");
}

//グローバル変数に保存されている音声を再生する
void playerTask(void* pvParameters){
  while (1)  {
    ulTaskNotifyTake(
      pdTRUE,
      portMAX_DELAY
    );

    for (size_t offset = 0; offset < decodedLength; offset += CHUNK_SIZE) {
      size_t length = min(
        CHUNK_SIZE,
        decodedLength - offset
      );

      // ボリュームを下げる
      int16_t *samples = (int16_t *)(pcmData + offset);
      for (size_t i = 0; i < length / 2; i++) {
          samples[i] *= volume;
      }

      size_t written = I2S.write(
        pcmData + offset,
        length
      );

      if (written == 0) {
        Serial.println("I2S write failed");
        continue;
      }
    }
    safeFree((void**)&pcmData);
  }
}