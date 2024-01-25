#include <Arduino.h>
#include <Wire.h>
#include "state_machine_state.h"
#include "idle_state.h"
#include "setup_state.h"

const int flashBtnPin = 0;

State* StateMachineState::idleState = new IdleState();
State* StateMachineState::setupState = new SetupState();
State* StateMachineState::currentState = StateMachineState::idleState;

void setup(){
    Serial.begin(115200);
    while (!Serial) {;}

    pinMode(flashBtnPin, INPUT_PULLUP);
}

void loop(){
    StateMachineState::currentState->onEnter(); 
    StateMachineState::currentState->onUpdate();    
}