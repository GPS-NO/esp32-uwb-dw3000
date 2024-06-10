#include "shutdown_state.h"

void ShutdownState::onEnter() {
  Serial.println("[*] Enter State: Shutdown");
  stateMachinePtr = StateMachine::getInstance();
}

void ShutdownState::onUpdate() {
  stateMachinePtr->currentState = stateMachinePtr->idleState;
  ShutdownState::onExit();
}

void ShutdownState::onExit() {}