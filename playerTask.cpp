#include "playerTask.h"
#include "config.h"

const int SAMPLE_RATE = 24000;
const size_t CHUNK_SIZE = 4096;

float volume = 0.05f;

I2SClass I2S(I2S_NUM_0);


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
  else {
    Serial.println("I2S started");
  }
}

void IRAM_ATTR playerButton() {
  unsigned long currentTime = millis();

  if (currentTime - lastInterruptTime > debounceInterval) {
    BaseType_t higherPriorityTaskWoken = pdFALSE;

    vTaskNotifyGiveFromISR(
      playerTaskHandle,
      &higherPriorityTaskWoken
    );

    portYIELD_FROM_ISR(higherPriorityTaskWoken);
  }

  lastInterruptTime = currentTime;
}



//グローバルptr変数に保存されているアドレスの音声を再生する
void playerTask(void* pvParameters){
  while (1)  {
    ulTaskNotifyTake(
      pdTRUE,
      portMAX_DELAY
    );
    bool accepted = false;
    portENTER_CRITICAL(&audioStateMux);
    if (audioState == AudioState::PLAYING || audioState == AudioState::IDLE) {
      audioState = AudioState::PLAYING;
      accepted = true;
    }
    portEXIT_CRITICAL(&audioStateMux);
    if (!accepted) {
      continue;
    }
    Serial.println("再生");

    char temp[CHUNK_SIZE];
    for (size_t offset = 0; offset < pcmDataLength ; offset += CHUNK_SIZE) {
      size_t length = min(
        CHUNK_SIZE,
        pcmDataLength  - offset
      );


      // ボリュームを下げる
      memcpy(temp, pcmData + offset, length);
      volume = getVolume();

      int16_t *samples = (int16_t *)temp;
      for (size_t i = 0; i < length / 2; i++) {
          samples[i] *= volume;
      }

      size_t written = I2S.write(temp, length);

      if (written == 0) {
        Serial.println("I2S write failed");
        continue;
      }
    }
    Serial.println("再生終了");
    portENTER_CRITICAL(&audioStateMux);
    audioState = AudioState::IDLE;
    portEXIT_CRITICAL(&audioStateMux);
  }
}



float getVolume(){
    int adcValue = analogRead(volumePin);

    // 0.0 ～ 1.0
    float knob = adcValue / 4095.0f;

    // -60 dB ～ 0 dB
    float db = -60.0f * (1.0f - knob);

    // dB → 振幅倍率
    return powf(10.0f, db / 20.0f);
}