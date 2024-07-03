#include "idle_state.h"

void IdleState::onEnter() {
  Serial.println("[*] Enter State: Idle");
  stateMachinePtr = StateMachine::getInstance();
  stateMachinePtr->setStatus(STATUS_IDLE);
}

void IdleState::onUpdate() {
  stateMachinePtr->currentState = stateMachinePtr->setupState;
}

void IdleState::onExit() {
  //
}