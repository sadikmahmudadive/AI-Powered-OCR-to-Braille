#include <Arduino.h>
#include <WiFi.h>
#include "Config.h"
#include "CameraManager.h"
#include "camera_server.h"
#include "OCRClient.h"
#include "BrailleTranslator.h"
#include "SolenoidController.h"
#include "ButtonManager.h"
#include "LCDManager.h"
#include "TextNavigator.h"
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
// Uncomment to enable Firebase streaming (default enabled)
#define ENABLE_FIREBASE_STREAM
LiquidCrystal_I2C lcd(0x27, 16, 2);
bool g_isPaused = false;
unsigned long lastDisplayUpdate = 0;
const unsigned long autoNextDelay = 2000;

// Firebase configuration objects
FirebaseData fbdoStream;
FirebaseAuth auth;
FirebaseConfig configFb;

// Forward declaration of display update helper
void updateDisplayForCurrentChar();

void streamCallback(FirebaseStream data) {
    if (data.dataType() == "string") {
        String text = data.stringData();
        Serial.printf("Firebase update: %s\n", text.c_str());
        
        if (text.length() > 0) {
            TextNavigator::setText(text);
            g_isPaused = false;
            updateDisplayForCurrentChar();
        }
    }
}

void streamTimeoutCallback(bool timeout) {
    if (timeout) {
        Serial.println("Firebase Stream timeout, resuming...");
    }
}
// Pause/resume Firebase stream during OCR upload
void pauseFirebaseStream(bool pause) {
    if (pause) {
        Firebase.RTDB.endStream(&fbdoStream);
        Serial.println("Firebase stream paused for OCR upload");
    } else {
        Firebase.RTDB.beginStream(&fbdoStream, "/active_text");
        Serial.println("Firebase stream resumed");
    }
}

#ifdef ENABLE_FIREBASE_STREAM
void setupFirebase() {
    configFb.host = FIREBASE_HOST;
    configFb.signer.tokens.legacy_token = FIREBASE_AUTH;
    
    Firebase.begin(&configFb, &auth);
    Firebase.reconnectWiFi(true);
    
    if (!Firebase.RTDB.beginStream(&fbdoStream, "/active_text")) {
        Serial.printf("Firebase stream error: %s\n", fbdoStream.errorReason().c_str());
    } else {
        Firebase.RTDB.setStreamCallback(&fbdoStream, streamCallback, streamTimeoutCallback);
        Serial.println("Firebase Stream configured successfully!");
    }
}
#endif

void setupWiFi() {
    LCDManager::displayStatus("Connecting WiFi");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("");
        Serial.println("WiFi connected");
        LCDManager::displayStatus("WiFi Connected!");
    } else {
        Serial.println("");
        Serial.println("WiFi connection failed");
        LCDManager::displayStatus("WiFi Failed");
    }
    delay(1000);
}

void setup() {
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    Serial.println();

    // Initialize peripherals
    ButtonManager::init();
    SolenoidController::init();
    LCDManager::init();

    // Connect to WiFi
    setupWiFi();

    // Set up Realtime Database listener
    setupFirebase();

    // Initialize camera
    if (!CameraManager::init()) {
        LCDManager::displayStatus("Camera Error");
        Serial.println("Camera init failed!");
    } else {
        Serial.println("Camera initialized OK");
    }

    // Start the camera web server (stream + control UI)
    startCameraServer();

    // Print access URLs
    Serial.print("Camera Ready! Use 'http://");
    Serial.print(WiFi.localIP());
    Serial.println("' to connect");
    Serial.print("Stream URL: http://");
    Serial.print(WiFi.localIP());
    Serial.println(":81/stream");

    // Show camera stream address on LCD for 3 seconds
    LCDManager::displayIPAddress(WiFi.localIP().toString());
    delay(3000);

    LCDManager::displayStatus("System Ready");

    // Default text for test
    TextNavigator::setText("Hello");
}

void updateDisplayForCurrentChar() {
    char c = TextNavigator::getCurrentChar();
    int pattern[6] = {0};
    BrailleTranslator::getBraillePattern(c, pattern);
    LCDManager::displayCharacter(c, pattern);
    SolenoidController::displayBrailleCell(pattern);
}

void loop() {
    ButtonManager::update();

    if (ButtonManager::isCapturePressed()) {
    // Pause Firebase stream to free a socket for the OCR HTTP request
    #ifdef ENABLE_FIREBASE_STREAM
    pauseFirebaseStream(true);
    #endif

    LCDManager::displayStatus("Capturing...");
    camera_fb_t* fb = CameraManager::capture();
    if (fb) {
        LCDManager::displayStatus("Uploading...");
        String result = OCRClient::sendImageForOCR(fb);
        CameraManager::release(fb);

        // Resume Firebase after we got the response
        #ifdef ENABLE_FIREBASE_STREAM
        pauseFirebaseStream(false);
        #endif

        if (result == "__ERROR__") {
            LCDManager::displayStatus("OCR Failed");
        } else if (result.length() > 0) {
            TextNavigator::setText(result);
            g_isPaused = false;
            updateDisplayForCurrentChar();
        } else {
            LCDManager::displayStatus("No Text Found");
        }
    } else {
        LCDManager::displayStatus("Cap Error");
    }
}


    if (ButtonManager::isPausePressed()) {
        g_isPaused = !g_isPaused;
        if (g_isPaused) {
            LCDManager::displayStatus("Paused");
        } else {
            updateDisplayForCurrentChar();
        }
    }

    if (ButtonManager::isPrevPressed()) {
        if (TextNavigator::prevChar()) {
            updateDisplayForCurrentChar();
        }
        lastDisplayUpdate = millis();
    }

    if (ButtonManager::isNextPressed()) {
        if (TextNavigator::nextChar()) {
            updateDisplayForCurrentChar();
        }
        lastDisplayUpdate = millis();
    }

    // Auto navigate if not paused
    if (!g_isPaused && TextNavigator::hasText()) {
        if (millis() - lastDisplayUpdate > autoNextDelay) {
            if (TextNavigator::nextChar()) {
                updateDisplayForCurrentChar();
            }
            lastDisplayUpdate = millis();
        }
    }

    delay(10);
}
