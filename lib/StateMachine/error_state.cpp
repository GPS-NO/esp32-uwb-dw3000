#include "error_state.h"

void ErrorState::onEnter() {
  Serial.println("[*] Enter State: Error");
  stateMachinePtr = StateMachine::getInstance();
  stateMachinePtr->setStatus(STATUS_ERROR);
}

void ErrorState::onUpdate() {}

void ErrorState::onExit() {}