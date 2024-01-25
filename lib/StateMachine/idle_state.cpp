#include "idle_state.h"
#include <Arduino.h>

void IdleState::onEnter() {
    Serial.println("[*] Enter State: Idle");
}

void IdleState::onUpdate() {
    Serial.println("================================");
    Serial.println("1: Wechsel in State: Setup");
    Serial.println("2: Wechsel in State: Error");
    Serial.println("================================");

    while(1){
        if (Serial.available() > 0) {
            int serialInput = Serial.parseInt();

            switch(serialInput) {
                case 1:
                    StateMachineState::currentState = StateMachineState::setupState;
                    IdleState::onExit();
                    return;;
                case 2:
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

void IdleState::onExit() {
    //
}