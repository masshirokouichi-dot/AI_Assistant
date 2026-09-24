#include <ESP_I2S.h>


void playerInit();
void IRAM_ATTR playerButton();
void playerTask(void* pvParameters);
float getVolume();