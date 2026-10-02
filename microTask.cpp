#include "microTask.h"
#include "config.h"

#define MIC_SHIFT 16


const int SAMPLE_RATE = 16000;
const int MAX_RECODE_SECONDS = 30;
const int MAX_LENGTH = (SAMPLE_RATE * MAX_RECODE_SECONDS) * sizeof(int16_t);

typedef int32_t my_int;

I2SClass micI2S(I2S_NUM_0);


void microInit() {
  micI2S.setPins(MIC_SCK, MIC_WS, -1, MIC_SD);

  if (!micI2S.begin(
    I2S_MODE_STD,
    SAMPLE_RATE,
    I2S_DATA_BIT_WIDTH_32BIT,
    I2S_SLOT_MODE_MONO,
    I2S_STD_SLOT_LEFT
  )) {
    Serial.println("INMP441 init failed.");
    return;
  }
  Serial.println("INMP441 ready.");
}


void microTask(void* pvParameters) {
  while (1) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    bool accepted = false;
    portENTER_CRITICAL(&audioStateMux);
    if (audioState == AudioState::IDLE) {
      audioState = AudioState::RECORDING;
      accepted = true;
    }
    portEXIT_CRITICAL(&audioStateMux);
    if (!accepted) {
      continue;
    }


    Serial.println("Recording...");
    pcmDataLength = 0;
    safeFree((void**)&pcmData);
    pcmData = (char *)ps_malloc(MAX_LENGTH);
    
    bool recording = true;
    int status = 0;
    while (recording) {
      if (ulTaskNotifyTake(pdTRUE, 0) > 0) {
        recording = false;
        break;
      }

      char samples[128];

      size_t bytesRead = micI2S.readBytes(
        (char *)samples,
        sizeof(samples)
      );
      if (bytesRead == 0) {
        Serial.println("I2SClass microphone read failed.");
        status = -1;
        break;
      }

      size_t sampleCount = bytesRead;
      if (MAX_LENGTH - pcmDataLength <= sampleCount) {
        sampleCount = MAX_LENGTH - pcmDataLength;
        recording = false;
      }


      int32_t *samples_32 = (int32_t *)samples;
      int16_t *pcm_int16 = (int16_t *)pcmData;
      for (size_t i = 0; i < sampleCount / sizeof(int32_t); i++) {
        int16_t sample = (int16_t)((uint32_t)samples_32[i] >> MIC_SHIFT);

        pcm_int16[pcmDataLength / 2] = sample;
        pcmDataLength += 2;
      }
    }

    Serial.println("Recording Over");
  }
}