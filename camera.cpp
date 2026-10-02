#include "camera.h"

void cameraInit() {
  static camera_config_t config = {};

  config.pin_pwdn = -1;
  config.pin_reset = -1;

  config.pin_xclk = 15;

  config.pin_sscb_sda = 4;
  config.pin_sscb_scl = 5;

  config.pin_d7 = 16;
  config.pin_d6 = 17;
  config.pin_d5 = 18;
  config.pin_d4 = 12;
  config.pin_d3 = 10;
  config.pin_d2 = 8;
  config.pin_d1 = 9;
  config.pin_d0 = 11;

  config.pin_vsync = 6;
  config.pin_href = 7;
  config.pin_pclk = 13;


  config.xclk_freq_hz = 20000000;
  config.ledc_timer = LEDC_TIMER_0;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_SXGA;
  config.jpeg_quality = 10;
  config.fb_count = 1;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  

  if (psramFound()) {
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.frame_size = FRAMESIZE_SXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  }
  else {
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);

  if (err != ESP_OK) {
    Serial.printf(
        "Camera init failed: 0x%x\n",
        err
    );
    return;
  }
  Serial.println("Camera init OK");
} 

// int cameraTake(String *p) {
//   camera_fb_t *fb = esp_camera_fb_get();

//   if (fb == nullptr) {
//     Serial.println("Camera capture failed");
//     return -1;
//   }
//   Serial.printf(
//     "Photo: %dx%d, %d bytes\n",
//     fb->width,
//     fb->height,
//     fb->len
//   );

//   *p = base64::encode(fb->buf, fb->len);
//   esp_camera_fb_return(fb);
  
//   return 0;
// }


// String cameraTake() {
//   camera_fb_t * fb = esp_camera_fb_get();

//   if (fb == nullptr) {
//     Serial.println("Camera capture failed");
//     return;
//   }
//   Serial.printf(
//     "Photo: %dx%d, %d bytes\n",
//     fb->width,
//     fb->height,
//     fb->len
//   );

//   String imageBase64 = base64::encode(fb->buf, fb->len);
//   esp_camera_fb_return(fb);

//   return imageBase64;
// }



