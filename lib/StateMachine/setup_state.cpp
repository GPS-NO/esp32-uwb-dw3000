#include "setup_state.h"
#include "config_manager.h"
#include <esp_system.h>
#include <WiFi.h>
#include <Arduino.h> 

const int maxAttempts = 10;
const int attemptDelay = 1000;

void SetupState::onEnter() {
    Serial.println("[*] Enter State: Setup");

    ConfigManager::loadConfig();

    if(strlen(ConfigManager::deviceConfig.ssid) == 0 || strlen(ConfigManager::deviceConfig.password) == 0) {
        Serial.println("No WiFi credentials found.");
        return;
    }
    connectToWiFi(ConfigManager::deviceConfig.ssid, ConfigManager::deviceConfig.password);
}

void SetupState::onUpdate() {
    Serial.println("================================");
    Serial.println("1: Wechsel in State: Error");
    Serial.println("================================");

    while(1){
        if (Serial.available() > 0) {
            int serialInput = Serial.parseInt();

            switch(serialInput) {
                case 1:
                    Serial.println("Dieser State existiert noch nicht!");
                    break;
                default:
                    Serial.println("Ungültige Kombination.");
                    break;
            }
        }

        delay(100);
    }
}

void SetupState::onExit() {
    //
}

void SetupState::generateId(char* buffer, int length) {
    const char characters[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    for (size_t i = 0; i < length - 1; ++i) {
        buffer[i] = characters[random(sizeof(characters) - 1)];
    }

    buffer[length - 1] = '\0';  
}

void SetupState::connectToWiFi(const char* ssid, const char* password) {    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    unsigned long startMillis = millis();
    int attempts = 0;

    while (WiFi.status() != WL_CONNECTED && millis() - startMillis < maxAttempts * attemptDelay) {
        delay(attemptDelay);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("");
        Serial.println("Successfully connected to the WiFi network with the following local IP: ");
        Serial.print(WiFi.localIP());
        Serial.println("");
    } else {
        Serial.println("Error connecting to the WiFi network!");
        Serial.println(WiFi.status());
    }
}
