#include <Arduino.h>
#include <Wire.h>

#include "idle_state.h"
#include "setup_state.h"
#include "ranging_state.h"
#include "state_machine_state.h"

const int flashBtnPin = 0;

State* StateMachineState::idleState = new IdleState();
State* StateMachineState::setupState = new SetupState();
State* StateMachineState::rangingState = new RangingState();
State* StateMachineState::currentState = StateMachineState::idleState;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;

  pinMode(flashBtnPin, INPUT_PULLUP);

  Serial.println(F("###################################################"));
  Serial.println(F("(c) 2023-2024 Hochschule Bochum GPS:NO - Martin Peth, Niklas Schuetrumpf"));
  Serial.print(F("Compiled with c++ version "));
  Serial.println(__VERSION__);
  Serial.printf("Version v%s @%s %s at %s", VERSION_STRING, GIT_COMMIT, __DATE__, __TIME__);
  Serial.println();
  Serial.println(F("###################################################"));
}

void loop() {
  StateMachineState::currentState->onEnter();
  StateMachineState::currentState->onUpdate();
}