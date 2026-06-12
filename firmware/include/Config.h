#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration
#define WIFI_SSID "IoTbaseLow"
#define WIFI_PASSWORD "01835120307"

// Cloud API Configuration
#define API_URL "http://192.168.0.103:8000/scan"

// Firebase Configuration
#define FIREBASE_HOST "ai-powered-ocr-to-braille-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "SOE1nnppVLfPkHML0ShPCHplb5KIF5Xaf5hyOWom"

// I2C LCD
#define PIN_I2C_SDA 21
#define PIN_I2C_SCL 14

// DRV8833 #1
#define PIN_DRV1_AIN1 38
#define PIN_DRV1_AIN2 39
#define PIN_DRV1_BIN1 40
#define PIN_DRV1_BIN2 41

// DRV8833 #2
#define PIN_DRV2_AIN1 1
#define PIN_DRV2_AIN2 2
#define PIN_DRV2_BIN1 3
#define PIN_DRV2_BIN2 42

// DRV8833 #3
#define PIN_DRV3_AIN1 45
#define PIN_DRV3_AIN2 46
#define PIN_DRV3_BIN1 47
#define PIN_DRV3_BIN2 48

// Buttons
#define PIN_BTN_CAPTURE 0 // On-board BOOT button
#define PIN_BTN_PAUSE 19  // USB D- (safe if Native USB unplugged)
#define PIN_BTN_PREV 20   // USB D+ (safe if Native USB unplugged)
#define PIN_BTN_NEXT                                                           \
  44 // RX pin (safe as input, TX still works for Serial output)

extern bool g_isPaused;

#endif
