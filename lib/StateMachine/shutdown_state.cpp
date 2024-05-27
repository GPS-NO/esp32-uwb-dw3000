#include "shutdown_state.h"

void ShutdownState::onEnter() {
  Serial.println("[*] Enter State: Shutdown");
  stateMachinePtr = &StateMachine::getInstance();
}

void ShutdownState::onUpdate() {
  StateMachine::getInstance().currentState = StateMachine::getInstance().idleState;
  ShutdownState::onExit();
}

void ShutdownState::onExit() {}