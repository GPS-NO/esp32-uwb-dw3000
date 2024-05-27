#include <Arduino.h>
#include <Wire.h>

#include "boarddefines.h"
#include "state_machine.h"

StateMachine* stateMachinePtr;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  pinMode(PIN_BTN, INPUT_PULLUP);

  Serial.println(F("###################################################"));
  Serial.println(F("(c) 2023-2024 Hochschule Bochum GPS:NO - Martin Peth, Niklas Schuetrumpf"));
  Serial.print(F("Compiled with c++ version "));
  Serial.println(__VERSION__);
  Serial.printf("Version v%s @%s %s at %s", VERSION_STRING, GIT_COMMIT, __DATE__, __TIME__);
  Serial.println();
  Serial.println(F("###################################################"));

  stateMachinePtr = StateMachine::getInstance();
}

void loop() {
  stateMachinePtr->currentState->onEnter();
  stateMachinePtr->currentState->onUpdate();
}