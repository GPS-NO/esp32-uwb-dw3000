#include "setup_state.h"
#include <Arduino.h>

void SetupState::onEnter() {
    Serial.println("[*] Enter State: Idle");
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