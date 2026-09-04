#ifndef CONFIG_H
#define CONFIG_H

#include "global.h"


// constexpr char* ssid = "CMCC-3fXC";
// constexpr char* password = "5hnvu3fa";
constexpr char* ssid = "DESKTOP-S1FRS03 6988"; 
constexpr char* password = "s1frs03 6988";

// constexpr char* pcIp = "192.168.1.30";

constexpr AIModel model = AIModel::PC;

constexpr float volume = 0.02f;

constexpr int stackCheckButtonPin = 1;
constexpr int ledButtonPin = 2;
constexpr int RGBLedPin = 48;
constexpr int BCLK = 21;
constexpr int LRC = 14;
constexpr int DIN = 38;
constexpr int playerButtonPin = 45;


#endif