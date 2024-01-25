#include "setup_state.h"
#include "config_manager.h"
#include <Arduino.h>
#include <WiFiManager.h>  

const int maxAttempts = 10;
const int attemptDelay = 1000;

void SetupState::onEnter() {
    Serial.println("[*] Enter State: Setup");
    ConfigManager::loadConfig();
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
